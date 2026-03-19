#pragma once
#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>

class WebRenderer
{
public:

    WebRenderer() = default;
    virtual ~WebRenderer();

    bool Init();

    uint32_t Render();

private:

    std::thread m_WebThread;
    std::shared_ptr<class Client> m_Client = nullptr;
    bool m_SharedTexture = false;
    std::mutex m_Mutex;
    std::condition_variable m_Condition;
};