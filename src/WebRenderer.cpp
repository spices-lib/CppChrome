#include "WebRenderer.h"
#include "ScopeTimer.h"
#include "D3D11Texture.h"
#include "NvInteropTexture.h"

#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>

#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_life_span_handler.h"
#include "include/cef_load_handler.h"
#include "include/wrapper/cef_helpers.h"
#include "include/capi/cef_app_capi.h"
#include "include/cef_client.h"

class LifeSpanHandler : public CefLifeSpanHandler {
public:
    LifeSpanHandler() = default;

    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override {
        CEF_REQUIRE_UI_THREAD();
        std::cout << "Browser created, ID: " << browser->GetIdentifier() << std::endl;
        std::cout << "[LifeSpan] Browser created, ID: " << browser->GetIdentifier() << std::endl;

        m_Host = browser->GetHost();
    }

    bool DoClose(CefRefPtr<CefBrowser> browser) override {
        CEF_REQUIRE_UI_THREAD();
        std::cout << "Browser closing..." << std::endl;
        return false;
    }

    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override {
        CEF_REQUIRE_UI_THREAD();
        std::cout << "Browser closed, ID: " << browser->GetIdentifier() << std::endl;
    }

    IMPLEMENT_REFCOUNTING(LifeSpanHandler);

public:

    CefRefPtr<CefBrowserHost> GetHost() { return m_Host; }

private:

    CefRefPtr<CefBrowserHost> m_Host = nullptr;
};

class LoadHandler : public CefLoadHandler {
public:
    LoadHandler() = default;

    void OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
        bool isLoading,
        bool canGoBack,
        bool canGoForward) override {
        CEF_REQUIRE_UI_THREAD();
        if (!isLoading) {
            std::cout << "[LoadHandler] Page loaded successfully!" << std::endl;
        }
    }

    IMPLEMENT_REFCOUNTING(LoadHandler);
};

// RenderHandler - 用于捕获渲染数据
class RenderHandler : public CefRenderHandler {
public:
    RenderHandler() = default;

    void GetViewRect(scoped_refptr<CefBrowser> browser, CefRect& rect) override
    {
        CEF_REQUIRE_UI_THREAD();
        rect.Set(0, 0, 3840, 2160);
        std::cout << "[RenderHandler] GetViewRect called: 3840*2160" << std::endl;

        static std::once_flag flag;

        std::call_once(flag, [&]() {

            m_D3D11Texture.Init();
        });
    }

    bool GetScreenInfo(scoped_refptr<CefBrowser> browser, CefScreenInfo& screen_info) override
    {
        CEF_REQUIRE_UI_THREAD();
        CefRect rect(0, 0, 3840, 2160);
        CefRect available_rect(0, 0, 3840, 2160);
        screen_info.Set(1.0f, 24, 24, false, rect, available_rect);
        std::cout << "[RenderHandler] GetScreenInfo called" << std::endl;
        return true;
    }

    // 绘制回调
    void OnPaint(CefRefPtr<CefBrowser> browser,
        PaintElementType type,
        const RectList& dirtyRects,
        const void* buffer,
        int width,
        int height) override {
        CEF_REQUIRE_UI_THREAD();

        m_Width = width;

        m_Height = height;

        m_SharedTexture = false;

        m_Data = std::make_shared<std::vector<uint8_t>>(width * height * 4);
        
        memcpy(m_Data->data(), buffer, width * height * 4);
        
        m_Condition.notify_all();
    }

    // 硬件加速渲染回调 - 共享纹理
    // 1.Windows下的纹理是D3D11纹理
    // 2.Angle后端支持零成本纹理转换，但是需要 OpenGL ES 环境
    void OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
        PaintElementType type,
        const RectList& dirtyRects,
        const CefAcceleratedPaintInfo& info) override {
        CEF_REQUIRE_UI_THREAD();
        
        m_Width = info.extra.content_rect.width;

        m_Height = info.extra.content_rect.height;

        m_SharedTexture = true;

        HANDLE sharedHandle = info.shared_texture_handle;

        if constexpr (0)
        {
            m_Data = m_D3D11Texture.ReadTexture(sharedHandle, m_Width, m_Height);
        }
        else
        {
            m_NvInteropTexture.ReadTexture(sharedHandle);
        }

        m_Condition.notify_all();
    }

    IMPLEMENT_REFCOUNTING(RenderHandler);

