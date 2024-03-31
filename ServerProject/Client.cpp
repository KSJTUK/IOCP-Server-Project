#include "pch.h"
#include "Client.h"

Client::Client(__int32 index) : m_index{ index }, m_socket{ INVALID_SOCKET }  {
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

bool Client::SendMsg(std::string_view message) {
	DWORD sendSize{ };
	DWORD flag{ };

	if (message.size() >= MAX_BUFFER_SIZE) {
		std::cout << std::format("[Send Fail] Send Message Over {} bytes", MAX_BUFFER_SIZE);
		return false;
	}

	OverlappedEx sendIO{ };
	ZeroMemory(std::addressof(sendIO), sizeof(OverlappedEx));
	std::copy(message.begin(), message.end(), m_sendBuffer);
	sendIO.buffer.len = static_cast<ULONG>(message.size());
	sendIO.buffer.buf = m_sendBuffer;	
	sendIO.ioType = IO_TYPE::SEND;

	int sendResult{ ::WSASend(m_socket,
		std::addressof(sendIO.buffer),
		1,
		std::addressof(sendSize),
		0,
		std::addressof(sendIO.overlapped),
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
	m_recvIO.buffer.buf = m_recvBuffer;
	m_recvIO.ioType = IO_TYPE::RECV;

	int recvResult{ ::WSARecv(m_socket,						// socket for receiving data
		std::addressof(m_recvIO.buffer),		// pointer of WSA buffer
		1,											// num of recv buffer 
		std::addressof(recvSize),
		std::addressof(flag) ,
		std::addressof(m_recvIO.overlapped),
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
	std::memset(m_sendBuffer, 0, MAX_BUFFER_SIZE);
}
