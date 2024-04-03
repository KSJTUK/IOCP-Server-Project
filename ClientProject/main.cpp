#include "pch.h"
#include "NetworkClient.h"

int main(int argc, char* argv[])
{
    short defaultPort{ 10000 };
    std::string serverIP{ "192.168.21.13" };

    NetworkClient nc{ };
    nc.ConnectToServer(defaultPort, serverIP);

    nc.StartServer();
    
    while (true) {
        std::string sendMsg{ };
        std::cout << "메시지 입력: ";
        std::getline(std::cin, sendMsg);

        nc.InsertPacketQueue(sendMsg);
    }
}
