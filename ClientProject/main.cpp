#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#include <string>
#include <format>
#include <thread>
#include <iostream>

std::string defaultPort{ "10000" };
std::string serverIP{ "192.168.21.180" };

int main()
{
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
    
    if (::connect(socket, reinterpret_cast<sockaddr*>(std::addressof(sockAddr)), sizeof(sockaddr_in)) == SOCKET_ERROR) {
        std::cout << "Connect fail\n" << std::endl;
        ::closesocket(socket);
        ::WSACleanup();
        exit(EXIT_FAILURE);
    }

    WSAEVENT event{ ::WSACreateEvent() };

    OVERLAPPED overlappedIO{ };
    std::memset(std::addressof(overlappedIO), 0, sizeof(OVERLAPPED));
    overlappedIO.hEvent = event;

    WSABUF data{ };
    DWORD flag{ };
    DWORD ioSize{ };
    std::string message{ };

    while (true) {
        std::cout << "메시지 입력: ";
        std::getline(std::cin, message);

        data.len = static_cast<ULONG>(message.size());
        data.buf = const_cast<char*>(message.c_str());

        if (::WSASend(socket, std::addressof(data), 1, std::addressof(ioSize), 0, std::addressof(overlappedIO), nullptr) == SOCKET_ERROR) {
            if (::WSAGetLastError() != WSA_IO_PENDING) {
                std::cout << "Sending Error! Error Code: " << ::WSAGetLastError() << std::endl;
                break;
            }
        }
        
        ::WSAWaitForMultipleEvents(1, std::addressof(event), TRUE, WSA_INFINITE, FALSE);

        ::WSAGetOverlappedResult(socket, std::addressof(overlappedIO), std::addressof(ioSize), FALSE, std::addressof(flag));
        std::cout << std::format("전송완료 bytes: {}\n", ioSize);

        if (::WSARecv(socket, std::addressof(data), 1, std::addressof(ioSize), std::addressof(flag), std::addressof(overlappedIO), nullptr) == SOCKET_ERROR) {
            if (::WSAGetLastError() != WSA_IO_PENDING) {
                std::cout << "Receiving Error! Error Code: " << ::WSAGetLastError() << std::endl;
                break;
            }
        }

        std::cout << std::format("Receive Data: {}\n", data.buf);
    }

    ::closesocket(socket);
    ::WSACleanup();
}