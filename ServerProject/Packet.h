#pragma once

#include "MemoryBuf.h"
#define MAX_CHAT_BUF 512

// 패킷 타입을 지정할 enum class
enum PACKET_TYPE : unsigned __int16 {
	CHAT_TYPE,
	POS_TYPE,
};

struct PCHEADER {
	unsigned __int16 type{ };
	unsigned __int16 length{ };
	unsigned __int16 from{ };
};

class Packet abstract {
public:
	virtual unsigned __int16 Length() const PURE;
	virtual unsigned __int16 Type() const PURE;

	unsigned __int16 From() const { return m_header.from; }
	void SetFrom(unsigned __int16 from) { m_header.from = from; }

	virtual void Encode(MemoryBuf& buf) PURE;
	virtual void Decode(MemoryBuf& buf) PURE;

	virtual void PrintPacket() const PURE;

protected:
	PCHEADER m_header{ };
};

class ChatPacket : public Packet {
public:
	ChatPacket() { m_header = { CHAT_TYPE, sizeof(*this) - MAX_CHAT_BUF - sizeof(Packet*), }; }
	ChatPacket(std::string_view str) {
		m_header = { CHAT_TYPE, static_cast <unsigned __int16>(sizeof(*this) - MAX_CHAT_BUF - sizeof(Packet*) + str.size()) };
		std::copy(str.begin(), str.end(), m_chatMessage.data());
	}

	virtual unsigned __int16 Length() const { return m_header.length; }
	virtual unsigned __int16 Type() const { return m_header.type; }

	virtual void Encode(MemoryBuf& buf) {
		buf << m_header.type;
		buf << m_header.length;
		buf << m_header.from;
		buf << m_chatMessage;
	}

	virtual void Decode(MemoryBuf& buf) {
		buf >> m_header.type >> m_header.length >> m_header.from >> m_chatMessage;
	}

	void SetMessage(std::string_view str) {
		std::copy(str.begin(), str.end(), m_chatMessage.data());
	}

	virtual void PrintPacket() const {
		std::cout << std::format("Packet Type: Chat, Packet Data: {}\n", m_chatMessage.data());
	}

protected:
	std::array<char, MAX_CHAT_BUF> m_chatMessage{ };
};

class PositionPacket : public Packet {
public:
	PositionPacket() { m_header = { POS_TYPE, sizeof(*this) - sizeof(Packet*), }; }
	PositionPacket(float x, float y, float z) : x{ x }, y{ y }, z{ z } { m_header = { POS_TYPE, sizeof(*this) - sizeof(Packet*), }; }

public:
	virtual unsigned __int16 Length() const { return m_header.length; }
	virtual unsigned __int16 Type() const { return m_header.type; }

	virtual void Encode(MemoryBuf& buf) {
		buf << m_header.type;
		buf << m_header.length;
		buf << m_header.from;
		buf << x << y << z;
	}

	virtual void Decode(MemoryBuf& buf) {
		buf >> m_header.type >> m_header.length >> m_header.from >> x >> y >> z;
	}

	virtual void PrintPacket() const {
		std::cout << std::format("Packet Type: Position, Packet Data: ({}, {}, {})\n", x, y, z);
	}

private:
	float x{ };
	float y{ };
	float z{ };
};

class PacketFacrory {
public:
	inline static std::unique_ptr<MemoryBuf> m_buffer = std::make_unique<MemoryBuf>();

public:
	static Packet* CreatePacket(PACKET_TYPE type) {
		switch (type) {
		case CHAT_TYPE:
			return new ChatPacket{ };

		case POS_TYPE:
			return new PositionPacket{ };
		}

		return nullptr;
	}

	static Packet* CreatePacket(const char* pRawPacketData) {
		PACKET_TYPE type{ };
		std::memcpy(std::addressof(type), pRawPacketData, sizeof(unsigned __int16));
		return CreatePacket(type);
	}

	template <typename T, typename... Args>
	static Packet* CreatePacket(Args... args) {
		return new T{ args... };
	}
};