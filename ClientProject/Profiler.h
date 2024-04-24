#pragma once

#include <filesystem>
#include <fstream>
#include <chrono>

class Profiler {

};

class TimeProfiler {
public:
	TimeProfiler();
	~TimeProfiler();

public:
	double GetTimeMilli() const { return m_elapsedTimeSec.count(); }
	double GetTimeSec() const { return m_elapsedTimeSec.count() / 1000.0; }
	auto GetElapsedTime() const { return m_elapsedTimeSec; }

	void SetTimeStamp() { m_timeStamp = m_elapsedTimeSec; }
	double GetTimeFromStamp() const { return m_elapsedTimeSec.count() - m_timeStamp.count(); }
	double GetTimeFromStampSec() const { return (m_elapsedTimeSec.count() - m_timeStamp.count()) / 1000.0; }

private:
	void TimerThread();

private:
	bool m_running{ true };
	std::chrono::duration<double, std::milli> m_elapsedTimeSec{ };
	std::chrono::duration<double, std::milli> m_timeStamp{ };
	std::jthread m_timerThread{ };
	std::chrono::steady_clock::time_point m_startTimeStamp{ };
};

// ȭ�鿡 ��µǴ� ������ ������� ����ϱ� ���� ��ġ
class ConsoleIO {
public:
	static void End()
	{
		m_running = false;
	}

	static void OutputString(const std::string& str)
	{
		std::unique_lock<std::mutex> lock{ m_lock };
		m_outputs.emplace_back(std::move(str));
		m_cv.notify_one();
	}

	static void OutputString(const char* str)
	{
		std::unique_lock<std::mutex> lock{ m_lock };
		m_outputs.emplace_back(std::move(str));
		m_cv.notify_one();
	}

private:
	static void IOThread()
	{
		while (m_running) {
			std::unique_lock<std::mutex> lock{ m_lock };
			m_cv.wait(lock, []() { return !m_outputs.empty(); });

			std::cout << m_outputs.front() << std::endl;
			m_outputs.pop_front();

			lock.unlock();
		}
	}

private:
	inline static bool m_running{ true };
	inline static std::condition_variable m_cv;
	inline static std::mutex m_lock{ };
	inline static std::jthread m_outputThread{ ConsoleIO::IOThread };
	inline static std::deque<std::string> m_outputs{ };
};