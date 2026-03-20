#include "WebRenderer.h"
#include "Window.h"

int main() {

	Window window;

	WebRenderer webRenderer;
	
	// Cef Must be executed first.
	// 1.sub progress entry here and return.
	// 2.wgl needs glfw init.
	if (!webRenderer.Init(true)) {
		return 1;
	}

	window.Init();

	CppRenderer cppRenderer;

	cppRenderer.Init();

	window.EventLoop(webRenderer, cppRenderer);
}