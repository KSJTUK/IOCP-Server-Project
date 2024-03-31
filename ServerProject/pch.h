#pragma once

#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

#include <deque>
#include <thread>
#include <vector>
#include <memory>
#include <format>
#include <optional>
#include <iostream>
#include <functional>
#include <syncstream>

inline constexpr unsigned __int32 MAX_PACKET_SIZE{ 512 };

enum class IO_TYPE {
	RECV,
	SEND
};

struct OverlappedEx {
	OVERLAPPED overlapped{ };
	WSABUF buffer{ };
	SOCKET socket{ };
	IO_TYPE ioType{ };
};

struct ChatPacket {
	short length{ };
	short toWhom{ };
	char msg[MAX_PACKET_SIZE - sizeof(short) * 2];
};