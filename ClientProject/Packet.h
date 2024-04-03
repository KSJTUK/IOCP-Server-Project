#pragma once

class MemoryBuf {
private:
	size_t m_offset{ };
	size_t m_readPoint{ };
	std::array<char, MAX_BUFFER_SIZE> m_data{ };

};

// ��Ŷ Ÿ���� ������ enum class
enum PACKET_TYPE : unsigned __int16 {
	CHAT_TYPE,
	POS_TYPE,
};

// ��Ŷ�� ũ��, Ÿ�Ե��� ������ ����ü
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