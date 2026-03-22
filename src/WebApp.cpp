#include "WebApp.h"
#include "CEF/CEFInterface.h"

bool WebApp::Init()
{
    CefMainArgs mainArgs;

    CefRefPtr<App> app(new App());

    // 执行子进程
    if (CefExecuteProcess(mainArgs, app, nullptr) >= 0)
    {
        return false;
    }

    m_App = app;

    return true;
}