#include "pch.h"
#include "NetworkClient.h"
#include "MemoryBuf.h"

int main(int argc, char* argv[])
{
    TimeUtil::Init();
    short defaultPort{ 8080 };

    //std::string serverIP{ "220.120.143.86" };
    std::string serverIP{ "127.0.0.1" };

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
        sendMsg = ConsoleIO::Input();

        nc.InsertPacketQueue(PacketFacrory::CreatePacket<ChatPacket>(sendMsg));

        std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
#endif
    }
    TimeUtil::End();
    ConsoleIO::End();
}
