#pragma once
#include "CppRenderer.h"
#include "WebClient.h"

class Window
{
public:

	Window() = default;
	virtual ~Window() = default;

	void Init();

	void EventLoop(WebClient& webClient, CppRenderer& cppRenderer);

private:

	void* m_Windows = nullptr;
};