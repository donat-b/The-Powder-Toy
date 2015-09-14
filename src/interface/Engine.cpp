#include "SDLCompat.h"
#include "graphics.h"
#ifdef WIN
#include <windows.h>
#include <shellapi.h>
#include "game/Menus.h"
#include "update.h"
#endif
#include "Engine.h"
#include "interface.h"
#include "misc.h"
#include "Window.h"
#include "common/Point.h"
#include "common/Platform.h"
#include "graphics/VideoBuffer.h"


Engine::Engine():
	windows(std::stack<Window_*>()),
	top(NULL),
	nextTop(NULL),
	lastMousePosition(Point(0, 0)),
	lastModifiers(0),
	lastTick(SDL_GetTicks())
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
		lastModifiers = sdl_mod = static_cast<unsigned short>(SDL_GetModState());
		top->DoKeyPress(event.key.keysym.sym, event.key.keysym.unicode, static_cast<unsigned short>(sdl_mod));

		if (event.key.keysym.sym == SDLK_ESCAPE && top->CanQuit())
			return true;
		else if (event.key.keysym.sym == 'q' && (sdl_mod & (KMOD_CTRL|KMOD_META)))
		{
			if (confirm_ui(vid_buf, "You are about to quit", "Are you sure you want to quit?", "Quit"))
			{
				has_quit = 1;
				return true;
			}
		}
		break;

	case SDL_KEYUP:
		lastModifiers = sdl_mod = static_cast<unsigned short>(SDL_GetModState());
		top->DoKeyRelease(event.key.keysym.sym, event.key.keysym.unicode, static_cast<unsigned short>(sdl_mod));
		break;
	case SDL_MOUSEBUTTONDOWN:
	{
		int mx = event.motion.x/sdl_scale, my = event.motion.y/sdl_scale;
		if (event.button.button == SDL_BUTTON_WHEELUP)
			top->DoMouseWheel(mx, my, 1);
		else if (event.button.button == SDL_BUTTON_WHEELDOWN)
			top->DoMouseWheel(mx, my, -1);
		else
			top->DoMouseDown(mx, my, SDL_BUTTON(event.button.button));
		lastMousePosition = Point(mx, my);
		break;
	}
	case SDL_MOUSEBUTTONUP:
	{
		int mx = event.motion.x/sdl_scale, my = event.motion.y/sdl_scale;
		top->DoMouseUp(mx, my, SDL_BUTTON(event.button.button));
		lastMousePosition = Point(mx, my);
		break;
	}
	case SDL_MOUSEMOTION:
	{
		int mx = event.motion.x/sdl_scale, my = event.motion.y/sdl_scale;
		top->DoMouseMove(mx, my, mx-lastMousePosition.X, my-lastMousePosition.Y);
		lastMousePosition = Point(mx, my);
		break;
	}
	case SDL_VIDEORESIZE:
		// screen resize event, we are supposed to call SDL_SetVideoMode with the new size. Ignore this entirely and call it with the old size :)
		// if we don't do this, the touchscreen calibration variables won't ever be set properly
		SetSDLVideoMode((XRES + BARSIZE) * sdl_scale, (YRES + MENUSIZE) * sdl_scale);
		break;
	case SDL_QUIT:
		if (fastquit)
			has_quit = 1;
		return true;

	// Special events are handled here (copy/paste on linux, open on windows)
	case SDL_SYSWMEVENT:
#if defined(LIN) && defined(SDL_VIDEO_DRIVER_X11)
		if (event.syswm.msg->subsystem != SDL_SYSWM_X11)
			break;
		sdl_wminfo.info.x11.lock_func();
		XEvent xe = event.syswm.msg->event.xevent;
		if (xe.type==SelectionClear)
		{
			if (clipboard_text!=NULL)
			{
				free(clipboard_text);
				clipboard_text = NULL;
			}
		}
		else if (xe.type==SelectionRequest)
		{
			XEvent xr;
			xr.xselection.type = SelectionNotify;
			xr.xselection.requestor = xe.xselectionrequest.requestor;
			xr.xselection.selection = xe.xselectionrequest.selection;
			xr.xselection.target = xe.xselectionrequest.target;
			xr.xselection.property = xe.xselectionrequest.property;
			xr.xselection.time = xe.xselectionrequest.time;
			if (xe.xselectionrequest.target==XA_TARGETS)
			{
				// send list of supported formats
				Atom targets[] = {XA_TARGETS, XA_STRING, XA_UTF8_STRING};
				xr.xselection.property = xe.xselectionrequest.property;
				XChangeProperty(sdl_wminfo.info.x11.display, xe.xselectionrequest.requestor, xe.xselectionrequest.property, XA_ATOM, 32, PropModeReplace, (unsigned char*)targets, (int)(sizeof(targets)/sizeof(Atom)));
			}
			// TODO: Supporting more targets would be nice
			else if ((xe.xselectionrequest.target==XA_STRING || xe.xselectionrequest.target==XA_UTF8_STRING) && clipboard_text)
			{
				XChangeProperty(sdl_wminfo.info.x11.display, xe.xselectionrequest.requestor, xe.xselectionrequest.property, xe.xselectionrequest.target, 8, PropModeReplace, (unsigned char*)clipboard_text, strlen(clipboard_text)+1);
			}
			else
			{
				// refuse clipboard request
				xr.xselection.property = None;
			}
			XSendEvent(sdl_wminfo.info.x11.display, xe.xselectionrequest.requestor, 0, 0, &xr);
		}
		sdl_wminfo.info.x11.unlock_func();
