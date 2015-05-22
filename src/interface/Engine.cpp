#ifdef NEWINTERFACE
#include <SDL/SDL.h>
#include "Engine.h"
#include "interface.h"
#include "misc.h"
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

	// Special events are handled here (copy/paste on linux, open on windows)
	case SDL_SYSWMEVENT:
#if defined(LIN) && defined(SDL_VIDEO_DRIVER_X11)
		if (event.syswm.msg->subsystem != SDL_SYSWM_X11)
			break;
		sdl_wminfo.info.x11.lock_func();
		XEvent xe = event.syswm.msg->event.xevent;
		if (xe.type==SelectionClear)
		{
			if (clipboard_text!=NULL) {
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
				char *exename = exe_name(), args[64];
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
		sdl_mod = SDL_GetModState();
		while (SDL_PollEvent(&event))
		{
			int ret = EventProcess(event);
			if (ret)
			{
				Window_ *temp = windows.top();
				temp->toDelete = true;
			}
		}
		top->DoTick(0);
		top->DoDraw();
		if (top->toDelete)
			CloseWindow(top);
		sdl_blit(0, 0, XRES+BARSIZE, YRES+MENUSIZE, vid_buf /*potato->GetVid()->GetVid()*/, XRES+BARSIZE);
		//memset(vid_buf, 0, (XRES+BARSIZE)*(YRES+MENUSIZE)*PIXELSIZE);
		limit_fps();
	}
	SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
}

void Engine::ShowWindow(Window_ *window)
{
	windows.push(window);
	if (top == NULL)
		top = window;
	fillrect(vid_buf, -1, -1, XRES+BARSIZE+1, YRES+MENUSIZE+1, 0, 0, 0, 100);
}

void Engine::CloseWindow(Window_ *window)
{
	if (window == windows.top())
	{
		windows.pop();
		if (windows.size())
			top = windows.top();
		else
			top = NULL;
	}
}

#endif
