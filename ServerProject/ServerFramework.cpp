#include "pch.h"
#include "ServerFramework.h"

ServerFramework::ServerFramework() {
	WSADATA wsaData{ };
	if (::WSAStartup(MAKEWORD(2, 2), std::addressof(wsaData))) {
		std::cout << std::format("[Fatal Error] WSAStartup() Function Fail, Error Code: {}\n", ::WSAGetLastError());
		return;
	}

	m_listeningSocket = ::WSASocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, NULL, 0);
	if (m_listeningSocket == INVALID_SOCKET) {
		std::cout << std::format("[Fatal Error] WSASocket() Fucntion Fail (listening socket create fail), Error Code: {}\n", ::WSAGetLastError());
		return;
	}
}

ServerFramework::~ServerFramework() {
	m_workThreadRunning = false;
	::CloseHandle(m_cpHandle);

	m_acceptThreadRunning = false;
	::closesocket(m_listeningSocket);

	::WSACleanup();
}

bool ServerFramework::BindAndListen(const unsigned __int16 port) {
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

bool ServerFramework::StartServer(const unsigned __int32 maxClient, unsigned __int32 maxThread) {
	CreateClients(maxClient);
	m_cpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, NULL, maxThread);
	if (not m_cpHandle) {
		std::cout << std::format("[Fatal Error] New Completion Port Creating Fail, Error Code: {}", ::GetLastError());
		return false;
	}

	bool threadCreateSuccess{ CreateWorkThread(maxThread) };
	if (not threadCreateSuccess) {
		std::cout << std::format("[Fatal Error] Server Worker Thread Creating Fail\n");
		return false;
	}

	threadCreateSuccess = CreateAcceptThread();
	if (not threadCreateSuccess) {
		std::cout << std::format("[Fatal Error] Server Accept Thread Creating Fail\n");
		return false;
	}

	std::cout << "Server Start...\n";
	return true;
}

Client& ServerFramework::GetClient(__int32 clientIndex) {
	return m_clients[clientIndex];
}

void ServerFramework::CreateClients(const unsigned __int32 maxClient) {
	for (unsigned __int32 i = 0; i < maxClient; ++i) {
		m_clients.emplace_back(i);
	}
}

bool ServerFramework::CreateWorkThread(unsigned __int32 maxThread) {
	for (unsigned __int32 i = 0; i < maxThread; ++i) {
		m_workThreads.emplace_back([this]() { WorkThread(); });
	}
	return true;
}

bool ServerFramework::CreateAcceptThread() {
	m_acceptThread = std::move(std::jthread{ [this]() { AcceptThread(); } });
	return true;
}

void ServerFramework::WorkThread() {
	bool ioComplete{ };
	DWORD ioSize{ };
	LPOVERLAPPED overlapped{ };
	Client client{ 0 };

	while (true) {
		// ::GetQueuedCompletionStatus return -> TRUE: i/o complete -> thread awake, FALSE: waiting time over -> thread sleep again
		ioComplete = ::GetQueuedCompletionStatus(
			m_cpHandle,												// completion port handle
			std::addressof(ioSize),									// pointer of byte size to save completed io
			reinterpret_cast<SOCKET*>(std::addressof(client)),						// pointer of context to save info of completed io
			std::addressof(overlapped),								// overlaepped struct pointer for async io work
			INFINITE												// Waiting time
		);

		// terminate client Thread
		if (ioComplete and ioSize == 0 and overlapped == nullptr) {
			m_workThreadRunning = false;
			continue;
		}

		if (overlapped == nullptr) {
			continue;
		}


		// client disconnected
		if (not ioComplete or (ioComplete and ioSize == 0)) {
			client.CloseSocket();
			continue;
		}

		OverlappedEx overlappedEx{ std::move(*overlapped) };

		// After Recv complete
		if (overlappedEx.ioType == IO_TYPE::RECV) {
			Receive(client.GetIndex(), client.GetRecvBuffer());
			client.BindRecv();
		}
		// After Send complete
		else if (overlappedEx.ioType == IO_TYPE::SEND) {
			client.SendComplete(ioSize);
		}
		else {
			// exception
			std::cout << std::format("[Exception] socket: {}\n", client.GetSocket());
		}
	}
}

void ServerFramework::AcceptThread() {
	sockaddr_in clientAddress{ };
	int addressLength{ sizeof(sockaddr_in) };

	while (m_acceptThreadRunning) {
		auto optionalClient{ GetEmptyClient() };
		if (not optionalClient.has_value()) {
			std::cout << std::format("[Client Connect Fail] Client Full: Current Client( {} )\n", m_connectedClientSize);
			return;
		}

		Client& client{ optionalClient.value().get() };

		SOCKET socket = ::accept(m_listeningSocket, reinterpret_cast<sockaddr*>(std::addressof(clientAddress)), std::addressof(addressLength));
		if (socket == INVALID_SOCKET) {
			continue;
		}

		bool callSuccess{ client.BindIOCP(m_cpHandle, socket) };
		if (not callSuccess) {
			std::cout << std::format("[Exception] Client Socket bind fail, Error Code: {}\n", ::WSAGetLastError());
			return;
		}

		callSuccess = client.BindRecv();
		if (not callSuccess) {
			return;
		}

		char clientIP[INET_ADDRSTRLEN]{ };
		::inet_ntop(PF_INET, std::addressof(clientAddress.sin_addr), clientIP, INET_ADDRSTRLEN);
		std::cout << std::format("Client [IP: {} | SOCET: {}] is connected\n", clientIP, client.GetSocket());
	}
	++m_connectedClientSize;
}

std::optional<std::reference_wrapper<Client>> ServerFramework::GetEmptyClient() {
	for (auto& client : m_clients) {
		if (client.GetSocket() == INVALID_SOCKET) {
			return client;
		}
	}
	return std::nullopt;
}

bool EchoServer::SendMsg(__int32 clientIndex, std::string_view message) {
	Client& client{ GetClient(clientIndex) };
	return client.SendMsg(message);
}
