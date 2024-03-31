#pragma once

#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

#include <thread>
#include <vector>
#include <memory>
#include <format>
#include <optional>
#include <iostream>
#include <syncstream>

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