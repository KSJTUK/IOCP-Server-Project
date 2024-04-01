#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#include <deque>
#include <mutex>
#include <string>
#include <format>
#include <thread>
#include <iostream>

inline constexpr int MAX_BUFFER_SIZE{ 1024 };
inline constexpr int MAX_PACKET_SIZE{ 512 };

std::string defaultPort{ "10000" };

struct OverlappedEx {
    OVERLAPPED overlapped{ };
    WSABUF buffer{ };  
};

struct ChatPacket {
    short length{ };
    short toWhom{ };
    char msg[MAX_PACKET_SIZE - sizeof(short) * 2];
};

bool processingPacket{ true };
std::deque<ChatPacket> packetQueue{ };
std::mutex packetLock{ };
std::condition_variable cv;

void tryConnect(SOCKET& socket, sockaddr_in serverAddress);
void insertPacketData(ChatPacket& packet);
void insertPacketData(std::string_view msg);
int sendThread(SOCKET socket, OverlappedEx* sendIO, DWORD* ioSize, DWORD* flag);
bool sendPacket(SOCKET& socket, std::string_view msg, OverlappedEx& sendIO, DWORD& ioSize, DWORD flag = 0);
ChatPacket dequePacketData();

int main(int argc, char* argv[])
{
    std::string serverIP{ "192.168.21.180" };
    WSADATA wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        std::cout << "WSAStartup Fail" << std::endl;
        exit(EXIT_FAILURE);
    }

    SOCKET socket{ ::WSASocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, NULL, WSA_FLAG_OVERLAPPED) };
    if (socket == INVALID_SOCKET) {
        std::cout << "Create Socket Fail\n" << std::endl;
        exit(EXIT_FAILURE);
    }

    sockaddr_in sockAddr{ };
    sockAddr.sin_family = PF_INET;
    sockAddr.sin_addr.s_addr = ::inet_addr(serverIP.c_str());
    sockAddr.sin_port = ::htons(std::stoi(defaultPort));

    tryConnect(socket, sockAddr);

    WSAEVENT sendEvent{ ::WSACreateEvent() };
    WSAEVENT recvEvent{ ::WSACreateEvent() };

    OverlappedEx sendIO{ };
    std::memset(std::addressof(sendIO), 0, sizeof(OverlappedEx));
    sendIO.overlapped.hEvent = sendEvent;

    char recvBuffer[MAX_BUFFER_SIZE]{ };


    DWORD flag{ };
    DWORD ioSize{ };
    std::string message{ };

    std::thread sendingTh{ sendThread, socket, std::addressof(sendIO), std::addressof(ioSize), std::addressof(flag)};

    while (true) {
        std::cout << "메시지 입력: ";
        std::getline(std::cin, message);

        insertPacketData(message);
        
        ::WSAWaitForMultipleEvents(1, std::addressof(sendIO.overlapped.hEvent), TRUE, WSA_INFINITE, FALSE);
        ::WSAGetOverlappedResult(socket, reinterpret_cast<LPOVERLAPPED>(std::addressof(sendIO)), std::addressof(ioSize), FALSE, std::addressof(flag));

        std::cout << std::format("전송완료 bytes: {}\n", ioSize);
    }

    std::cout << "\n\n아무키나 누르면 종료합니다.\n";
    int closeInput = std::getchar();

    processingPacket = false;
    ::closesocket(socket);
    ::WSACleanup();
}

void tryConnect(SOCKET& socket, sockaddr_in serverAddress)
{
    while (true) {
        if (::connect(socket, reinterpret_cast<sockaddr*>(std::addressof(serverAddress)), sizeof(sockaddr_in))) {
            std::cout << std::format("[Connect Fail] Try Reconnecting, Error Code: {}]\n", ::WSAGetLastError());
            continue;
        }
        break;
    }
}

void insertPacketData(ChatPacket& packet)
{
    std::lock_guard lock{ packetLock };
    packetQueue.emplace_back(packet);
}

void insertPacketData(std::string_view msg) {
    std::lock_guard lock{ packetLock };
    ChatPacket packet{ static_cast<short>(msg.size()) };
    std::copy(msg.begin(), msg.end(), packet.msg);
    packetQueue.emplace_back(packet);
}

int sendThread(SOCKET socket, OverlappedEx* sendIO, DWORD* ioSize, DWORD* flag)
{
    while (processingPacket) {
        ChatPacket packet{ std::move(dequePacketData()) };
        if (packet.length != 0) {
            sendPacket(socket, packet.msg, *sendIO, *ioSize, *flag);
        }
        else {
            std::this_thread::yield();
        }
    }
    return 0;
}

bool sendPacket(SOCKET& socket, std::string_view msg, OverlappedEx& sendIO, DWORD& ioSize, DWORD flag)
{
    sendIO.buffer.len = static_cast<ULONG>(msg.size());
    sendIO.buffer.buf = const_cast<char*>(msg.data());

    std::cout << "send" << std::endl;
    if (::WSASend(socket, std::addressof(sendIO.buffer), 1, std::addressof(ioSize), flag, reinterpret_cast<LPOVERLAPPED>(std::addressof(sendIO)), nullptr) == SOCKET_ERROR) {
        if (::WSAGetLastError() != WSA_IO_PENDING) {
            std::cout << "Sending Error! Error Code: " << ::WSAGetLastError() << std::endl;
            return false;
        }
    }
    return true;
}

int recvThread(SOCKET socket)
{
    return 0;
}

ChatPacket dequePacketData()
{
    if (packetQueue.empty()) {
        return ChatPacket{};
    }
    ChatPacket packet{ std::move(packetQueue.front()) };
    packetQueue.pop_front();
    return packet;
}

