#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#include <deque>
#include <mutex>
#include <string>
#include <format>
#include <thread>
#include <iostream>

inline constexpr int MAX_BUFFER_SIZE{ 1024 };
inline constexpr int MAX_PACKET_SIZE{ 512 };

enum class IO_TYPE {
    RECV,
    SEND
};

struct OverlappedEx {
    OVERLAPPED overlapped{ };
    WSABUF buffer{ };
    IO_TYPE ioType{ };
};

struct ChatPacket {
    short length{ };
    short toWhom{ };
    char msg[MAX_PACKET_SIZE - sizeof(short) * 2];
};