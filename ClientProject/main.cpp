#include "pch.h"
#include "SendThread.h"

std::string defaultPort{ "10000" };

void tryConnect(SOCKET& socket, sockaddr_in serverAddress);

int main(int argc, char* argv[])
{
    std::string serverIP{ "172.30.1.43" };
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

    OverlappedEx sendIO{ };
    std::memset(std::addressof(sendIO), 0, sizeof(OverlappedEx));
    
    char recvBuffer[MAX_BUFFER_SIZE]{ };

    SendThread sendTh{ };
    sendTh.StartSendingProcess(socket, sendEvent);

    DWORD flag{ };
    DWORD ioSize{ };
    std::string message{ };
    DWORD eventResult{ };
    while (true) {
        std::cout << "메시지 입력: ";
        std::getline(std::cin, message);

        sendTh.InsertPacket(message);

        eventResult = ::WSAWaitForMultipleEvents(1, std::addressof(sendEvent), TRUE, WSA_INFINITE, FALSE);
        std::cout << std::format("WSAWaitForMultipleEvents Return {}\n", eventResult);
        if (eventResult == WSA_WAIT_EVENT_0) {
            sendTh.SendComplete();
        }
        else {
            std::cout << std::format("WSAWaitForMiltipleEvents Error Code: {}\n", ::WSAGetLastError());
        }
    }

    std::cout << "\n\n아무키나 누르면 종료합니다.\n";
    int closeInput = std::getchar();
  
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
