#pragma once

class Client {
public:
	Client(__int32 index);
	Client(const Client&& other) noexcept;
	virtual ~Client();

public:
	__int32 GetIndex() const { return m_index; }
	SOCKET GetSocket() const { return m_socket; }

public:
	void SetId(const std::string_view id) { m_id = id; }
	std::string_view GetId() const { return m_id; }

	bool Connect(HANDLE cpHandle, SOCKET socket);
	bool BindIOCP(HANDLE cpHandle);

	bool SendPacketData(class Packet* pPacket);
	bool BindRecv();

	void CloseSocket(bool forcedClose = false);
	void SendComplete(DWORD sendSize);

protected:
	__int32 m_index{ };

	std::string m_id{ };

	SOCKET m_socket{ INVALID_SOCKET };
	IOData m_recvIO{ };
	IOData m_sendIO{ };
};