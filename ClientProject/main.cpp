#include "pch.h"
#include "NetworkClient.h"

#define SERVER_TEST 1
#define TEST_TIME_MS 2000


int main(int argc, char* argv[])
{
    TimeUtil::Init();
    short defaultPort{ 10000 };
    std::string serverIP{ "192.168.21.13" };

    NetworkClient nc{ };
    nc.ConnectToServer(defaultPort, serverIP);

    nc.StartServer();
    
    while (true) {
#if SERVER_TEST
        nc.InsertPacketQueue("dummy message");
        std::this_thread::sleep_for(std::chrono::milliseconds{ TEST_TIME_MS });
#else
        std::string sendMsg{ };
        std::cout << "메시지 입력: ";
        std::getline(std::cin, sendMsg);

        nc.InsertPacketQueue(sendMsg);
#endif
    }
    TimeUtil::End();
}
