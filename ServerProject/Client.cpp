#include "pch.h"
#include "Client.h"

Client::Client(__int32 index) : m_index{ index }, m_socket{ INVALID_SOCKET }  {
	m_recvBuffer = std::make_unique<char[]>(MAX_BUFFER_SIZE);
	ZeroMemory(std::addressof(m_recvIO), sizeof(OverlappedEx));
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

bool Client::SendPacketData(PacketHead* pPacket) {
	DWORD sendSize{ };
	DWORD flag{ };

	m_sendIO.buffer.len = pPacket->length;
	m_sendIO.buffer.buf = new char[pPacket->length];
	std::memcpy(m_sendIO.buffer.buf, pPacket, pPacket->length);
	m_sendIO.ioType = IO_TYPE::SEND;

	int sendResult{ ::WSASend(m_socket,
		std::addressof(m_sendIO.buffer),
		1,
		std::addressof(sendSize),
		0,
		reinterpret_cast<LPOVERLAPPED>(std::addressof(m_sendIO)),
		nullptr
	) };

	if (sendResult == SOCKET_ERROR and ::WSAGetLastError() == ERROR_IO_PENDING) {
		return false;
	}

	return true;
}

bool Client::BindRecv() {
	DWORD flag{ };
	DWORD recvSize{ };
	
	m_recvIO.buffer.len = MAX_BUFFER_SIZE;
	m_recvIO.buffer.buf = m_recvBuffer.get();
	m_recvIO.ioType = IO_TYPE::RECV;

	int recvResult{ ::WSARecv(m_socket,						// socket for receiving data
		std::addressof(m_recvIO.buffer),					// pointer of WSA buffer
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
	std::memset(std::addressof(m_sendIO), 0, sizeof(OverlappedEx));
}
