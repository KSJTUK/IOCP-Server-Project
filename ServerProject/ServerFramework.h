#pragma once

#include "Client.h"
#include "Profiler.h"

class NetworkServer abstract {
public:
	explicit NetworkServer();
	virtual ~NetworkServer();

public:
	bool BindAndListen(const unsigned __int16 port);
	bool StartServer(const unsigned __int32 maxClient, unsigned __int32 maxThread = 0);

	Client& GetClient(__int32 clientIndex);

	virtual void Connect(__int32 clientIndex) PURE;
	virtual void Receive(__int32 clientIndex, std::size_t recvSize, char* pRecvData) PURE;
	virtual void Close(__int32 clientIndex) PURE;

protected:
	void End();

private:
	void CreateClients(const unsigned __int32 maxClient);
	bool RunIOWorkThread(unsigned __int32 maxThread);
	bool RunAcceptThread();

	void WorkThread();
	void AcceptThread();


	std::optional<std::reference_wrapper<Client>> GetUnConnectedClient();

protected:
	TimeProfiler m_timer{ };
	std::vector<Client> m_clients{ };

private:
	SOCKET m_listeningSocket{ INVALID_SOCKET };

	std::mutex m_defaultLock{ };

	HANDLE m_cpHandle{ nullptr };

	unsigned __int32 m_connectedClientSize{ };

	std::vector<std::jthread> m_workThreads{ };
	std::jthread m_acceptThread{ };

	bool m_workThreadRunning{ true };
	bool m_acceptThreadRunning{ true };
};

// --------------------------------------------
class EchoServer : public NetworkServer {
public:
	EchoServer() { };
	virtual ~EchoServer();

public:
	virtual void Connect(__int32 clientIndex) override { };
	virtual void Receive(__int32 clientIndex, std::size_t recvByte, char* pRecvData);
	virtual void Close(__int32 clientIndex) override;

	bool SendPacket(__int32 clinetIndex, Packet* packet);
	bool SendAll(__int32 clientIndex, Packet* packet);

	void InsertPacketQueue(char* pData, __int32 clientIndex);

	void Run(unsigned __int32 maxClient, unsigned __int32 maxThread=0);
	void End();

private:
	void ProcessingPacket();
	void ProcessCreateTypePacket(Packet* pPacket);
	void ProcessChatPacket(Packet* pPacket);
	void ProcessPositionPacket(Packet* pPacket);
	void ProcessVoicePacket(Packet* pPacket);

private:
	bool m_processingPacket{ true };

	std::condition_variable m_cv{ };

	std::jthread m_procPacketThread{ };
	std::mutex m_packetLock{ };
	std::deque<Packet*> m_packetQueue{ };
	std::unordered_map<unsigned __int16, std::function<void(EchoServer&, Packet*)>> m_processFuncs{ };
};

// --------------------------------------------