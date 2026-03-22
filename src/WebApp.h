#pragma once
#include "CEF/CEFInterface.h"

class WebApp
{
public:

	WebApp() = default;
	virtual ~WebApp() = default;

	bool Init();

	CefRefPtr<App> Handle() { return m_App; }

private:

	CefRefPtr<App> m_App;
};