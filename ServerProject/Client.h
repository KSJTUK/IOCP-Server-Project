#pragma once

inline constexpr __int32 MAX_BUFFER_SIZE{ 1024 };

class Client {
public:
	Client(__int32 index);
	~Client();

public:
	__int32 GetIndex() const { return m_index; }
	SOCKET GetSocket() const { return m_socket; }

public:
	bool Connect(HANDLE cpHandle, SOCKET socket);
	bool BindIOCP(HANDLE cpHandle);

	bool SendPacketData(PacketHead* pPacket);
	bool BindRecv();

	void CloseSocket(bool forcedClose = false);
	void SendComplete(DWORD sendSize);

private:
	__int32 m_index{ };

	SOCKET m_socket{ INVALID_SOCKET };
	OverlappedEx m_recvIO{ };
	OverlappedEx m_sendIO{ };

	std::unique_ptr<char[]> m_recvBuffer{ };
};

