#include "pch.h"
#include "ServerFramework.h"

constexpr short DEFAULT_PORT{ 8080 };
constexpr __int32 MAX_CLIENT{ 50 };
constexpr __int32 MAX_THREAD{ 4 };

int main()
{
	TimeUtil::Init();

	EchoServer echoServer{ };
	echoServer.BindAndListen(DEFAULT_PORT);

	echoServer.Run(MAX_CLIENT, MAX_THREAD);

	while (true) {

	}

	TimeUtil::End();
	ConsoleIO::End();
}