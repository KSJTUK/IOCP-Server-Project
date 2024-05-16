#pragma once


#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#include "Voice.h"

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

#include "../Packets/Packet.h"
#pragma comment(lib, "../Lib/Packets.lib")

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

struct Client {
	IOData sendIO{ };
	IOData recvIO{ };
};

class NetworkClient {
public:
	NetworkClient();
	~NetworkClient();

public:
	bool ConnectToServer(unsigned __int16 port, std::string_view serverIP);
	bool BindIOCP();
	bool BindRecv();

	bool MainThread();
	void StartServer();

	void RecvComplete(char* pData, size_t size);
	void SendComplete();

	void PacketProcess();
	void SendPacket();

	void Run();

	void InsertPacketQueue(Packet* pPacket);

	void ProcessCreateTypePacket(Packet* pPacket);
	void ProcessChatPacket(Packet* pPacket);
	void ProcessPositionPacket(Packet* pPacket);
	void ProcessVoicePacket(Packet* pPacket);

public:
	void SetClientId(std::string_view id);

protected:
	std::string m_id{ };

private:
	SOCKET m_socket{ };
	HANDLE m_cpHandle{ };
	std::condition_variable m_cv{ };

	bool m_packetProcRunning{ true };

	std::jthread m_mainThread{ };
	std::jthread m_packetProcThread{ };

	std::mutex m_packetLock{ };
	std::deque<Packet*> m_packetQueue{ };

	std::unordered_map<unsigned __int16, std::function<void(NetworkClient&, Packet*)>> m_processFuncs{};

	Client m_processStruct{ };
	std::unique_ptr<VoicePlayer> m_voicePlayer{ };
};


// not server func
#pragma region FOR_TEST
#define SERVER_TEST 0

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

#ifdef _DEBUG
#include <crtdbg.h>
#define CRT_START _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif