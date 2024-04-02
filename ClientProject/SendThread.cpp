#include "pch.h"
#include "SendThread.h"

void SendThread::BindSocket(const SOCKET& socket) {
	m_socket = socket;
}

void SendThread::BindEvent(HANDLE eventHandle) {
	m_sendIO.overlapped.hEvent = eventHandle;
}

SendThread::~SendThread() {
	m_processing = false;
}

void SendThread::StartSendingProcess(const SOCKET& socket, HANDLE eventHandle) {
	BindSocket(socket);
	BindEvent(eventHandle);
	m_sendProcessThread = std::jthread{ [this]() { SendProcessing(); } };
}

void SendThread::InsertPacket(ChatPacket& packet) {
	m_packetQueue.emplace_back(packet);
}

void SendThread::InsertPacket(std::string_view msg) {
	ChatPacket packet{ static_cast<short>(msg.size()) };
	std::copy(msg.begin(), msg.end(), packet.msg);
	m_packetQueue.emplace_back(packet);
}

void SendThread::SendProcessing() {
	while (m_processing) {
		ChatPacket packet{ std::move(DequePacket()) };
		if (packet.length != 0) {
			SendMsg(packet.msg);
		}
		else {
			std::this_thread::yield();
		}
	}
}

ChatPacket SendThread::DequePacket() {
	if (m_packetQueue.empty()) {
		return ChatPacket{ };
	}
	ChatPacket packet{ std::move(m_packetQueue.front()) };
	m_packetQueue.pop_front();
	return packet;
}

bool SendThread::SendMsg(std::string_view msg) {
	m_sendIO.buffer.len = static_cast<ULONG>(msg.size());
	m_sendIO.buffer.buf = const_cast<char*>(msg.data());

	if (::WSASend(m_socket, std::addressof(m_sendIO.buffer), 1, std::addressof(m_ioSize), 0, reinterpret_cast<LPOVERLAPPED>(std::addressof(m_sendIO)), nullptr) == SOCKET_ERROR) {
		if (::WSAGetLastError() != WSA_IO_PENDING) {
			std::cout << "Sending Error! Error Code: " << ::WSAGetLastError() << std::endl;
			return false;
		}
	}
	return true;
}

void SendThread::SendComplete() {
	DWORD resultFlag{ };
	DWORD resultSize{ };
	::WSAGetOverlappedResult(m_socket, reinterpret_cast<LPOVERLAPPED>(std::addressof(m_sendIO)), std::addressof(resultSize), FALSE, std::addressof(resultFlag));
	std::cout << std::format("Àü¼Û byte: {}\n", resultSize);
	

	std::memset(std::addressof(m_sendIO), 0, sizeof(OverlappedEx));
}
