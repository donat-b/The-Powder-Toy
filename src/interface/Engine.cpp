#ifdef NEWINTERFACE
#include "Engine.h"
#include "interface.h"
#include "Window.h"
#include "common/Point.h"
#include "graphics/VideoBuffer.h"


Engine::Engine():
	windows(std::stack<Window_*>()),
	top(NULL)
{

}

Engine::~Engine()
{
	while(!windows.empty())
	{
		delete windows.top();
		windows.pop();
	}
}

bool Engine::EventProcess(SDL_Event event)
{
	/*if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
	{
		if (event.key.keysym.unicode == 0)
		{
			// If unicode is zero, this could be a numpad key with numlock off, or numlock on and shift on (unicode is set to 0 by SDL or the OS in these circumstances. If numlock is on, unicode is the relevant digit character).
			// For some unknown reason, event.key.keysym.mod seems to be unreliable on some computers (keysum.mod&KEY_MOD_NUM is opposite to the actual value), so check keysym.unicode instead.
			// Note: unicode is always zero for SDL_KEYUP events, so this translation won't always work properly for keyup events.
			SDLKey newKey = MapNumpad(event.key.keysym.sym);
			if (newKey != event.key.keysym.sym)
			{
				event.key.keysym.sym = newKey;
				event.key.keysym.unicode = 0;
			}
		}
	}*/
	switch (event.type)
	{
	case SDL_KEYDOWN:
		top->DoKeyPress(event.key.keysym.sym, event.key.keysym.unicode, (unsigned char)sdl_mod);

		if (event.key.keysym.sym == SDLK_ESCAPE)
			return true;
		else if (event.key.keysym.sym == 'q' && (sdl_mod & (KMOD_CTRL|KMOD_META)))
		{
			if (confirm_ui(vid_buf, "You are about to quit", "Are you sure you want to quit?", "Quit"))
			{
				//has_quit = 1;
				return true;
			}
		}
		break;

	case SDL_KEYUP:
		top->DoKeyRelease(event.key.keysym.sym, event.key.keysym.unicode, (unsigned char)sdl_mod);
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (event.button.button == SDL_BUTTON_WHEELUP)
			top->DoMouseWheel(event.motion.x/sdl_scale, event.motion.y/sdl_scale, 1);
		else if (event.button.button == SDL_BUTTON_WHEELDOWN)
			top->DoMouseWheel(event.motion.x/sdl_scale, event.motion.y/sdl_scale, -1);
		else
			top->DoMouseDown(event.motion.x/sdl_scale, event.motion.y/sdl_scale, event.button.button);
		lastMousePosition = Point(event.motion.x/sdl_scale, event.motion.y/sdl_scale);
		break;
	case SDL_MOUSEBUTTONUP:
		top->DoMouseUp(event.motion.x/sdl_scale, event.motion.y/sdl_scale, event.button.button);
		lastMousePosition = Point(event.motion.x/sdl_scale, event.motion.y/sdl_scale);
		break;
	case SDL_MOUSEMOTION:
		top->DoMouseMove(event.motion.x/sdl_scale, event.motion.y/sdl_scale, event.motion.x/sdl_scale-lastMousePosition.X, event.motion.y/sdl_scale-lastMousePosition.Y);
		lastMousePosition = Point(event.motion.x/sdl_scale, event.motion.y/sdl_scale);
		break;
	case SDL_QUIT:
		//if (fastquit)
		//	has_quit = 1;
		return true;
	}
	return false;
}

void Engine::MainLoop()
{
	SDL_Event event;
	Window_* potato = new Window_(Point(100,100), Point(120,200));
	windows.push(potato);
	top = potato;
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	while (true)
	{
		sdl_mod = SDL_GetModState();
		while (SDL_PollEvent(&event))
		{
			int ret = EventProcess(event);
			if (ret)
			{
				SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
				return;
			}
		}
		top->DoTick(0);
		top->DoDraw();
		sdl_blit(0, 0, XRES+BARSIZE, YRES+MENUSIZE, vid_buf /*potato->GetVid()->GetVid()*/, XRES+BARSIZE);
		//memset(vid_buf, 0, (XRES+BARSIZE)*(YRES+MENUSIZE)*PIXELSIZE);
		limit_fps();
	}
}
#endif
