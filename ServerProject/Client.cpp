#include "pch.h"
#include "Client.h"
#include "Packet.h"

Client::Client(__int32 index) : m_index{ index }, m_socket{ INVALID_SOCKET } {
	ZeroMemory(std::addressof(m_recvIO), sizeof(IOData));
}

Client::Client(const Client&& other) noexcept : m_index{ other.m_index }, m_socket{ other.m_socket } {
}

Client::~Client() { }

bool Client::Connect(HANDLE cpHandle, SOCKET socket) {
	m_socket = socket;

	if (not BindIOCP(cpHandle)) {
		return false;
	}
	return BindRecv();
}

bool Client::BindIOCP(HANDLE bindHandle) {
	HANDLE cpHandle{ ::CreateIoCompletionPort(
		reinterpret_cast<HANDLE>(m_socket),
		bindHandle,
		reinterpret_cast<ULONG_PTR>(this),
		0
	) };

	if (not cpHandle or cpHandle != bindHandle) {
		std::cout << std::format("[Exception] Io Completed Port Create Fail, Error Code: {}\n", ::GetLastError());
		return false;
	}

	return true;
}

bool Client::SendPacketData(Packet* pPacket) {
	DWORD sendSize{ };
	DWORD flag{ };

	std::memcpy(m_sendIO.buffer.data(), DerivedCpyPointer(pPacket), pPacket->Length());
	m_sendIO.wsaBuf.len = static_cast<ULONG>(pPacket->Length());

	m_sendIO.wsaBuf.buf = m_sendIO.buffer.data();
	m_sendIO.ioType = IO_TYPE::SEND;

	int sendResult{ ::WSASend(m_socket,
		std::addressof(m_sendIO.wsaBuf),
		1,
		std::addressof(sendSize),
		0,
		reinterpret_cast<LPOVERLAPPED>(std::addressof(m_sendIO)),
		nullptr
	) };

	if (sendResult == SOCKET_ERROR and ::WSAGetLastError() == ERROR_IO_PENDING) {
		std::cout << std::format("[Exception] Send Fail, Error Code: {}\n", ::WSAGetLastError());
		return false;
	}

	return true;
}

bool Client::BindRecv() {
	DWORD flag{ };
	DWORD recvSize{ };
	
	m_recvIO.wsaBuf.len = MAX_BUFFER_SIZE;
	m_recvIO.wsaBuf.buf = m_recvIO.buffer.data();
	m_recvIO.ioType = IO_TYPE::RECV;

	int recvResult{ ::WSARecv(m_socket,						// socket for receiving data
		std::addressof(m_recvIO.wsaBuf),					// pointer of WSA buffer
		1,													// num of recv buffer 
		std::addressof(recvSize),
		std::addressof(flag) ,
		reinterpret_cast<LPOVERLAPPED>(std::addressof(m_recvIO)),
		nullptr
		) };

	if (recvResult == SOCKET_ERROR and ::WSAGetLastError() != ERROR_IO_PENDING) {
		std::cout << std::format("[Exception] WSARecv Function Fail, Error Code: {}\n", ::WSAGetLastError());
		return false;
	}

	return true;
}

void Client::CloseSocket(bool forcedClose) {
	static linger staticLinger{ 0, 0 };
	static __int32 lingerSize{ sizeof(linger) };

	staticLinger.l_onoff = forcedClose;

	// forced teminate receiving, sending datas
	::shutdown(m_socket, SD_BOTH);
	::setsockopt(m_socket, SOL_SOCKET, SO_LINGER, reinterpret_cast<const char*>(std::addressof(staticLinger)), lingerSize);
	::closesocket(m_socket);

	m_socket = INVALID_SOCKET;
}

void Client::SendComplete(DWORD sendSize) {
	std::memset(std::addressof(m_sendIO), 0, sizeof(IOData));
}