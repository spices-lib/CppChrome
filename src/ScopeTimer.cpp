#include "ScopeTimer.h"
#include <iostream>

ScopeTimer::ScopeTimer(const std::string& log)
	: m_Log(log)
{
	m_InTime = std::chrono::high_resolution_clock::now();
}

ScopeTimer::~ScopeTimer()
{
	m_LeaveTime = std::chrono::high_resolution_clock::now();

	std::stringstream ss;
	ss << "ScopeTimer : " << m_Log << " Cost " << std::chrono::duration<float, std::chrono::milliseconds::period>(m_LeaveTime - m_InTime).count() << " ms ";

	std::cout << ss.str() << std::endl;
}