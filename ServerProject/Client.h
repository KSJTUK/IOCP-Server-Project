#pragma once

class Session {
public:
	Session(__int32 index);
	Session(const Session&& other) noexcept;
	~Session();

public:
	__int32 GetIndex() const { return m_index; }
	SOCKET GetSocket() const { return m_socket; }

public:
	bool Connect(HANDLE cpHandle, SOCKET socket);
	bool BindIOCP(HANDLE cpHandle);

	bool SendPacketData(Packet* pPacket);
	bool BindRecv();

	void CloseSocket(bool forcedClose = false);
	void SendComplete(DWORD sendSize);

private:
	__int32 m_index{ };

	SOCKET m_socket{ INVALID_SOCKET };
	IOData m_recvIO{ };
	IOData m_sendIO{ };
};

