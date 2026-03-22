#include "CefInterface.h"
#include "ScopeTimer.h"

#include <iostream>
#include <mutex>
#include <glad/glad.h>

void LifeSpanHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
	CEF_REQUIRE_UI_THREAD();

	std::cout << "Browser created, ID: " << browser->GetIdentifier() << std::endl;
	std::cout << "[LifeSpan] Browser created, ID: " << browser->GetIdentifier() << std::endl;

	m_Host = browser->GetHost();

	m_ID = browser->GetIdentifier();
}

bool LifeSpanHandler::DoClose(CefRefPtr<CefBrowser> browser)
{
	CEF_REQUIRE_UI_THREAD();

	std::cout << "Browser closing..." << std::endl;
	return false;
}

void LifeSpanHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
	CEF_REQUIRE_UI_THREAD();

	std::cout << "Browser closed, ID: " << browser->GetIdentifier() << std::endl;
}

void LoadHandler::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward)
{
	CEF_REQUIRE_UI_THREAD();

	if (!isLoading) {
		std::cout << "[LoadHandler] Page loaded successfully!" << std::endl;
	}
}

void RenderHandler::GetViewRect(scoped_refptr<CefBrowser> browser, CefRect& rect)
{
	CEF_REQUIRE_UI_THREAD();

	rect.Set(0, 0, 3840, 2160);
	std::cout << "[RenderHandler] GetViewRect called: 3840*2160" << std::endl;
}

bool RenderHandler::GetScreenInfo(scoped_refptr<CefBrowser> browser, CefScreenInfo& screen_info)
{
	CEF_REQUIRE_UI_THREAD();

	CefRect rect(0, 0, 3840, 2160);
	CefRect available_rect(0, 0, 3840, 2160);
	screen_info.Set(1.0f, 24, 24, false, rect, available_rect);
	std::cout << "[RenderHandler] GetScreenInfo called" << std::endl;
	return true;
}

void RenderHandler::OnPaint(
	CefRefPtr<CefBrowser> browser,
	PaintElementType type,
	const RectList& dirtyRects,
	const void* buffer,
	int width,
	int height
){
	CEF_REQUIRE_UI_THREAD();

	m_Width = width;

	m_Height = height;

	m_SharedTexture = false;

	m_Data = std::make_shared<std::vector<uint8_t>>(width * height * 4);

	memcpy(m_Data->data(), buffer, width * height * 4);

	m_Condition.notify_all();
}

void RenderHandler::OnAcceleratedPaint(
	CefRefPtr<CefBrowser> browser,
	PaintElementType type,
	const RectList& dirtyRects,
	const CefAcceleratedPaintInfo& info
){
	CEF_REQUIRE_UI_THREAD();

	m_Width = info.extra.content_rect.width;

	m_Height = info.extra.content_rect.height;

	m_SharedTexture = true;

	HANDLE sharedHandle = info.shared_texture_handle;

	m_NvInteropTexture.ReadTexture(sharedHandle);

	m_Condition.notify_all();
}

uint32_t RenderHandler::WaitRender() {

	{
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

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0,
			GL_BGRA, GL_UNSIGNED_BYTE, nullptr);

		if (!m_SharedTexture)
		{
			glGenBuffers(1, &m_PBO);

			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_PBO);

			glBufferData(GL_PIXEL_UNPACK_BUFFER, m_Data->size(), nullptr, GL_STREAM_DRAW);
		}

		m_NvInteropTexture.Init();
	});

	if (!m_SharedTexture)
	{
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
		m_NvInteropTexture.ShareTexture(m_TextureID);
	}

	return m_TextureID;
}

void App::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) 
{
	// 启用硬件加速相关参数
	command_line->AppendSwitch("enable-gpu");
	command_line->AppendSwitch("shared-texture-enabled");
	command_line->AppendSwitch("disable-gpu-vsync");
	command_line->AppendSwitch("disable-frame-rate-limit");
	command_line->AppendSwitchWithValue("remote-debugging-port", "9222");
}