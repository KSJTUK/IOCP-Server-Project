#include "pch.h"
#include "ServerFramework.h"

constexpr short DEFAULT_PORT{ 8080 };
constexpr __int32 MAX_CLIENT{ 50 };
constexpr __int32 MAX_THREAD{ 4 };

std::unique_ptr<ChatingServer> chatServer = std::make_unique<ChatingServer>();

int main()
{
#ifdef _DEBUG
	CRT_START
#endif

	TimeUtil::Init();

	chatServer->BindAndListen(DEFAULT_PORT);

	chatServer->Run(MAX_CLIENT, MAX_THREAD);

	std::string msg{ };
	while (true) {
		std::getline(std::cin, msg);
		if (msg == "quit") {
			break;
		}
	}

	chatServer->End();

	TimeUtil::End();
	ConsoleIO::End();
}