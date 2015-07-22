#ifndef SDLCOMPAT_H
#define SDLCOMPAT_H

#ifdef SDL_R_INCL
#include <SDL.h>
#else
#include <SDL/SDL.h>
#endif

#if defined(WIN) || defined(LIN)
#ifdef SDL_R_INCL
#include <SDL_syswm.h>
#else
#include <SDL/SDL_syswm.h>
#endif
#endif

#undef Above
#undef Below

#endif // SDLCOMPAT_H
