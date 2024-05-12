#pragma once

#define MAX_CHAT_BUF 512

// 패킷 타입을 지정할 enum class
enum PACKET_TYPE : unsigned __int16 {
	CHAT_TYPE,
	POS_TYPE,
	VOICE_TYPE,
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

	virtual std::string PrintPacket() const PURE;

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

	void SetMessage(std::string_view str) {
		std::copy(str.begin(), str.end(), m_chatMessage.data());
	}

	virtual std::string PrintPacket() const {
		return std::format("From {}: {}\n", m_header.from, m_chatMessage.data());
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

	virtual unsigned __int16 Length() const { return m_header.length; }
	virtual unsigned __int16 Type() const { return m_header.type; }

	void SetVoiceData(char* data, size_t dataSize) {
		std::memcpy(m_data.data(), data, dataSize);
	}

	const char* GetBuffer() const { return m_data.data(); }

	virtual std::string PrintPacket() const { return ""; }

private:
	std::array<char, RECORDE_BUFFER_SIZE> m_data{ };
};

class PacketFacrory {
public:
	static Packet* CreatePacket(PACKET_TYPE type) {
		switch (type) {
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