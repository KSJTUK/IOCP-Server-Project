#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

#include <deque>
#include <thread>
#include <vector>
#include <memory>
#include <format>
#include <chrono>
#include <sstream>
#include <optional>
#include <iostream>
#include <functional>
#include <syncstream>

#include "Packet.h"

inline constexpr unsigned __int32 MAX_PACKET_SIZE{ 512 };

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
	SOCKET socket{ };
	IO_TYPE ioType{ };
};