#ifndef ENGINE_H
#define ENGINE_H

#include <stack>
#include "Window.h"
#include "common/Point.h"

union SDL_Event;
class Engine
{
public:
	Engine();
	~Engine();

	void MainLoop();
	void ShowWindow(Window_ *window);

private:
	bool EventProcess(SDL_Event event);
	std::stack<Window_*> windows;
	Window_ *top;

	Point lastMousePosition;
};

#endif
