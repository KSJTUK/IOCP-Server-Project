#include "pch.h"
#include "ServerFramework.h"

NetworkServer::NetworkServer() {
	WSADATA wsaData{ };
	if (::WSAStartup(MAKEWORD(2, 2), std::addressof(wsaData))) {
		std::cout << std::format("[Fatal Error] WSAStartup() Function Fail, Error Code: {}\n", ::WSAGetLastError());
		return;
	}

	m_listeningSocket = ::WSASocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, NULL, WSA_FLAG_OVERLAPPED);
	if (m_listeningSocket == INVALID_SOCKET) {
		std::cout << std::format("[Fatal Error] WSASocket() Fucntion Fail (listening socket create fail), Error Code: {}\n", ::WSAGetLastError());
		return;
	}
}

NetworkServer::~NetworkServer() {
	m_workThreadRunning = false;
	::CloseHandle(m_cpHandle);

	m_acceptThreadRunning = false;
	::closesocket(m_listeningSocket);

	::WSACleanup();
}

bool NetworkServer::BindAndListen(const unsigned __int16 port) {
	sockaddr_in serverAddress{ };
	serverAddress.sin_family = PF_INET;
	serverAddress.sin_port = ::htons(port);
	serverAddress.sin_addr.s_addr = ::htons(INADDR_ANY);
	if (::bind(m_listeningSocket, reinterpret_cast<sockaddr*>(std::addressof(serverAddress)), sizeof(sockaddr_in)) == SOCKET_ERROR) {
		std::cout << std::format("[Socket Error] bind() function fail, Error Code: {}\n", ::WSAGetLastError());
		return false;
	}
	
	if (::listen(m_listeningSocket, SOMAXCONN) == SOCKET_ERROR) {
		std::cout << std::format("[Socket Error] listen() function fail, Error Code: {}\n", ::WSAGetLastError());
		return false;
	}

	return true;
}

bool NetworkServer::StartServer(const unsigned __int32 maxClient, unsigned __int32 maxThread) {
	CreateClients(maxClient);
	m_cpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, NULL, maxThread);
	if (not m_cpHandle) {
		std::cout << std::format("[Fatal Error] New Completion Port Creating Fail, Error Code: {}", ::GetLastError());
		return false;
	}

	bool threadCreateSuccess{ RunIOWorkThread(maxThread) };
	if (not threadCreateSuccess) {
		std::cout << std::format("[Fatal Error] Server Worker Thread Creating Fail\n");
		return false;
	}

	threadCreateSuccess = RunAcceptThread();
	if (not threadCreateSuccess) {
		std::cout << std::format("[Fatal Error] Server Accept Thread Creating Fail\n");
		return false;
	}

	std::cout << "Server Start...\n";
	return true;
}

Client& NetworkServer::GetClient(__int32 clientIndex) {
	return m_clients[clientIndex];
}

void NetworkServer::CreateClients(const unsigned __int32 maxClient) {
	for (unsigned __int32 i = 0; i < maxClient; ++i) {
		m_clients.emplace_back(i);
	}
}

bool NetworkServer::RunIOWorkThread(unsigned __int32 maxThread) {
	for (unsigned __int32 i = 0; i < maxThread; ++i) {
		m_workThreads.emplace_back([this]() { WorkThread(); });
	}
	return true;
}

bool NetworkServer::RunAcceptThread() {
	m_acceptThread = std::jthread{ [this]() { AcceptThread(); } };
	return true;
}

void NetworkServer::WorkThread() {
	bool ioComplete{ true };
	DWORD ioSize{ };
	LPOVERLAPPED pOverlapped{ };
	Client* pSession{ nullptr };

	while (m_workThreadRunning) {
		// ::GetQueuedCompletionStatus return -> TRUE: i/o complete -> thread awake, FALSE: waiting time over -> thread sleep again
		ioComplete = ::GetQueuedCompletionStatus(
			m_cpHandle,												// completion port handle
			std::addressof(ioSize),									// pointer of byte size to save completed io
			reinterpret_cast<PULONG_PTR>(std::addressof(pSession)),	// pointer of context to save info of completed io
			std::addressof(pOverlapped),								// overlaepped struct pointer for async io work
			INFINITE												// Waiting time
		);

		// terminate client Thread
		if (ioComplete and ioSize == 0 and pOverlapped == nullptr) {
			m_workThreadRunning = false;
			continue;
		}

		if (pOverlapped == nullptr) {
			continue;
		}

		// client disconnected
		if (not ioComplete or (ioComplete and ioSize == 0)) {
			Close(pSession->GetIndex());
			pSession->CloseSocket();
			continue;
		}

		IOData* pOverlappedEx = reinterpret_cast<IOData*>(pOverlapped);

		// After Recv complete
		if (pOverlappedEx->ioType == IO_TYPE::RECV) {
			Receive(pSession->GetIndex(), static_cast<size_t>(ioSize), pOverlappedEx->buffer.data());
			pSession->BindRecv();
		}
		// After Send complete
		else if (pOverlappedEx->ioType == IO_TYPE::SEND) {
			pSession->SendComplete(ioSize);
		}
		else {
			// exception
			ConsoleIO::OutputString(std::format("[Exception] socket: {}\n", pSession->GetSocket()));
		}
	}
}

