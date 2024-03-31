#pragma once

inline constexpr __int32 MAX_BUFFER_SIZE{ 1024 };

class Client {
public:
	Client(__int32 index);
	~Client();

public:
	__int32 GetIndex() const { return m_index; }
	SOCKET GetSocket() const { return m_socket; }
	const char* GetRecvBuffer() const { return m_recvBuffer; }

public:
	bool Connect(HANDLE cpHandle, SOCKET socket);
	bool BindIOCP(HANDLE cpHandle);

	bool SendMsg(std::string_view message);
	bool BindRecv();

	void CloseSocket(bool forcedClose = false);
	void SendComplete(DWORD sendSize);

private:
	__int32 m_index{ };

	SOCKET m_socket{ INVALID_SOCKET };
	OverlappedEx m_recvIO{ };

	char m_recvBuffer[MAX_BUFFER_SIZE]{ };
	char m_sendBuffer[MAX_BUFFER_SIZE]{ };
};

