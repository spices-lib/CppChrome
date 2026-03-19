#pragma once
#include "CppRenderer.h"
#include "WebRenderer.h"

class Window
{
public:

	Window() = default;
	virtual ~Window() = default;

	void Init();

	void EventLoop(WebRenderer& webRenderer, CppRenderer& cppRenderer);

private:

	void* m_Windows = nullptr;
};