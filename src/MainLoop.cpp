#include "WebRenderer.h"
#include "Window.h"

int main() {

	WebRenderer webRenderer;
	
	// Cef Must be executed first.
	if (!webRenderer.Init()) {
		return 1;
	}

	Window window;

	window.Init();

	CppRenderer cppRenderer;

	cppRenderer.Init();

	window.EventLoop(webRenderer, cppRenderer);
}