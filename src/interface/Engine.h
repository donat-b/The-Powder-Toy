#ifndef ENGINE_H
#define ENGINE_H

#include <stack>
#include <SDL/SDL.h>
#include "Window.h"
#include "common/Point.h"

class Engine
{
public:
	Engine();
	~Engine();

	void MainLoop();

private:
	bool EventProcess(SDL_Event event);
	std::stack<Window_*> windows;
	Window_ *top;

	Point lastMousePosition;
};

#endif
