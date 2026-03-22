#include "WebClient.h"
#include "CEF/CEFInterface.h"
#include "ScopeTimer.h"
#include "WebApp.h"

#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>

void WebClient::Init(class WebApp* app, bool shaderTexture)
{
    // CEF 设置
    CefSettings                                 settings{};
    settings.log_severity                     = LOGSEVERITY_VERBOSE;
    settings.windowless_rendering_enabled     = true;
    settings.no_sandbox                       = true;
    settings.remote_debugging_port            = 9222;
    settings.multi_threaded_message_loop      = true;
    CefString(&settings.locale).FromASCII("en-US");
    CefString(&settings.resources_dir_path).FromASCII("D:/CppChrome/vendor/cef/Resources");
    CefString(&settings.locales_dir_path).FromASCII("D:/CppChrome/vendor/cef/Resources/locales");

    CefMainArgs mainArgs;

    if (!CefInitialize(mainArgs, settings, app->Handle(), nullptr)) {
        std::cout << "Failed to initialize CEF!" << std::endl;
        return;
    }

    std::cout << "CEF initialized successfully!" << std::endl;

    // 创建浏览器窗口信息
    RECT browserRect = { 0, 0, 3840, 2160 };

    CefWindowInfo                               windowInfo{};
    windowInfo.SetAsWindowless(NULL);
    windowInfo.shared_texture_enabled         = shaderTexture;
    windowInfo.external_begin_frame_enabled   = true;

    // 浏览器设置
    CefBrowserSettings browserSettings;
    browserSettings.background_color = CefColorSetARGB(255, 255, 255, 255);
    browserSettings.windowless_frame_rate = 120;

    // 创建浏览器
    m_Client = std::make_shared<Client>();
    bool created = CefBrowserHost::CreateBrowser(
        windowInfo, 
        m_Client.get(),
        "https://threejs.org/examples/#webgl_instancing_dynamic", 
        browserSettings,
        nullptr,
        nullptr
    );

    if (!m_Client) {
        std::cout << "Failed to create browser!" << std::endl;
        CefShutdown();
        return;
    }

    std::cout << "Browser created! Running message loop..." << std::endl;


    for(int i = 0; i < 3; ++i)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        auto lifeSpan = dynamic_cast<LifeSpanHandler*>(m_Client->GetLifeSpanHandler().get());

        while (!lifeSpan->GetHost()) {}

        lifeSpan->GetHost()->SendExternalBeginFrame();
    }

    m_TaskThread = std::thread([&]() {
        
        while (1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            Render();
        }
    });

    return;
}

WebClient::~WebClient()
{
    m_TaskThread.join();
}

uint32_t WebClient::Render()
{
    auto lifeSpan = dynamic_cast<LifeSpanHandler*>(m_Client->GetLifeSpanHandler().get());

    std::stringstream ss;
    ss << "WebClient::Render::ID::" << lifeSpan->GetID();

    SCOPE_TIME_COUNTER(ss.str())

    lifeSpan->GetHost()->SendExternalBeginFrame();

    auto renderHandler = dynamic_cast<RenderHandler*>(m_Client->GetRenderHandler().get());

    auto id = renderHandler->WaitRender();

    return id;
}