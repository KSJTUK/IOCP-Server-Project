#include "pch.h"
#include "ServerFramework.h"

constexpr short DEFAULT_PORT{ 8080 };
constexpr __int32 MAX_CLIENT{ 50 };
constexpr __int32 MAX_THREAD{ 4 };

std::unique_ptr<EchoServer> echoServer = std::make_unique<EchoServer>();

int main()
{
#ifdef _DEBUG
	CRT_START
#endif

	TimeUtil::Init();

	echoServer->BindAndListen(DEFAULT_PORT);

	echoServer->Run(MAX_CLIENT, MAX_THREAD);

	while (true) {

	}

	TimeUtil::End();
	ConsoleIO::End();
}