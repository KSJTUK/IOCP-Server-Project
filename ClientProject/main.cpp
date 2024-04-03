#include "pch.h"
#include "NetworkClient.h"

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
        auto len{ uidLength(dre) };
        std::string str{ };
        str.resize(len, 0);
        for (auto& c : str) {
            c = static_cast<char>(uidChar(dre));
        }

        nc.InsertPacketQueue(str);
        std::this_thread::sleep_for(std::chrono::milliseconds{ TEST_TIME_MS });
#else
        std::string sendMsg{ };

        // 문자 출력 밀림을 방지하기 위한 sleep
        std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
        std::cout << "메시지 입력: ";
        std::getline(std::cin, sendMsg);

        nc.InsertPacketQueue(sendMsg);
#endif
    }
    TimeUtil::End();
}
