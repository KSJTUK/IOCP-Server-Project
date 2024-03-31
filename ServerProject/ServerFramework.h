#pragma once

#include "Client.h"

class ServerFramework abstract {
public:
	explicit ServerFramework();
	virtual ~ServerFramework();

public:
	bool BindAndListen(const unsigned __int16 port);
	bool StartServer(const unsigned __int32 maxClient, unsigned __int32 maxThread = 0);

	Client& GetClient(__int32 clientIndex);

	virtual void Connect(__int32 clientIndex) PURE;
	virtual void Receive(__int32 clientIndex, std::string_view recvMessage) PURE;
	virtual void Receive(__int32 clientIndex, std::size_t dataSize, const char* recvData) PURE;
	virtual void Close(__int32 clinetIndex) PURE;

private:
	void CreateClients(const unsigned __int32 maxClient);
	bool CreateWorkThread(unsigned __int32 maxThread);
	bool CreateAcceptThread();

	void WorkThread();
	void AcceptThread();

	std::optional<std::reference_wrapper<Client>> GetEmptyClient();

private:
	SOCKET m_listeningSocket{ INVALID_SOCKET };

	HANDLE m_cpHandle{ nullptr };

	unsigned __int32 m_connectedClientSize{ };
	std::vector<Client> m_clients{ };

	std::vector<std::jthread> m_workThreads{ };
	std::jthread m_acceptThread{ };

	bool m_workThreadRunning{ true };
	bool m_acceptThreadRunning{ true };
};

// --------------------------------------------
class EchoServer : public ServerFramework {
public:
	virtual void Connect(__int32 clientIndex) override { };
	virtual void Receive(__int32 clientIndex, std::string_view recvMessage) { };
	virtual void Receive(__int32 clientIndex, std::size_t dataSize, const char* recvData) override { };
	virtual void Close(__int32 clientIndex) override { };

	//bool SendMsg(__int32 clientIndex, DWORD dataSize, const char* data);
	bool SendMsg(__int32 clientIndex, std::string_view message);

	void Run(unsigned __int32 maxClient, unsigned __int32 maxThread);
	void End();

private:
	void ProcessingPacket();
	ChatPacket DequePacketData();

private:
	bool m_processingPacket{ };
	std::jthread m_procPacketThread{ };
	std::mutex m_packetLock{ };
	std::deque<ChatPacket> m_packetData{ };
};