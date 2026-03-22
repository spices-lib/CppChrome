#pragma once
#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>

class WebClient
{
public:

    WebClient() = default;
    virtual ~WebClient();

    void Init(class WebApp* app, bool shaderTexture);

    uint32_t Render();

private:

    std::thread m_TaskThread;
    std::shared_ptr<class Client> m_Client = nullptr;
    uint32_t m_ID;
};