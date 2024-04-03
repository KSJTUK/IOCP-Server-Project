#pragma once

struct ProcessPacket {
	OverlappedEx sendIO{ };
	OverlappedEx recvIO{ };

	char recvBuffer[MAX_BUFFER_SIZE]{ };
	char sendBuffer[MAX_BUFFER_SIZE]{ };
};

class MessageHandler {
	
	OverlappedEx sendIO{ };
	OverlappedEx recvIO{ };
	char m_recvBuffer[MAX_BUFFER_SIZE]{ };
};

class NetworkClient {
public:
	NetworkClient();
	~NetworkClient();

public:
	void InsertPacketQueue(std::string_view msg);

	bool ConnectToServer(unsigned __int16 port, std::string_view serverIP);
	bool BindIOCP();
	bool BindRecv();

	bool MainThread();
	void StartServer();

	void SendComplete();

	void PacketProcess();
	void SendMsg();

	void Run();

private:

private:
	SOCKET m_socket{ };
	HANDLE m_cpHandle{ };
	std::condition_variable m_cv{ };

	bool m_packetProcRunning{ true };

	std::jthread m_mainThread{ };
	std::jthread m_packetProcThread{ };

	std::mutex m_packetLock{ };
	std::deque<ChatPacket> m_packetQueue{ };

	ProcessPacket m_processStruct{ };
};