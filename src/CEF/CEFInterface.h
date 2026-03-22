#pragma once
#include "NvInteropTexture.h"

#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_life_span_handler.h"
#include "include/cef_load_handler.h"
#include "include/wrapper/cef_helpers.h"
#include "include/capi/cef_app_capi.h"
#include "include/cef_client.h"

#include <condition_variable>
#include <mutex>
#include <memory>
#include <vector>

class LifeSpanHandler : public CefLifeSpanHandler 
{
public:

    LifeSpanHandler() = default;

    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;

    bool DoClose(CefRefPtr<CefBrowser> browser) override;

    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    IMPLEMENT_REFCOUNTING(LifeSpanHandler);

public:

    CefRefPtr<CefBrowserHost> GetHost() { return m_Host; }

    int GetID() { return m_ID; }

private:

    CefRefPtr<CefBrowserHost> m_Host = nullptr;
    int m_ID;
};

class LoadHandler : public CefLoadHandler 
{
public:

    LoadHandler() = default;

    void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward) override;

    IMPLEMENT_REFCOUNTING(LoadHandler);
};

class RenderHandler : public CefRenderHandler
{
public:

    RenderHandler() = default;

    void GetViewRect(scoped_refptr<CefBrowser> browser, CefRect& rect) override;

    bool GetScreenInfo(scoped_refptr<CefBrowser> browser, CefScreenInfo& screen_info) override;

    void OnPaint(
        CefRefPtr<CefBrowser> browser,
        PaintElementType type,
        const RectList& dirtyRects,
        const void* buffer,
        int width,
        int height) override;

    void OnAcceleratedPaint(
        CefRefPtr<CefBrowser> browser,
        PaintElementType type,
        const RectList& dirtyRects,
        const CefAcceleratedPaintInfo& info) override;

    IMPLEMENT_REFCOUNTING(RenderHandler);

public:

    uint32_t WaitRender();

private:

    std::mutex m_Mutex;
    std::condition_variable m_Condition;
    uint32_t m_TextureID;
    uint32_t m_PBO;
    uint32_t m_Width;
    uint32_t m_Height;
    NvInteropTexture m_NvInteropTexture;
    std::shared_ptr<std::vector<uint8_t>> m_Data;
    bool m_SharedTexture = false;
};

class Client : public CefClient 
{
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

class App : public CefApp 
{
public:

    App() = default;

    void OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) override;

    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
        return nullptr;
    }

    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
        return nullptr;
    }

    IMPLEMENT_REFCOUNTING(App);
};