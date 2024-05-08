#pragma once

#include "Profiler.h"
#include "Voice.h"
#include "Packet.h"

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

	void InsertPacketQueue(char* pData);
	void InsertPacketQueue(Packet* pPacket);

	void ProcessChatPacket(Packet* pPacket);
	void ProcessPositionPacket(Packet* pPacket);
	void ProcessVoicePacket(Packet* pPacket);

protected:
	TimeProfiler m_timer{ };

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