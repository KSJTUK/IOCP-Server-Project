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

bool NetworkClient::ConnectToServer(unsigned __int16 port, std::string_view serverIP) {
	sockaddr_in serverAddress{ };

	serverAddress.sin_family = PF_INET;
	serverAddress.sin_port = ::htons(port);
	serverAddress.sin_addr.s_addr = ::inet_addr(serverIP.data());
	if (::connect(m_socket, reinterpret_cast<sockaddr*>(std::addressof(serverAddress)), sizeof(sockaddr_in)) == SOCKET_ERROR) {
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

	m_processStruct.recvIO.wsaBuf.len = MAX_BUFFER_SIZE;
	m_processStruct.recvIO.wsaBuf.buf = m_processStruct.recvIO.buffer.data();
	m_processStruct.recvIO.ioType = IO_TYPE::RECV;

	int recvResult{ ::WSARecv(m_socket,
		std::addressof(m_processStruct.recvIO.wsaBuf),
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
	Session* pSession{ };

	while (true) {
		ioComplete = ::GetQueuedCompletionStatus(m_cpHandle,
			std::addressof(ioSize),
			reinterpret_cast<PULONG_PTR>(std::addressof(pSession)),
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

		IOData* data{ reinterpret_cast<IOData*>(pOverlapped) };
		if (data->ioType == IO_TYPE::RECV) {
			RecvComplete(data->buffer.data(), ioSize);
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

	m_processFuncs.insert(std::make_pair(CHAT_TYPE, &NetworkClient::ProcessChatPacket));
	m_processFuncs.insert(std::make_pair(POS_TYPE, &NetworkClient::ProcessPositionPacket));
}

void NetworkClient::RecvComplete(char* pData, size_t size) {
	std::lock_guard packetLock{ m_packetLock };

	std::unique_ptr<Packet> tempPacket{ PacketFacrory::CreatePacket(pData) };
	std::memcpy(DerivedCpyPointer(tempPacket.get()), pData, size);
	m_processFuncs[tempPacket->Type()](*this, tempPacket.get());
}

void NetworkClient::SendComplete() {
	m_processStruct.sendIO.BufClear();
}

void NetworkClient::PacketProcess() {
	while (m_packetProcRunning) {
		std::unique_lock lock{ m_packetLock };
		m_cv.wait(lock, [this]() { return !m_packetQueue.empty(); });
		
		SendPacket();

		lock.unlock();
	}
}

void NetworkClient::SendPacket() {
	MemoryBuf buf{ };

	DWORD ioSize{ };
	DWORD flag{ };

	Packet* pPacket{ m_packetQueue.front() };
	m_packetQueue.pop_front();

	std::memcpy(m_processStruct.sendIO.buffer.data(), DerivedCpyPointer(pPacket), pPacket->Length());
	m_processStruct.sendIO.wsaBuf.len = static_cast<ULONG>(pPacket->Length());
	delete pPacket;

	m_processStruct.sendIO.wsaBuf.buf = m_processStruct.sendIO.buffer.data();
	m_processStruct.sendIO.ioType = IO_TYPE::SEND;

	int sendResult{ ::WSASend(m_socket,
		std::addressof(m_processStruct.sendIO.wsaBuf),
		1,
		std::addressof(ioSize),
		flag,
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

void NetworkClient::InsertPacketQueue(char* pData) {
	std::lock_guard<std::mutex> packetLock{ m_packetLock };

	MemoryBuf buf{ };

	Packet* pPacket{ PacketFacrory::CreatePacket(pData) };
	buf.Write(pData, sizeof(pPacket->Length()));
	pPacket->Decode(buf);
	m_packetQueue.emplace_back(pPacket);

	m_cv.notify_one();
}

void NetworkClient::InsertPacketQueue(Packet* pPacket) {
	std::lock_guard<std::mutex> packetLock{ m_packetLock };

	m_packetQueue.emplace_back(pPacket);
	m_cv.notify_one();
}

void NetworkClient::ProcessChatPacket(Packet* pPacket) {
	ConsoleIO::OutputString(pPacket->PrintPacket());
}

void NetworkClient::ProcessPositionPacket(Packet* pPacket) {
	ConsoleIO::OutputString(pPacket->PrintPacket());
}
