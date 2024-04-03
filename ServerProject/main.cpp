#include "pch.h"
#include "ServerFramework.h"

constexpr short DEFAULT_PORT{ 10000 };
constexpr __int32 MAX_CLIENT{ 100 };
constexpr __int32 MAX_THREAD{ 4 };

int main()
{
	EchoServer echoServer{ };
	echoServer.BindAndListen(DEFAULT_PORT);

	echoServer.Run(MAX_CLIENT, MAX_THREAD);

	while (true) {

	}
}