#pragma once

#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

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