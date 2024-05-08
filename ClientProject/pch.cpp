#include "pch.h"
#include "NetworkClient.h"
#include "Profiler.h"

void TimeUtil::Init()
{
	prevLoc = std::cout.imbue(std::locale("ko_KR"));
}

void TimeUtil::End()
{
	std::cout.imbue(prevLoc);
}

void TimeUtil::PrintTime()
{
	timeNow = std::chrono::system_clock::now();
	timeUTC = std::chrono::system_clock::to_time_t(timeNow);
	timeLocal = localtime(&timeUTC);

	ConsoleIO::OutputString(GetTime());
}

std::string TimeUtil::GetTime()
{
	std::ostringstream os{ };
	os << std::put_time(timeLocal, "[%x %a %X]");
	return os.str();
}

std::unique_ptr<NetworkClient> nc = std::make_unique<NetworkClient>();