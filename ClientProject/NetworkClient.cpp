#include "pch.h"
#include "NetworkClient.h"

NetworkClient::NetworkClient() {
	WSADATA wsaData{ };
	if (::WSAStartup(MAKEWORD(2, 2), std::addressof(wsaData))) {
		std::cout << std::format("[Fatal Error] WSAStartup Function Fail, Error Code: {}\n", ::WSAGetLastError());
		return;
	}

	m_socket = ::WSASocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, NULL, WSA_FLAG_OVERLAPPED);
	if (m_socket == INVALID_SOCKET) {
		std::cout << std::format("[Fatal Error] Socket Create Fail, Error Code: {}\n", ::WSAGetLastError());
		return;
	}

	m_cpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, NULL, 1);
	if (m_cpHandle == nullptr) {
		std::cout << std::format("[Fatal Error] I/O Completion Port Create Fail, Error Code: {}\n", ::GetLastError());
		return;
	}
}

NetworkClient::~NetworkClient() {
	::closesocket(m_socket);
	::CloseHandle(m_cpHandle);

	::WSACleanup();
}

void NetworkClient::InsertPacketQueue(std::string_view msg) {
	std::lock_guard<std::mutex> packetLock{ m_packetLock };
	ChatPacket packet{ static_cast<short>(msg.size()) };
	std::copy(msg.begin(), msg.end(), packet.msg);
	m_packetQueue.emplace_back(packet);

	m_cv.notify_one();
}

bool NetworkClient::ConnectToServer(unsigned __int16 port, std::string_view serverIP) {
	sockaddr_in serverAddress{ };
	serverAddress.sin_family = PF_INET;
	serverAddress.sin_port = ::htons(port);
	serverAddress.sin_addr.s_addr = ::inet_addr(serverIP.data());
	if (::connect(m_socket, reinterpret_cast<sockaddr*>(std::addressof(serverAddress)), sizeof(sockaddr_in))) {
		std::cout << std::format("[Fatal Error] Server Connecting Fail!, Error Code: {}\n", ::WSAGetLastError());
		return false;
	}
	return true;
}

bool NetworkClient::BindIOCP() {
	HANDLE cpHandle{ ::CreateIoCompletionPort(
		reinterpret_cast<HANDLE>(m_socket),
		m_cpHandle,
		reinterpret_cast<ULONG_PTR>(std::addressof(m_processStruct)),
		1
	) };

	if (cpHandle == nullptr or cpHandle != m_cpHandle) {
		std::cout << std::format("[Fatal Error] I/O Completion Port Binding Fail, Error Code: {}\n", ::GetLastError());
		return false;
	}

	return true;
}

bool NetworkClient::BindRecv() {
	DWORD ioSize{ };
	DWORD flag{ };

	std::memset(std::addressof(m_processStruct.recvIO), 0, sizeof(OverlappedEx));
	std::memset(m_processStruct.recvBuffer, 0, MAX_BUFFER_SIZE);

	m_processStruct.recvIO.buffer.len = MAX_BUFFER_SIZE;
	m_processStruct.recvIO.buffer.buf = m_processStruct.recvBuffer;
	m_processStruct.recvIO.ioType = IO_TYPE::RECV;

	int recvResult{ ::WSARecv(m_socket,
		std::addressof(m_processStruct.recvIO.buffer),
		1,
		std::addressof(ioSize),
		std::addressof(flag),
		reinterpret_cast<LPOVERLAPPED>(std::addressof(m_processStruct.recvIO)),
		NULL
	) };

	if (recvResult == SOCKET_ERROR and ::WSAGetLastError() != WSA_IO_PENDING) {
		std::cout << std::format("[Exception] Receive Bidning Fail, Error Code: {}\n", ::WSAGetLastError());
		return false;
	}

	return true;
}

bool NetworkClient::MainThread() {
	bool ioComplete{ };
	DWORD ioSize{ };
	LPOVERLAPPED pOverlapped{ };
	ProcessPacket* pProcPacket{ };

	while (true) {
		ioComplete = ::GetQueuedCompletionStatus(m_cpHandle,
			std::addressof(ioSize),
			reinterpret_cast<PULONG_PTR>(std::addressof(pProcPacket)),
			std::addressof(pOverlapped),
			INFINITE
		);

		if (ioComplete and ioSize == 0 and pOverlapped == nullptr) {
			continue;
		}

		if (pOverlapped == nullptr) {
			continue;
		}

		if (not ioComplete and (ioComplete and ioSize == 0)) {
			return false;
		}

		OverlappedEx* data{ reinterpret_cast<OverlappedEx*>(pOverlapped) };
		if (data->ioType == IO_TYPE::RECV) {
			std::cout << std::format("¼ö½Å byte{}: {}\n", ioSize, data->buffer.buf);
			BindRecv();
		}
		else if (data->ioType == IO_TYPE::SEND) {
			SendComplete();
		}
		else {

		}
	}
}

void NetworkClient::StartServer() {
	BindIOCP();
	BindRecv();

	m_packetProcThread = std::jthread{ [this]() { PacketProcess(); } };
	m_mainThread = std::jthread{ [this]() { MainThread();  } };
}

void NetworkClient::SendComplete() {
	std::memset(m_processStruct.sendBuffer, 0, MAX_BUFFER_SIZE);
}

void NetworkClient::PacketProcess() {
	while (m_packetProcRunning) {
		std::unique_lock lock{ m_packetLock };
		m_cv.wait(lock, [this]() { return !m_packetQueue.empty(); });
		
		SendMsg();

		lock.unlock();
	}
}

void NetworkClient::SendMsg() {
	DWORD ioSize{ };
	DWORD flag{ };

	ChatPacket packet{ std::move(m_packetQueue.front()) };
	m_packetQueue.pop_front();
	
	std::memset(std::addressof(m_processStruct.sendIO), 0, sizeof(OverlappedEx));

	std::memcpy(m_processStruct.sendBuffer, packet.msg, packet.length);
	m_processStruct.sendIO.buffer.len = static_cast<ULONG>(packet.length);
	m_processStruct.sendIO.buffer.buf = m_processStruct.sendBuffer;
	m_processStruct.sendIO.ioType = IO_TYPE::SEND;

	int sendResult{ ::WSASend(m_socket,
		std::addressof(m_processStruct.sendIO.buffer),
		1,
		std::addressof(ioSize),
		0,
		reinterpret_cast<LPOVERLAPPED>(std::addressof(m_processStruct.sendIO)),
		nullptr
	) };

	if (sendResult == SOCKET_ERROR and ::WSAGetLastError() != WSA_IO_PENDING) {
		std::cout << std::format("[Exception] Send Reserve Fail, Error Code: {}\n", ::WSAGetLastError());
	}
}

void NetworkClient::Run() {
	StartServer();
}
