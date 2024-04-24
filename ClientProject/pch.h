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
#include <array>
#include <iostream>
#include <unordered_map>
#include <functional>

#include "Packet.h"

#pragma region FOR_TEST
#define SERVER_TEST 1

#if SERVER_TEST

#define TEST_TIME_MS 2000
#define MIN_RAND_MESSAGE_LENGTH 10
#define MAX_RAND_MESSAGE_LENGTH 100

#include <random>
inline std::random_device rd{ };
inline std::default_random_engine dre{ rd() };
inline std::uniform_int_distribution uidLength{ MIN_RAND_MESSAGE_LENGTH, MAX_RAND_MESSAGE_LENGTH };
inline std::uniform_int_distribution<int> uidChar{ 'a', 'z' };
inline std::uniform_int_distribution<unsigned __int16> uidPacket{ CHAT_TYPE, POS_TYPE };
inline std::uniform_real_distribution<float> ufd{ 0.f, 100.f };

#endif
#pragma endregion

inline constexpr int MAX_BUFFER_SIZE{ 1024 };
inline constexpr int MAX_PACKET_SIZE{ 512 };

template <typename DerivedType>
inline void* DerivedCpyPointer(DerivedType* pData)
{
    return reinterpret_cast<char*>(pData) + sizeof(DerivedType*);
}

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

struct IOData {
    OVERLAPPED overlapped{ };
    WSABUF wsaBuf{ };
    IO_TYPE ioType{ };

    std::array<char, MAX_BUFFER_SIZE> buffer{ };

public:
    void BufClear() {
        buffer.fill(0);
    }
};