#pragma once

class SendThread {
public:
    ~SendThread();

public:
    void StartSendingProcess(const SOCKET& socket, HANDLE eventHandle);

    void InsertPacket(ChatPacket& packet);
    void InsertPacket(std::string_view msg);

    void SendComplete();

private:
    void BindSocket(const SOCKET& socket);
    void BindEvent(HANDLE eventHandle);

    void SendProcessing();

    ChatPacket DequePacket();
    bool SendMsg(std::string_view msg);


private:
    std::jthread m_sendProcessThread{ };

    OverlappedEx m_sendIO{ };
    
    bool m_processing{ true };

    SOCKET m_socket{ };
    DWORD m_ioSize{ };
    std::deque<ChatPacket> m_packetQueue{ };
};