void NetworkServer::AcceptThread() {
	sockaddr_in clientAddress{ };
	int addressLength{ sizeof(sockaddr_in) };

	while (m_acceptThreadRunning) {
		auto optionalClient{ GetUnConnectedClient() };
		if (not optionalClient.has_value()) {
			ConsoleIO::OutputString(std::format("[Client Connect Fail] Client Full: Current Client( {} )\n", m_connectedClientSize));
			continue;
		}

		Client& client{ optionalClient.value().get() };

		SOCKET socket{ ::accept(m_listeningSocket, reinterpret_cast<sockaddr*>(std::addressof(clientAddress)), std::addressof(addressLength)) };
		if (socket == INVALID_SOCKET) {
			continue;
		}

		bool callSuccess{ client.Connect(m_cpHandle, socket) };
		if (not callSuccess) {
			client.CloseSocket(true);
			return;
		}

		char clientIP[INET_ADDRSTRLEN]{ };
		::inet_ntop(PF_INET, std::addressof(clientAddress.sin_addr), clientIP, INET_ADDRSTRLEN);
		//::getpeername(socket, reinterpret_cast<sockaddr*>(std::addressof(clientAddress)), std::addressof(addressLength));

		TimeUtil::PrintTime();
		ConsoleIO::OutputString(std::format(" Client [IP: {} | SOCKET: {} | index: {}] is connected\n", clientIP, client.GetSocket(), client.GetIndex()));

		++m_connectedClientSize;
	}
}

std::optional<std::reference_wrapper<Client>> NetworkServer::GetUnConnectedClient() {
	for (auto& client : m_clients) {
		if (client.GetSocket() == INVALID_SOCKET) {
			return client;
		}
	}
	return std::nullopt;
}

void EchoServer::Receive(__int32 clientIndex, std::size_t recvByte, char* pRecvData) {
	std::lock_guard<std::mutex> packetGuard(m_packetLock);

	m_timer.SetTimeStamp();

	Packet* packet{ PacketFacrory::CreatePacket(pRecvData) };
	std::memcpy(DerivedCpyPointer(packet), pRecvData, recvByte);
	packet->SetFrom(clientIndex);
	m_packetQueue.emplace_back(packet);

	TimeUtil::PrintTime();
	ConsoleIO::OutputString("  Recv - " + (packet->PrintPacket()) + "\n");
	m_cv.notify_one();
}

void EchoServer::Close(__int32 clientIndex) {
	Client& client{ GetClient(clientIndex) };
	sockaddr_in clientAddress{ };
	int addressLength{ sizeof(sockaddr_in) };

	char clientIP[INET_ADDRSTRLEN]{ };
	::inet_ntop(PF_INET, std::addressof(clientAddress.sin_addr), clientIP, INET_ADDRSTRLEN);
	ConsoleIO::OutputString(std::format("Client [SOCKET: {} | index: {}] is disconnected\n", client.GetSocket(), client.GetIndex()));
}

bool EchoServer::SendPacket(__int32 clientIndex, Packet* pPacket) {
	Client& client{ GetClient(clientIndex) };
	bool rtVal = client.SendPacketData(pPacket);
	delete pPacket;
	return rtVal;
}

bool EchoServer::SendAll(__int32 clientIndex, Packet* pPacket) {
	for (auto& client : m_clients) {
		if (client.GetSocket() != INVALID_SOCKET) {
			client.SendPacketData(pPacket);
		}
	}
	delete pPacket;
	pPacket = nullptr;
	return true;
}

void EchoServer::InsertPacketQueue(char* pData, __int32 clientIndex) {
	Packet* pPacket{ PacketFacrory::CreatePacket(pData) };
	std::memcpy(DerivedCpyPointer(pPacket), pData, pPacket->Length());
	pPacket->SetFrom(clientIndex);
	m_packetQueue.emplace_back(pPacket);

	m_cv.notify_one();
}

void EchoServer::ProcessingPacket() {
	while (m_processingPacket) {
		std::unique_lock lock{ m_packetLock };

		m_cv.wait(lock, [this]() { return !m_packetQueue.empty(); });

		Packet* pPacket{ std::move(m_packetQueue.front()) };
		m_packetQueue.pop_front();
		m_processFuncs[pPacket->Type()](*this, pPacket);

		//SendPacket(pPacket->From(), pPacket);
		SendAll(pPacket->From(), pPacket);
		
		lock.unlock();
	}
}

void EchoServer::ProcessChatPacket(Packet* pPacket) {
}

void EchoServer::ProcessPositionPacket(Packet* pPacket) {
}

void EchoServer::ProcessVoicePacket(Packet* pPacket) {
}

void EchoServer::Run(unsigned __int32 maxClient, unsigned __int32 maxThread) {
	m_procPacketThread = std::jthread{ [this]() { ProcessingPacket(); } };

	m_processFuncs.insert(std::make_pair(CHAT_TYPE, &EchoServer::ProcessChatPacket));
	m_processFuncs.insert(std::make_pair(POS_TYPE, &EchoServer::ProcessPositionPacket));
	m_processFuncs.insert(std::make_pair(VOICE_TYPE, &EchoServer::ProcessVoicePacket));

	StartServer(maxClient, maxThread);
}

void EchoServer::End() {
	m_processingPacket = false;
}
