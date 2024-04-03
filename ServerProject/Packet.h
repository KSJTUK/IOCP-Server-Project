#pragma once

// 패킷 타입을 지정할 enum class
enum class PACKET_TYPE : unsigned __int16 {
	CHAT_TYPE
};

// 패킷의 크기, 타입등을 지정할 구조체
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