#elif WIN
		switch (event.syswm.msg->msg)
		{
		case WM_USER+614:
			if (!ptsaveOpenID && !saveURIOpen && num_tabs < 24-GetNumMenus() && main_loop)
				ptsaveOpenID = event.syswm.msg->lParam;
			//If we are already opening a save, we can't have it do another one, so just start it in a new process
			else
			{
				char *exename = Platform::ExecutableName(), args[64];
				sprintf(args, "ptsave noopen:%i", event.syswm.msg->lParam);
				if (exename)
				{
					ShellExecute(NULL, "open", exename, args, NULL, SW_SHOWNORMAL);
					free(exename);
				}
				//I doubt this will happen ... but as a last resort just open it in this window anyway
				else
					saveURIOpen = event.syswm.msg->lParam;
			}
			break;
		}
#else
		break;
#endif
	}
	return false;
}

void Engine::MainLoop()
{
	SDL_Event event;
	while (top)
	{
		// weird thing done when entering old interfaces that use sdl_poll loops
		if (sendNewEvents)
		{
			int mx, my;
			SDL_GetMouseState(&mx, &my);
			mx /= sdl_scale;
			my /= sdl_scale;
			if (Point(mx, my) != lastMousePosition)
			{
				top->DoMouseMove(mx, my, mx-lastMousePosition.X, my-lastMousePosition.Y);
				lastMousePosition = Point(mx, my);
			}

			unsigned short modState = SDL_GetModState();
			if (modState != lastModifiers)
			{
				top->DoKeyPress(-1, 0, modState);
				lastModifiers = modState;
			}
			sendNewEvents = false;
		}
		top->UpdateComponents();

		while (SDL_PollEvent(&event))
		{
			int ret = EventProcess(event);
			if (ret)
			{
				Window_ *temp = windows.top();
				temp->toDelete = true;
			}
		}

		uint32_t currentTick = SDL_GetTicks();
		top->DoTick(currentTick-lastTick);
		lastTick = currentTick;

		top->DoDraw();
		sdl_blit(0, 0, XRES+BARSIZE, YRES+MENUSIZE, vid_buf /*potato->GetVid()->GetVid()*/, XRES+BARSIZE);
		//memset(vid_buf, 0, (XRES+BARSIZE)*(YRES+MENUSIZE)*PIXELSIZE);
		limit_fps();

		// close / open any windows that need to be shown
		CloseWindowDelayed();
		if (nextTop)
			ShowWindowDelayed();
	}
}

// can only show one new window per frame
void Engine::ShowWindow(Window_ *window)
{
	if (!window || nextTop)
		return;
	nextTop = window;
	// show window now if this is the first
	if (!top)
		ShowWindowDelayed();
}

void Engine::ShowWindowDelayed()
{
	// key repeat on all windows except main one
	if (windows.size())
		SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	fillrect(vid_buf, -1, -1, XRES+BARSIZE+1, YRES+MENUSIZE+1, 0, 0, 0, 100);
	windows.push(nextTop);
	top = nextTop;
	nextTop = NULL;

	// update mouse position on any new windows
	int mx, my;
	SDL_GetMouseState(&mx, &my);
	mx /= sdl_scale;
	my /= sdl_scale;
	top->DoMouseMove(mx, my, 0, 0);
}

void Engine::CloseWindow(Window_ *window)
{
	if (window == windows.top())
	{
		window->toDelete = true;
	}
}

void Engine::CloseWindowDelayed()
{
	while (top && (top->toDelete || has_quit))
	{
		delete top;
		windows.pop();
		if (windows.size())
		{
			top = windows.top();
			top->FocusComponent(NULL);
			// update mouse position on any new windows
			int mx, my;
			SDL_GetMouseState(&mx, &my);
			mx /= sdl_scale;
			my /= sdl_scale;
			top->DoMouseMove(mx, my, 0, 0);
		}
		else
			top = NULL;

	}
	if (windows.size() == 1)
	{
		// make sure key repeat is off, breaks stuff
		SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
	}
}
