#pragma once

#include <WinSock2.h>
#include <string>
#include <array>
#include <string_view>
#include <format>

inline constexpr unsigned __int32 MAX_BUFFER_SIZE{ 8096 };
inline constexpr unsigned __int32 MAX_PACKET_SIZE{ 512 };
inline constexpr int RECORDE_MILLISEC = 500;
inline constexpr int RECORDE_HZ = 8000;
inline constexpr int RECORDE_CHANNEL = 1;
inline constexpr int RECORDE_BUFFER_SIZE = int((RECORDE_HZ * RECORDE_CHANNEL) * (RECORDE_MILLISEC / 1000.0f));

inline constexpr int MAX_ID = 32;
inline constexpr int MAX_CHAT_BUF = 512;

template <typename DerivedType>
inline void* DerivedCpyPointer(DerivedType* pData)
{
	return reinterpret_cast<char*>(pData) + sizeof(DerivedType*);
}

// 패킷 타입을 지정할 enum class
enum PACKET_TYPE : unsigned __int16 {
	CREATE_TYPE,
	CHAT_TYPE,
	POS_TYPE,
	VOICE_TYPE,
};

struct PCHEADER {
	unsigned __int16 type{ };
	unsigned __int16 length{ };
	unsigned __int16 from{ };
	std::array<char, MAX_ID> id{ };
};

class Packet abstract {
public:
	unsigned __int16 Length() const { return m_header.length; }
	unsigned __int16 Type() const { return m_header.type; }

	unsigned __int16 From() const { return m_header.from; }
	void SetFrom(unsigned __int16 from) { m_header.from = from; }

	void SetId(const std::string_view id) {
		std::copy(id.begin(), id.end(), m_header.id.data());
	}

	std::string_view GetId() const { return m_header.id.data(); }

	virtual std::string PrintPacket() const PURE;

protected:
	PCHEADER m_header{ };
};

class IdPacket : public Packet {
public:
	IdPacket() { m_header = { CREATE_TYPE, static_cast<unsigned short>(sizeof(*this) - sizeof(Packet*)) }; }
	IdPacket(std::string_view id) { 
		m_header = { CREATE_TYPE, static_cast<unsigned short>(sizeof(*this) - sizeof(Packet*)) }; 
		SetId(id);
	}
	virtual std::string PrintPacket() const { return ""; }
};

class ChatPacket : public Packet {
public:
	ChatPacket() { m_header = { CHAT_TYPE, static_cast<unsigned short>(sizeof(*this) - MAX_CHAT_BUF - sizeof(Packet*)), }; }
	ChatPacket(std::string_view str) {
		m_header = { CHAT_TYPE, static_cast <unsigned __int16>(sizeof(*this) - MAX_CHAT_BUF - sizeof(Packet*) + str.size()) };
		std::copy(str.begin(), str.end(), m_chatMessage.data());
	}

	void SetMessage(std::string_view str) {
		std::copy(str.begin(), str.end(), m_chatMessage.data());
	}

	virtual std::string PrintPacket() const {
		return std::format("From {}: {}\n", m_header.id.data(), m_chatMessage.data());
	}

protected:
	std::array<char, MAX_CHAT_BUF> m_chatMessage{ };
};

class PositionPacket : public Packet {
public:
	PositionPacket() { m_header = { POS_TYPE, sizeof(*this) - sizeof(Packet*), }; }
	PositionPacket(float x, float y, float z) : x{ x }, y{ y }, z{ z } { m_header = { POS_TYPE, sizeof(*this) - sizeof(Packet*), }; }

public:
	virtual std::string PrintPacket() const {
		return std::format("Packet Type: Position, Packet Data: ({}, {}, {})\n", x, y, z);
	}

private:
	float x{ };
	float y{ };
	float z{ };
};

class VoicePacket : public Packet {
public:
	VoicePacket() { m_header = { VOICE_TYPE, sizeof(*this) - sizeof(Packet*), }; }
	VoicePacket(char* data, size_t dataSize) {
		m_header = { VOICE_TYPE, static_cast <unsigned __int16>(sizeof(*this) - RECORDE_BUFFER_SIZE - sizeof(Packet*) + dataSize) };
		std::memcpy(m_data.data(), data, dataSize);
	}

	void SetVoiceData(char* data, size_t dataSize) {
		std::memcpy(m_data.data(), data, dataSize);
	}

	const char* GetBuffer() const { return m_data.data(); }

	virtual std::string PrintPacket() const { return ""; }

private:
	std::array<char, RECORDE_BUFFER_SIZE> m_data{ };
};

class PacketFactory {
public:
	static Packet* CreatePacket(PACKET_TYPE type) {
		switch (type) {
		case CREATE_TYPE:
			return new IdPacket{ };

		case CHAT_TYPE:
			return new ChatPacket{ };

		case POS_TYPE:
			return new PositionPacket{ };

		case VOICE_TYPE:
			return new VoicePacket{ };
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