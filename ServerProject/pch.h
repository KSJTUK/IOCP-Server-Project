#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "winmm.lib")

#include <deque>
#include <thread>
#include <vector>
#include <memory>
#include <format>
#include <array>
#include <chrono>
#include <sstream>
#include <optional>
#include <iostream>
#include <functional>
#include <unordered_map>
#include <syncstream>

#include "Packets/Packet.h"
#pragma comment(lib, "../Lib/Packets.lib")

#ifdef _DEBUG
#include <crtdbg.h>
#define CRT_START _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

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
	SOCKET socket{ };
	IO_TYPE ioType{ };

	std::array<char, MAX_BUFFER_SIZE> buffer{ };

public:
	void BufClear();
};