public:

    uint32_t WaitRender() {

        {
            SCOPE_TIME_COUNTER("WaitRender::Wait")

            std::unique_lock lock(m_Mutex);

            m_Condition.wait(lock);
        }

        static std::once_flag flag;

        std::call_once(flag, [&]() {

            glGenTextures(1, &m_TextureID);

            glBindTexture(GL_TEXTURE_2D, m_TextureID);
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0,
                GL_BGRA, GL_UNSIGNED_BYTE, nullptr);

            glGenBuffers(1, &m_PBO);

            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_PBO);

            glBufferData(GL_PIXEL_UNPACK_BUFFER, m_Data->size(), nullptr, GL_STREAM_DRAW);

            m_NvInteropTexture.Init();
        });

        // Method1
        if (!m_SharedTexture)
        {
            SCOPE_TIME_COUNTER("WaitRender::Upload Data")

            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_PBO);

            void* pboMemory = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
            if (!pboMemory) {

                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

                return m_TextureID;
            }

            memcpy(pboMemory, m_Data->data(), m_Data->size());

            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

            glBindTexture(GL_TEXTURE_2D, m_TextureID);

            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height,
                GL_BGRA, GL_UNSIGNED_BYTE, nullptr);

            glBindTexture(GL_TEXTURE_2D, 0);

            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        }
        else
        {
            SCOPE_TIME_COUNTER("WaitRender::Upload Data")

            m_NvInteropTexture.ShareTexture(m_TextureID);
        }

        return m_TextureID; 
    }

private:
    
    std::mutex m_Mutex;
    std::condition_variable m_Condition;
    uint32_t m_TextureID;
    uint32_t m_PBO;
    uint32_t m_Width;
    uint32_t m_Height;
    D3D11Texture m_D3D11Texture;
    NvInteropTexture m_NvInteropTexture;
    std::shared_ptr<std::vector<uint8_t>> m_Data;
    bool m_SharedTexture = false;
};

class Client : public CefClient {
public:
    Client()
        : life_span_handler_(new LifeSpanHandler())
        , load_handler_(new LoadHandler())
        , render_handler_(new RenderHandler()) {
    }

    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override {
        return life_span_handler_;
    }

    CefRefPtr<CefLoadHandler> GetLoadHandler() override {
        return load_handler_;
    }

    CefRefPtr<CefRenderHandler> GetRenderHandler() override {
        return render_handler_;
    }

    IMPLEMENT_REFCOUNTING(Client);

private:
    CefRefPtr<CefLifeSpanHandler> life_span_handler_;
    CefRefPtr<CefLoadHandler> load_handler_;
    CefRefPtr<CefRenderHandler> render_handler_;
};

class App : public CefApp {
public:
    App() = default;

    void OnBeforeCommandLineProcessing(
        const CefString& process_type,
        CefRefPtr<CefCommandLine> command_line) override {

        // 启用硬件加速相关参数
        command_line->AppendSwitch("enable-gpu");
        //command_line->AppendSwitch("enable-gpu-rasterization");
        //command_line->AppendSwitch("enable-zero-copy");
        command_line->AppendSwitch("shared-texture-enabled");
        //command_line->AppendSwitch("multi-threaded-message-loop");

        //command_line->AppendSwitch("disable-gpu-compositing");
        //command_line->AppendSwitch("disable-gpu-vsync");
        //command_line->AppendSwitch("disable-frame-rate-limit");
    }

    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
        return nullptr;
    }

    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
        return nullptr;
    }

    IMPLEMENT_REFCOUNTING(App);
};

bool WebRenderer::Init(bool shaderTexture)
{
    CefMainArgs mainArgs;

    CefRefPtr<App> app(new App());

    // 执行子进程
    if (CefExecuteProcess(mainArgs, app, nullptr) >= 0) {
        return false;
    }

    m_WebThread = std::thread([&]() {
        
        // CEF 设置
        CefSettings                                 settings{};
        settings.log_severity                     = LOGSEVERITY_VERBOSE;
        settings.windowless_rendering_enabled     = true;
        settings.no_sandbox                       = true;
        settings.remote_debugging_port            = 9222;
        settings.multi_threaded_message_loop      = false;
        CefString(&settings.locale).FromASCII("en-US");
        CefString(&settings.resources_dir_path).FromASCII("D:/CppChrome/vendor/cef/Resources");
        CefString(&settings.locales_dir_path).FromASCII("D:/CppChrome/vendor/cef/Resources/locales");

        if (!CefInitialize(mainArgs, settings, app.get(), nullptr)) {
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
            "https://www.bilibili.com/index.html", 
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

        m_Condition.notify_all();

        // 运行消息循环
        CefRunMessageLoop();
    });

    {
        std::unique_lock lock(m_Mutex);

        m_Condition.wait(lock);
    }

    for(int i = 0; i < 3; ++i)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        auto lifeSpan = dynamic_cast<LifeSpanHandler*>(m_Client->GetLifeSpanHandler().get());

        while (!lifeSpan->GetHost()) {}

        lifeSpan->GetHost()->SendExternalBeginFrame();
    }

    return true;
}

WebRenderer::~WebRenderer()
{
    m_WebThread.join();
}

uint32_t WebRenderer::Render()
{
    SCOPE_TIME_COUNTER("WebRenderer::Render")

    auto lifeSpan = dynamic_cast<LifeSpanHandler*>(m_Client->GetLifeSpanHandler().get());
    
    if (!lifeSpan) return 0;

    lifeSpan->GetHost()->SendExternalBeginFrame();

    auto renderHandler = dynamic_cast<RenderHandler*>(m_Client->GetRenderHandler().get());

    auto id = renderHandler->WaitRender();

    return id;
}