#include "WebClient.h"
#include "Window.h"
#include "WebApp.h"
#include <vector>

int main() {

	WebApp app;

	if (!app.Init())
	{
		return 0;
	}

	Window window;

	window.Init();

	std::vector<std::shared_ptr<WebClient>> clients;

	for (int i = 0; i < 5; ++i)
	{
		auto client = std::make_shared<WebClient>();

		client->Init(&app, true);

		clients.emplace_back(client);
	}

	CppRenderer cppRenderer;

	cppRenderer.Init();

	window.EventLoop(*clients[0], cppRenderer);
}