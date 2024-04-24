#include "pch.h"
#include "NetworkClient.h"
#include "MemoryBuf.h"

int main(int argc, char* argv[])
{
    MemoryBuf buf{ };

    TimeUtil::Init();
    short defaultPort{ 10000 };
    std::string serverIP{ "172.30.1.78" };

    NetworkClient nc{ };
    nc.ConnectToServer(defaultPort, serverIP);

    nc.StartServer();
    
    while (true) {
#if SERVER_TEST       
        if (uidPacket(dre) == CHAT_TYPE) {
            auto len{ uidLength(dre) };
            std::string str{ };
            str.resize(len, 0);
            for (auto& c : str) {
                c = static_cast<char>(uidChar(dre));
            }

			nc.InsertPacketQueue(PacketFacrory::CreatePacket<ChatPacket>(str));
        }
        else {
            nc.InsertPacketQueue(PacketFacrory::CreatePacket<PositionPacket>(
                ufd(dre), ufd(dre), ufd(dre)
            ));
        }


        std::this_thread::sleep_for(std::chrono::milliseconds{ TEST_TIME_MS });
#else
        std::string sendMsg{ };

        // 문자 출력 밀림을 방지하기 위한 sleep
        std::cout << "메시지 입력: ";
        std::getline(std::cin, sendMsg);

        nc.InsertPacketQueue(PacketFacrory::CreatePacket<ChatPacket>(sendMsg));

        std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
#endif
    }
    TimeUtil::End();
}
