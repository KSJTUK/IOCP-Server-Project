#include "pch.h"
#include "Profiler.h"

TimeProfiler::TimeProfiler() : m_startTimeStamp{ std::chrono::high_resolution_clock::now() } {
	m_timerThread = std::jthread{ [this]() { TimerThread(); } };
}

TimeProfiler::~TimeProfiler() {
	m_running = false;
}

void TimeProfiler::TimerThread() {
	while (m_running) {
		auto t = std::chrono::high_resolution_clock::now();
		m_elapsedTimeSec = t - m_startTimeStamp;
	}
}
