#ifndef ENGINE_H
#define ENGINE_H

#include <stack>
#include "Window.h"
#include "common/Singleton.h"
#include "common/tpt-stdint.h"
#include "common/Point.h"

union SDL_Event;
class Engine : public Singleton<Engine>
{
public:
	Engine();
	~Engine();

	void MainLoop();
	void ShowWindow(Window_ *window);
	void CloseWindow(Window_ *window);

private:
	bool EventProcess(SDL_Event event);
	std::stack<Window_*> windows;
	Window_ *top;

	Point lastMousePosition;
	unsigned short lastModifiers;
	uint32_t lastTick;
};

#endif
