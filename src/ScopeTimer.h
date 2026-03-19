#pragma once
#include <chrono>

class ScopeTimer
{
public:

	ScopeTimer(const std::string& log);
	virtual ~ScopeTimer();

private:

	std::chrono::steady_clock::time_point m_InTime;
	std::chrono::steady_clock::time_point m_LeaveTime;

	std::string m_Log;
};

#define SCOPE_TIME_COUNTER(...)   ::ScopeTimer thisScopeTimeCounter(__VA_ARGS__);