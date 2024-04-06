#include "pch.h"

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

	std::cout << std::put_time(timeLocal, "[%x - %X]");
}

std::string TimeUtil::GetTime()
{
	std::ostringstream os{ };
	os << std::put_time(timeLocal, "[%x %a %X]");
	return os.str();
}

void IOData::BufClear() {
	buffer.fill(0);
}
