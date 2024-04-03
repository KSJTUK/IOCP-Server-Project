#pragma once

// ��Ŷ Ÿ���� ������ enum class
enum class PACKET_TYPE : unsigned __int16 {
	CHAT_TYPE
};

// ��Ŷ�� ũ��, Ÿ�Ե��� ������ ����ü
struct PacketHead {
	unsigned __int16 length{ };
	unsigned __int16 type{ };
};

struct ChatPacket : PacketHead {
	unsigned __int16 from{ };
	char msg[1024]{ };

	ChatPacket(size_t messageLength, const char* message) {
		std::memcpy(msg, message, messageLength);
	}

	ChatPacket(const std::string& message) {
		std::copy(message.begin(), message.end(), msg);
	}
};