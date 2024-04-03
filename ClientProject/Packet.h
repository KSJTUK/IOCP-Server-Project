#pragma once

class MemoryBuf {
private:
	size_t m_offset{ };
	size_t m_readPoint{ };
	std::array<char, MAX_BUFFER_SIZE> m_data{ };

};

// 패킷 타입을 지정할 enum class
enum PACKET_TYPE : unsigned __int16 {
	CHAT_TYPE,
	POS_TYPE,
};

// 패킷의 크기, 타입등을 지정할 구조체
struct PacketHead {
	unsigned __int16 length{ };
	unsigned __int16 type{ };
};

struct ChatPacket : public PacketHead {
	unsigned __int16 from{ };
	char msg[1024]{ };
};

struct Position : public PacketHead {
	float x{ };
	float y{ };
	float z{ };
};

union Packet {	
	char* pData{ };
};