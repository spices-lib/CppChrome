#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>

#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_client.h"
#include "include/cef_frame.h"
#include "include/cef_life_span_handler.h"
#include "include/cef_load_handler.h"
#include "include/wrapper/cef_helpers.h"
#include "include/capi/cef_app_capi.h"

// 简单的 LifeSpanHandler 实现
class SimpleLifeSpanHandler : public CefLifeSpanHandler {
public:
    SimpleLifeSpanHandler() = default;

    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override {
        CEF_REQUIRE_UI_THREAD();
        std::cout << "Browser created, ID: " << browser->GetIdentifier() << std::endl;
        std::cout << "[LifeSpan] Browser created, ID: " << browser->GetIdentifier() << std::endl;
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

    IMPLEMENT_REFCOUNTING(SimpleLifeSpanHandler);
};

// 简单的 LoadHandler 实现
class SimpleLoadHandler : public CefLoadHandler {
public:
    SimpleLoadHandler() = default;

    void OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
                              bool isLoading,
                              bool canGoBack,
                              bool canGoForward) override {
        CEF_REQUIRE_UI_THREAD();
        if (!isLoading) {
            std::cout << "[LoadHandler] Page loaded successfully!" << std::endl;
        }
    }

    IMPLEMENT_REFCOUNTING(SimpleLoadHandler);
};

// RenderHandler - 用于捕获渲染数据
class SimpleRenderHandler : public CefRenderHandler {
public:
    SimpleRenderHandler() = default;

    void GetViewRect(scoped_refptr<CefBrowser> browser, CefRect& rect) override
    {
        CEF_REQUIRE_UI_THREAD();
        rect.Set(0, 0, 800, 600);
        std::cout << "[RenderHandler] GetViewRect called: 800x600" << std::endl;
    }
    
    bool GetScreenInfo(scoped_refptr<CefBrowser> browser, CefScreenInfo& screen_info) override
    {
        CEF_REQUIRE_UI_THREAD();
        CefRect rect(0, 0, 800, 600);
        CefRect available_rect(0, 0, 800, 600);
        screen_info.Set(1.0f, 24, 24, false, rect, available_rect);
        std::cout << "[RenderHandler] GetScreenInfo called" << std::endl;
        return true;
    }
    
    // 绘制回调 - 窗口模式下不会被调用
    void OnPaint(CefRefPtr<CefBrowser> browser,
                 PaintElementType type,
                 const RectList& dirtyRects,
                 const void* buffer,
                 int width,
                 int height) override {
        CEF_REQUIRE_UI_THREAD();
        
        std::cout << "[RenderHandler] OnPaint called (should not happen in windowed mode!)" << std::endl;
    }
    
    // 硬件加速渲染回调 - 共享纹理
    void OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
                            PaintElementType type,
                            const RectList& dirtyRects,
                            const CefAcceleratedPaintInfo& info) override {
        CEF_REQUIRE_UI_THREAD();

        std::cout << "[RenderHandler] *** OnAcceleratedPaint called ***" << std::endl;
        
        static int accel_paint_count = 0;
        ++accel_paint_count;
        std::cout << "[RenderHandler] *** OnAcceleratedPaint #" << accel_paint_count 
                  << " | Type=" << type 
                  << ", SharedTextureHandle=" << info.shared_texture_handle
                  << std::endl;
    }

    IMPLEMENT_REFCOUNTING(SimpleRenderHandler);
    
};

// 简单的 Client 实现
class SimpleClient : public CefClient {
public:
    SimpleClient() 
        : life_span_handler_(new SimpleLifeSpanHandler()),
          load_handler_(new SimpleLoadHandler()),
          render_handler_(new SimpleRenderHandler()) {}

    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override {
        return life_span_handler_;
    }

    CefRefPtr<CefLoadHandler> GetLoadHandler() override {
        return load_handler_;
    }

    CefRefPtr<CefRenderHandler> GetRenderHandler() override {
        return render_handler_;
    }

