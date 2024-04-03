#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#include <deque>
#include <mutex>
#include <string>
#include <sstream>
#include <format>
#include <thread>
#include <chrono>
#include <iostream>

inline constexpr int MAX_BUFFER_SIZE{ 1024 };
inline constexpr int MAX_PACKET_SIZE{ 512 };

class TimeUtil {
public:
    static void Init();
    static void End();
    static void PrintTime();
    static std::string GetTime();

private:
    inline static std::locale prevLoc{ };

    inline static std::chrono::system_clock::time_point timeNow{ };
    inline static time_t timeUTC{ };
    inline static tm* timeLocal{ };
};

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