    IMPLEMENT_REFCOUNTING(SimpleClient);

private:
    CefRefPtr<CefLifeSpanHandler> life_span_handler_;
    CefRefPtr<CefLoadHandler> load_handler_;
    CefRefPtr<CefRenderHandler> render_handler_;
};

// 简单的 App 实现
class SimpleApp : public CefApp {
public:
    SimpleApp() = default;

    void OnBeforeCommandLineProcessing(
        const CefString& process_type,
        CefRefPtr<CefCommandLine> command_line) override {
        
        // 启用硬件加速相关参数
        command_line->AppendSwitch("enable-gpu");
        command_line->AppendSwitch("enable-gpu-rasterization");
        command_line->AppendSwitch("enable-zero-copy");
        command_line->AppendSwitch("shared-texture-enabled");
        command_line->AppendSwitch("multi-threaded-message-loop");
        
        // 使用 ANGLE 的 D3D11 后端
        command_line->AppendSwitchWithValue("use-angle", "d3d11");
        
        // 不禁用 GPU 合成，让 GPU 正常工作
        command_line->AppendSwitch("no-sandbox");
        command_line->AppendSwitch("ignore-certificate-errors");
    }
    
    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
        return nullptr;
    }

    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
        return nullptr;
    }

    IMPLEMENT_REFCOUNTING(SimpleApp);
};


int main() {

    // 提供命令行参数
    CefMainArgs main_args;
    
    // 初始化 CEF
    CefRefPtr<SimpleApp> app(new SimpleApp());
    
    // 执行子进程
    int exit_code = CefExecuteProcess(main_args, app, nullptr);
    if (exit_code >= 0) {
        return exit_code;
    }
    
    // CEF 设置
    CefSettings settings;
    settings.log_severity = LOGSEVERITY_VERBOSE;
    settings.windowless_rendering_enabled = true;
    settings.no_sandbox = true;
    CefString(&settings.locale).FromASCII("en-US");
    CefString(&settings.resources_dir_path).FromASCII("D:/CppChrome/vendor/cef/Resources");
    CefString(&settings.locales_dir_path).FromASCII("D:/CppChrome/vendor/cef/Resources/locales");
    settings.remote_debugging_port = 9222;
    
    bool init_result = CefInitialize(main_args, settings, app.get(), nullptr);
    
    if (!init_result) {
        std::cerr << "Failed to initialize CEF!" << std::endl;
        return 1;
    }

    std::cout << "CEF initialized successfully!" << std::endl;

    // 创建真实的 Windows 窗口
    /*HWND hwnd = CreateWindowExW(
        0,
        L"STATIC",  // 使用静态窗口类
        L"CEF Browser Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );
    
    if (!hwnd) {
        std::cerr << "Failed to create window!" << std::endl;
        CefShutdown();
        return 1;
    }
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    
    std::cout << "Window created!" << std::endl;*/
    
    // 创建浏览器窗口信息
    CefWindowInfo windowInfo;
    RECT browserRect = {0, 0, 800, 600};
    windowInfo.SetAsWindowless(NULL); 
    windowInfo.shared_texture_enabled = true;
    windowInfo.external_begin_frame_enabled = false;
    
    // 浏览器设置
    CefBrowserSettings browserSettings;
    browserSettings.background_color = CefColorSetARGB(255, 255, 255, 255);
    
    // 创建浏览器
    CefRefPtr client(new SimpleClient());
    bool created = CefBrowserHost::CreateBrowser(
        windowInfo,
        client.get(),
        "https://www.bilibili.com/index.html",
        browserSettings,
        nullptr,
        nullptr
    );

    if (!client) {
        std::cerr << "Failed to create browser!" << std::endl;
        CefShutdown();
        return 1;
    }

    std::cout << "Browser created! Running message loop..." << std::endl;

    // 运行消息循环
    //CefDoMessageLoopWork();
    CefRunMessageLoop();

    std::cout << "Message loop ended." << std::endl;

    //DestroyWindow(hwnd);
    //CefShutdown();
    
    return 0;
}