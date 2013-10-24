/**
 * Powder Toy - user interface
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef MACOSX
#include <CoreFoundation/CFString.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bzlib.h>
#include <math.h>
#include <time.h>
#if defined(WIN32) && !defined(__GNUC__)
#include <io.h>
#else
#include <dirent.h>
#endif
#if defined(WIN32)
#include <direct.h>
#endif
#if defined(LIN32) || defined(LIN64)
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#ifdef WIN32
#include <SDL/SDL_syswm.h>
#endif

#include "http.h"
#include "md5.h"
#include "font.h"
#include "defines.h"
#include "powder.h"
#include "interface.h"
#include "misc.h"
#include "console.h"
#ifdef LUACONSOLE
#include "luaconsole.h"
#endif
#include "gravity.h"
#include "images.h"

#include "powdergraphics.h"
#include "save.h"
#include "hud.h"
#include "cJSON.h"
#include "update.h"

#include "game/Menus.h"
#include "simulation/Tool.h"

SDLMod sdl_mod;
int sdl_key, sdl_rkey, sdl_wheel, sdl_ascii, sdl_zoom_trig=0;
#if (defined(LIN32) || defined(LIN64)) && defined(SDL_VIDEO_DRIVER_X11)
SDL_SysWMinfo sdl_wminfo;
Atom XA_CLIPBOARD, XA_TARGETS;
#endif

char *shift_0="`1234567890-=[]\\;',./";
char *shift_1="~!@#$%^&*()_+{}|:\"<>?";

int svf_messages = 0;
int svf_login = 0;
int svf_admin = 0;
int svf_mod = 0;
char svf_user[64] = "";
char svf_user_id[64] = "";
char svf_pass[64] = "";
char svf_session_id[64] = "";
char svf_session_key[64] = "";

int svf_open = 0;
int svf_own = 0;
int svf_myvote = 0;
int svf_publish = 0;
int svf_modsave = 0;
char svf_filename[255] = "";
int svf_fileopen = 0;
char svf_id[16] = "";
char svf_name[64] = "";
char svf_description[255] = "";
char svf_author[64] = "";
char svf_tags[256] = "";
void *svf_last = NULL;
int svf_lsize;

char *search_ids[GRID_X*GRID_Y];
char *search_dates[GRID_X*GRID_Y];
int   search_votes[GRID_X*GRID_Y];
int   search_publish[GRID_X*GRID_Y];
int	  search_scoredown[GRID_X*GRID_Y];
int	  search_scoreup[GRID_X*GRID_Y];
char *search_names[GRID_X*GRID_Y];
char *search_owners[GRID_X*GRID_Y];
void *search_thumbs[GRID_X*GRID_Y];
int   search_thsizes[GRID_X*GRID_Y];

int search_own = 0;
int search_fav = 0;
int search_date = 0;
int search_page = 0;
int p1_extra = 0;
char search_expr[256] = "";

char server_motd[512] = "";

char *tag_names[TAG_MAX];
int tag_votes[TAG_MAX];

int zoom_en = 0;
int zoom_x=(XRES-ZSIZE_D)/2, zoom_y=(YRES-ZSIZE_D)/2;
int zoom_wx=0, zoom_wy=0;
unsigned char ZFACTOR = 8;
unsigned char ZSIZE = 32;

int numframes = 0;
int framenum = 0;
int hud_menunum = 0;
int has_quit = 0;
int dae = 0;
int dateformat = 7;
int show_ids = 0;

int drawgrav_enable = 0;

#include <simulation\Simulation.h>
#include <sstream>

//TODO: just do tooltips and make things much simpler
Tool* lastOver = NULL;

//fills all the menus with Tool*s
void menu_count()
{
	std::string tempActiveTools[3], decoActiveTools[3];
	if (activeTools[0]) //active tools might not have been initialized at the start
	{
		for (int i = 0; i < 3; i++)
			tempActiveTools[i] = activeTools[i]->GetIdentifier();
		for (int i = 0; i < 3; i++)
			decoActiveTools[i] = decoTools[i]->GetIdentifier();
	}
	for (int i = 0; i < SC_TOTAL; i++)
	{
		menuSections[i]->ClearTools();
	}
	lastOver = NULL;
	for (int i = 0; i < PT_NUM; i++)
	{
		if (globalSim->elements[i].Enabled && i != PT_LIFE)
		{
			if (globalSim->elements[i].MenuVisible || secret_els)
			{
				menuSections[globalSim->elements[i].MenuSection]->AddTool(new Tool(ELEMENT_TOOL, i, globalSim->elements[i].Identifier));
			}
			else
				menuSections[SC_OTHER]->AddTool(new Tool(ELEMENT_TOOL, i, globalSim->elements[i].Identifier));
		}
	}
	for (int i = 0; i < NGOL; i++)
	{
		menuSections[SC_LIFE]->AddTool(new Tool(GOL_TOOL, i, "DEFAULT_PT_LIFE_" + std::string(gmenu[i].name)));
	}
	for (int i = UI_WALLSTART; i < UI_WALLSTART+UI_WALLCOUNT; i++)
	{
		//TODO, deco / tool identifiers are wrong
		std::stringstream identifier;
		identifier << "DEFAULT_WL_" << i-UI_WALLSTART;
		if (!is_TOOL(i) && !is_DECOTOOL(i))
		{
			menuSections[SC_WALL]->AddTool(new Tool(WALL_TOOL, i, identifier.str()));
		}
		else if (is_DECOTOOL(i))
		{
			menuSections[SC_DECO]->AddTool(new Tool(DECO_TOOL, i, identifier.str()));
		}
		else
		{
			menuSections[SC_TOOL]->AddTool(new Tool(TOOL_TOOL, i, identifier.str()));
		}
	}
	menuSections[SC_FAV]->AddTool(new Tool(INVALID_TOOL, FAV_MORE, "DEFAULT_FAV_MORE"));
	for (int i = 0; i < 18; i++)
	{
		menuSections[SC_FAV]->AddTool(new Tool(INVALID_TOOL, FAV_MORE-1, "DEFAULT_FAV_FAKE"));
	}
	for (int i = FAV_START+1; i < FAV_END; i++)
	{
		menuSections[SC_FAV2]->AddTool(new Tool(INVALID_TOOL, i, "DEFAULT_FAV_" + std::string(fav[i-FAV_START].name)));
	}
	for (int i = HUD_START; i < HUD_START+HUD_NUM; i++)
	{
		menuSections[SC_HUD]->AddTool(new Tool(INVALID_TOOL, i, "DEFAULT_FAV_" + std::string(hud_menu[i-HUD_START].name)));
	}

	if (activeTools[0])
	{
		for (int i = 0; i < 3; i++)
			activeTools[i] = GetToolFromIdentifier(tempActiveTools[i]);
		for (int i = 0; i < 3; i++)
			decoTools[i] = GetToolFromIdentifier(decoActiveTools[i]);
	}
}

void get_sign_pos(int i, int *x0, int *y0, int *w, int *h)
{
	//Changing width if sign have special content
	if (strcmp(signs[i].text, "{p}")==0)
		*w = textwidth("Pressure: -000.00");

	if (strcmp(signs[i].text, "{t}")==0)
		*w = textwidth("Temp: 0000.00");

	if (!sregexp(signs[i].text, "^{[ct]:[0-9]*|.*}$") || !sregexp(signs[i].text, "^{s:.*|.*}$") || !sregexp(signs[i].text, "^{b|.*}$"))
	{
		int sldr, startm;
		char buff[256];
		memset(buff, 0, sizeof(buff));
		for (sldr=2; signs[i].text[sldr-1] != '|'; sldr++)
			startm = sldr + 1;

		sldr = startm;
		while (signs[i].text[sldr] != '}')
		{
			buff[sldr - startm] = signs[i].text[sldr];
			sldr++;
		}
		*w = textwidth(buff) + 5;
	}

	//Usual width
	if (strcmp(signs[i].text, "{p}") && strcmp(signs[i].text, "{t}") && sregexp(signs[i].text, "^{[ct]:[0-9]*|.*}$") && sregexp(signs[i].text, "^{s:.*|.*}$") && sregexp(signs[i].text, "^{b|.*}$"))
		*w = textwidth(signs[i].text) + 5;
	*h = 14;
	*x0 = (signs[i].ju == 2) ? signs[i].x - *w :
	      (signs[i].ju == 1) ? signs[i].x - *w/2 : signs[i].x;
	*y0 = (signs[i].y > 18) ? signs[i].y - 18 : signs[i].y + 4;
}

void add_sign_ui(pixel *vid_buf, int mx, int my)
{
	int i, w, h, x, y, nm=0, ju;
	int x0=(XRES-204)/2,y0=(YRES-80)/2,b=1,bq;
	ui_edit ed;

	// if currently moving a sign, stop doing so
	if (MSIGN!=-1)
	{
		MSIGN = -1;
		return;
	}

	// check if it is an existing sign
	for (i=0; i<MAXSIGNS; i++)
		if (signs[i].text[0])
		{
			get_sign_pos(i, &x, &y, &w, &h);
			if (mx>=x && mx<=x+w && my>=y && my<=y+h)
				break;
		}
	// else look for empty spot
	if (i >= MAXSIGNS)
	{
		nm = 1;
		for (i=0; i<MAXSIGNS; i++)
			if (!signs[i].text[0])
				break;
	}
	if (i >= MAXSIGNS)
		return;
	if (nm)
	{
		signs[i].x = mx;
		signs[i].y = my;
		signs[i].ju = 1;
	}

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	ui_edit_init(&ed, x0+25, y0+25, 184, 14);
	ed.def = "[message]";
	ed.cursor = ed.cursorstart = strlen(signs[i].text);
	strcpy(ed.str, signs[i].text);
	ju = signs[i].ju;

	fillrect(vid_buf, -1, -1, XRES, YRES+MENUSIZE, 0, 0, 0, 192);
	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		drawrect(vid_buf, x0, y0, 217, 80, 192, 192, 192, 255);
		clearrect(vid_buf, x0, y0, 217, 80);
		drawtext(vid_buf, x0+8, y0+8, nm ? "New sign:" : "Edit sign:", 255, 255, 255, 255);
		drawtext(vid_buf, x0+12, y0+23, "\xA1", 32, 64, 128, 255);
		drawtext(vid_buf, x0+12, y0+23, "\xA0", 255, 255, 255, 255);
		drawrect(vid_buf, x0+8, y0+20, 201, 16, 192, 192, 192, 255);
		ui_edit_draw(vid_buf, &ed);
		drawtext(vid_buf, x0+8, y0+46, "Justify:", 255, 255, 255, 255);
		draw_icon(vid_buf, x0+50, y0+42, (char)0x9D, ju == 0);
		draw_icon(vid_buf, x0+68, y0+42, (char)0x9E, ju == 1);
		draw_icon(vid_buf, x0+86, y0+42, (char)0x9F, ju == 2);





		if (!nm)
		{
			drawtext(vid_buf, x0+138, y0+45, "\x86", 160, 48, 32, 255);
			drawtext(vid_buf, x0+138, y0+45, "\x85", 255, 255, 255, 255);
			drawtext(vid_buf, x0+152, y0+46, "Delete", 255, 255, 255, 255);
			drawrect(vid_buf, x0+134, y0+42, 50, 15, 255, 255, 255, 255);
			drawrect(vid_buf,x0+104,y0+42,26,15,255,255,255,255);
			drawtext(vid_buf, x0+110, y0+48, "Mv.", 255, 255, 255, 255);
		}

		drawtext(vid_buf, x0+5, y0+69, "OK", 255, 255, 255, 255);
		drawrect(vid_buf, x0, y0+64, 217, 16, 192, 192, 192, 255);
#ifdef OGLR
		clearScreen(1.0f);
#endif
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		ui_edit_process(mx, my, b, bq, &ed);

		if (b && !bq && mx>=x0+50 && mx<=x0+67 && my>=y0+42 && my<=y0+59)
			ju = 0;
		if (b && !bq && mx>=x0+68 && mx<=x0+85 && my>=y0+42 && my<=y0+59)
			ju = 1;
		if (b && !bq && mx>=x0+86 && mx<=x0+103 && my>=y0+42 && my<=y0+59)
			ju = 2;

		if (!nm && b && !bq && mx>=x0+104 && mx<=x0+130 && my>=y0+42 && my<=y0+59)
		{
			MSIGN = i;
			break;
		}
		if (b && !bq && mx>=x0+9 && mx<x0+23 && my>=y0+22 && my<y0+36)
			break;
		if (b && !bq && mx>=x0 && mx<x0+192 && my>=y0+64 && my<=y0+80)
			break;

		if (!nm && b && !bq && mx>=x0+134 && my>=y0+42 && mx<=x0+184 && my<=y0+59)
		{
			signs[i].text[0] = 0;
			return;
		}

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
		{
			if (!ed.focus)
				return;
			ed.focus = 0;
		}
	}

	strcpy(signs[i].text, ed.str);
	signs[i].ju = ju;
}

void ui_edit_init(ui_edit *ed, int x, int y, int w, int h)
{
	ed->x = x;
	ed->y = y;
	ed->w = w;
	ed->h = h;
	ed->nx = 1;
	ed->def = "";
	strcpy(ed->str, "");
	ed->focus = 1;
	ed->alwaysFocus = 0;
	ed->hide = 0;
	ed->multiline = 0;
	ed->limit = 255;
	ed->cursor = ed->cursorstart = 0;
	ed->highlightstart = ed->highlightlength = 0;
	ed->resizable = ed->resizespeed = 0;
	ed->lastClick = ed->numClicks = 0;
	ed->overDelete = 0;
}

int ui_edit_draw(pixel *vid_buf, ui_edit *ed)
{
	int cx, i, cy, ret = 12;
	char echo[1024], *str, highlightstr[1024];

	if (ed->cursor>ed->cursorstart)
	{
		ed->highlightstart = ed->cursorstart;
		ed->highlightlength = ed->cursor-ed->cursorstart;
	}
	else
	{
		ed->highlightstart = ed->cursor;
		ed->highlightlength = ed->cursorstart-ed->cursor;
	}
	if (ed->hide)
	{
		for (i=0; ed->str[i]; i++)
			echo[i] = (char)0x8D;
		echo[i] = 0;
		str = echo;
	}
	else
		str = ed->str;

	if (ed->str[0])
	{
		int deletecolor = 127+ed->overDelete*64;
		if (ed->multiline) {
			ret = drawtextwrap(vid_buf, ed->x, ed->y, ed->w-14, 0, str, 255, 255, 255, 255);
			if (ed->highlightlength)
			{
				strncpy(highlightstr, &str[ed->highlightstart], ed->highlightlength);
				highlightstr[ed->highlightlength] = 0;
				drawhighlightwrap(vid_buf, ed->x, ed->y, ed->w-14, 0, ed->str, ed->highlightstart, ed->highlightlength);
			}
			drawtext(vid_buf, ed->x+ed->w-11, ed->y-1, "\xAA", deletecolor, deletecolor, deletecolor, 255);
		} else {
			ret = drawtext(vid_buf, ed->x, ed->y, str, 255, 255, 255, 255);
			if (ed->highlightlength)
			{
				strncpy(highlightstr, &str[ed->highlightstart], ed->highlightlength);
				highlightstr[ed->highlightlength] = 0;
				drawhighlight(vid_buf, ed->x+textwidth(str)-textwidth(&str[ed->highlightstart]), ed->y, highlightstr);
			}
			drawtext(vid_buf, ed->x+ed->w-11, ed->y-1, "\xAA", deletecolor, deletecolor, deletecolor, 255);
		}
	}
	else if (!ed->focus)
		drawtext(vid_buf, ed->x, ed->y, ed->def, 128, 128, 128, 255);
	if (ed->focus && ed->numClicks < 2)
	{
		if (ed->multiline) {
			textnpos(str, ed->cursor, ed->w-14, &cx, &cy);
		} else {
			cx = textnwidth(str, ed->cursor);
			cy = 0;
		}

		for (i=-3; i<9; i++)
			drawpixel(vid_buf, ed->x+cx, ed->y+i+cy, 255, 255, 255, 255);
	}
	if (ed->resizable && ed->multiline)
	{
		int diff = ((ret+2)-ed->h)/5;
		if (diff == 0)
			ed->h = ret+2;
		else
			ed->h += diff;
		ret = ed->h-2;
	}
	return ret;
}

void findWordPosition(const char *s, int position, int *cursorStart, int *cursorEnd, char *spaces)
{
	int wordLength = 0, totalLength = 0, strLength = strlen(s);
	while (totalLength < strLength)
	{
		wordLength = strcspn(s, spaces);
		if (totalLength + wordLength >= position)
		{
			*cursorStart = totalLength;
			*cursorEnd = totalLength+wordLength;
			return;
		}
		s += wordLength+1;
		totalLength += wordLength+1;
	}
	*cursorStart = totalLength;
	*cursorEnd = totalLength+wordLength;
}

void ui_edit_process(int mx, int my, int mb, int mbq, ui_edit *ed)
{
	char ch, ts[2], echo[1024], *str = ed->str;
	int l, i;
#ifdef RAWINPUT
	char *p;
#endif

	if (ed->alwaysFocus)
		ed->focus = 1;
	if (ed->cursor>ed->cursorstart)
	{
		ed->highlightstart = ed->cursorstart;
		ed->highlightlength = ed->cursor-ed->cursorstart;
	}
	else
	{
		ed->highlightstart = ed->cursor;
		ed->highlightlength = ed->cursorstart-ed->cursor;
	}
	if (ed->hide)
	{
		for (i=0; ed->str[i]; i++)
			echo[i] = (char)0x8D;
		echo[i] = 0;
		str = echo;
	}
	if (mx>=ed->x+ed->w-11 && mx<ed->x+ed->w && my>=ed->y-5 && my<ed->y+11) //on top of the delete button
	{
		if (!ed->overDelete && !mb)
			ed->overDelete = 1;
		else if (mb && !mbq)
			ed->overDelete = 2;
		if (!mb && mbq && ed->overDelete == 2)
		{
			ed->focus = 1;
			ed->cursor = 0;
			ed->str[0] = 0;
			if (!ed->numClicks)
				ed->numClicks = 1;
		}
	}
	else if (mb && (ed->focus || !mbq) && mx>=ed->x-ed->nx && mx<ed->x+ed->w && my>=ed->y-5 && my<ed->y+(ed->multiline?ed->h:11)) //clicking / dragging over textbox
	{
		ed->focus = 1;
		ed->overDelete = 0;
		ed->cursor = textposxy(str, ed->w-14, mx-ed->x, my-ed->y);
		if (!ed->numClicks)
			ed->numClicks = 1;
	}
	else if (mb && !mbq) //a first click anywhere outside
	{
		if (!ed->alwaysFocus)
		{
			ed->focus = 0;
			ed->cursor = ed->cursorstart = 0;
		}
	}
	else //when a click has moved outside the textbox
	{
		if (ed->numClicks && mb && (ed->focus || !mbq))
		{
			if (my <= ed->y-5)
				ed->cursor = 0;
			else if (my >= ed->y+ed->h)
				ed->cursor = strlen(ed->str);
		}
		ed->overDelete = 0;
	}
	if (ed->focus && sdl_key)
	{
		l = strlen(ed->str);
		switch (sdl_key)
		{
		case SDLK_HOME:
			ed->cursor = ed->cursorstart = 0;
			break;
		case SDLK_END:
			ed->cursor = ed->cursorstart = l;
			break;
		case SDLK_LEFT:
			if (ed->cursor > 0)
				ed->cursor --;
			if (ed->cursor > 0 && ed->str[ed->cursor-1] == '\b')
			{
				ed->cursor--;
			}
			ed->cursorstart = ed->cursor;
			break;
		case SDLK_RIGHT:
			if (ed->cursor < l && ed->str[ed->cursor] == '\b')
			{
				ed->cursor++;
			}
			if (ed->cursor < l)
				ed->cursor ++;
			ed->cursorstart = ed->cursor;
			break;
		case SDLK_DELETE:
			if (sdl_mod & (KMOD_LCTRL|KMOD_RCTRL))
				ed->str[ed->cursor] = 0;
			else if (ed->highlightlength)
			{
				memmove(ed->str+ed->highlightstart, ed->str+ed->highlightstart+ed->highlightlength, l-ed->highlightstart);
				ed->cursor = ed->highlightstart;
			}
			else if (ed->cursor < l)
			{
				if (ed->str[ed->cursor] == '\b')
				{
					memmove(ed->str+ed->cursor, ed->str+ed->cursor+2, l-ed->cursor);
				}
				else
					memmove(ed->str+ed->cursor, ed->str+ed->cursor+1, l-ed->cursor);
			}
			ed->cursorstart = ed->cursor;
			break;
		case SDLK_BACKSPACE:
			if (sdl_mod & (KMOD_LCTRL|KMOD_RCTRL))
			{
				if (ed->cursor > 0)
					memmove(ed->str, ed->str+ed->cursor, l-ed->cursor+1);
				ed->cursor = 0;
			}
			else if (ed->highlightlength)
			{
				memmove(ed->str+ed->highlightstart, ed->str+ed->highlightstart+ed->highlightlength, l-ed->highlightstart);
				ed->cursor = ed->highlightstart;
			}
			else if (ed->cursor > 0)
			{
				ed->cursor--;
				memmove(ed->str+ed->cursor, ed->str+ed->cursor+1, l-ed->cursor);
				if (ed->cursor > 0 && ed->str[ed->cursor-1] == '\b')
				{
					ed->cursor--;
					memmove(ed->str+ed->cursor, ed->str+ed->cursor+1, l-ed->cursor);
				}
			}
			ed->cursorstart = ed->cursor;
			break;
		default:
			if(sdl_mod & (KMOD_CTRL) && sdl_key=='c')//copy
			{
				if (ed->highlightlength)
				{
					char highlightstr[1024];
					strncpy(highlightstr, &str[ed->highlightstart], ed->highlightlength);
					highlightstr[ed->highlightlength] = 0;
					clipboard_push_text(highlightstr);
				}
				else
					clipboard_push_text(ed->str);
				break;
			}
			else if(sdl_mod & (KMOD_CTRL) && sdl_key=='v')//paste
			{
				char *paste = clipboard_pull_text();
				int pl = strlen(paste);
				if ((textwidth(str)+textwidth(paste) > ed->w-14 && !ed->multiline) || (pl+strlen(ed->str)>ed->limit) || (float)(((textwidth(str)+textwidth(paste))/(ed->w-14)*12) > ed->h && ed->multiline && ed->limit != 1023))
					break;
				if (ed->highlightlength)
				{
					memmove(ed->str+ed->highlightstart, ed->str+ed->highlightstart+ed->highlightlength, l-ed->highlightstart);
					ed->cursor = ed->highlightstart;
				}
				memmove(ed->str+ed->cursor+pl, ed->str+ed->cursor, l-ed->cursor+1);
				memcpy(ed->str+ed->cursor,paste,pl);
				ed->cursor += pl;
				ed->cursorstart = ed->cursor;
				break;
			}
			else if(sdl_mod & (KMOD_CTRL) && sdl_key=='a')//highlight all
			{
				ed->cursorstart = 0;
				ed->cursor = l;
			}
#ifdef RAWINPUT
			if (sdl_key>=SDLK_SPACE && sdl_key<=SDLK_z && l<ed->limit)
			{
				if (ed->highlightlength)
				{
					memmove(ed->str+ed->highlightstart, ed->str+ed->highlightstart+ed->highlightlength, l-ed->highlightstart);
					ed->cursor = ed->highlightstart;
				}
				ch = sdl_key;
				if ((sdl_mod & (KMOD_LSHIFT|KMOD_RSHIFT|KMOD_CAPS)))
				{
					if (ch>='a' && ch<='z')
						ch &= ~0x20;
					p = strchr(shift_0, ch);
					if (p)
						ch = shift_1[p-shift_0];
				}
				ts[0]=ed->hide?0x8D:ch;
				ts[1]=0;
				if ((textwidth(str)+textwidth(ts) > ed->w-14 && !ed->multiline) || (float)(((textwidth(str)+textwidth(ts))/(ed->w-14)*12) > ed->h && ed->multiline && ed->limit != 1023))
					break;
				memmove(ed->str+ed->cursor+1, ed->str+ed->cursor, l+1-ed->cursor);
				ed->str[ed->cursor] = ch;
				ed->cursor++;
				ed->cursorstart = ed->cursor;
			}
#else
			if ((sdl_mod & (KMOD_CTRL)) && (svf_admin || svf_mod))
			{
				if (ed->cursor > 1 && ed->str[ed->cursor-2] == '\b')
					break;
				if (sdl_key=='w' || sdl_key=='g' || sdl_key=='o' || sdl_key=='r' || sdl_key=='l' || sdl_key=='b' || sdl_key=='t')// || sdl_key=='p')
					ch = sdl_key;
				else
					break;
				if (ed->highlightlength)
				{
					memmove(ed->str+ed->highlightstart, ed->str+ed->highlightstart+ed->highlightlength, l-ed->highlightstart);
					ed->cursor = ed->highlightstart;
				}
				if ((textwidth(str) > ed->w-14 && !ed->multiline) || (float)(((textwidth(str))/(ed->w-14)*12) > ed->h && ed->multiline && ed->limit != 1023))
					break;
				memmove(ed->str+ed->cursor+2, ed->str+ed->cursor, l+2-ed->cursor);
				ed->str[ed->cursor] = '\b';
				ed->str[ed->cursor+1] = ch;
				ed->cursor+=2;
				ed->cursorstart = ed->cursor;
			}
			if (sdl_ascii>=' ' && sdl_ascii<127 && l<ed->limit)
			{
				if (ed->highlightlength)
				{
					memmove(ed->str+ed->highlightstart, ed->str+ed->highlightstart+ed->highlightlength, l-ed->highlightstart);
					ed->cursor = ed->highlightstart;
				}
				ch = sdl_ascii;
				ts[0]=ed->hide?0x8D:ch;
				ts[1]=0;
				if ((textwidth(str)+textwidth(ts) > ed->w-14 && !ed->multiline) || (float)(((textwidth(str)+textwidth(ts))/(ed->w-14)*12) > ed->h && ed->multiline && ed->limit != 1023))
					break;
				memmove(ed->str+ed->cursor+1, ed->str+ed->cursor, l+1-ed->cursor);
				ed->str[ed->cursor] = ch;
				ed->cursor++;
				ed->cursorstart = ed->cursor;
			}
#endif
			break;
		}
	}
	if (mb && !mbq && ed->focus && mx>=ed->x-ed->nx && mx<ed->x+ed->w && my>=ed->y-5 && my<ed->y+(ed->multiline?ed->h:11))
	{
		int clickTime = SDL_GetTicks()-ed->lastClick;
		ed->lastClick = SDL_GetTicks();
		if (clickTime < 300)
		{
			ed->clickPosition = ed->cursor;
			if (ed->numClicks == 2)
				ed->numClicks = 3;
			else
				ed->numClicks = 2;
		}
		ed->cursorstart = ed->cursor;
	}
	else if (!mb && SDL_GetTicks()-ed->lastClick > 300)
		ed->numClicks = 0;
	if (ed->numClicks >= 2)
	{
		int start = 0, end = strlen(ed->str);
		char *spaces = " .,!?\n";
		if (ed->numClicks == 3)
			spaces = "\n";
		findWordPosition(ed->str, ed->cursor, &start, &end, spaces);
		if (start < ed->clickPosition)
		{
			ed->cursorstart = start;
			findWordPosition(ed->str, ed->clickPosition, &start, &end, spaces);
			ed->cursor = end;
		}
		else
		{
			ed->cursorstart = end;
			findWordPosition(ed->str, ed->clickPosition, &start, &end, spaces);
			ed->cursor = start;
		}
	}
}

void ui_label_init(ui_label *label, int x, int y, int w, int h)
{
	label->x = x;
	label->y = y;
	label->w = w;
	label->h = h;
	label->maxHeight = 0;
	strcpy(label->str, "");
	label->focus = 0;
	label->multiline = 1;
	label->cursor = label->cursorstart = 0;
	label->highlightstart = label->highlightlength = 0;
	label->lastClick = label->numClicks = 0;
}

int ui_label_draw(pixel *vid_buf, ui_label *ed)
{
	char highlightstr[1024];
	int ret = 0, heightlimit = ed->maxHeight;

	if (ed->cursor>ed->cursorstart)
	{
		ed->highlightstart = ed->cursorstart;
		ed->highlightlength = ed->cursor-ed->cursorstart;
	}
	else
	{
		ed->highlightstart = ed->cursor;
		ed->highlightlength = ed->cursorstart-ed->cursor;
	}

	if (ed->str[0])
	{
		if (ed->multiline) {
			ret = drawtextwrap(vid_buf, ed->x, ed->y, ed->w-14, heightlimit, ed->str, 255, 255, 255, 185);
			if (ed->maxHeight)
				ed->h = ret;
			if (ed->highlightlength)
			{
				drawhighlightwrap(vid_buf, ed->x, ed->y, ed->w-14, heightlimit, ed->str, ed->highlightstart, ed->highlightlength);
			}
		} else {
			ret = drawtext(vid_buf, ed->x, ed->y, ed->str, 255, 255, 255, 255);
			if (ed->highlightlength)
			{
				strncpy(highlightstr, &ed->str[ed->highlightstart], ed->highlightlength);
				highlightstr[ed->highlightlength] = 0;
				drawhighlight(vid_buf, ed->x+textwidth(ed->str)-textwidth(&ed->str[ed->highlightstart]), ed->y, highlightstr);
			}
		}
	}
	return ret;
}

void ui_label_process(int mx, int my, int mb, int mbq, ui_label *ed)
{
	if (ed->cursor>ed->cursorstart)
	{
		ed->highlightstart = ed->cursorstart;
		ed->highlightlength = ed->cursor-ed->cursorstart;
	}
	else
	{
		ed->highlightstart = ed->cursor;
		ed->highlightlength = ed->cursorstart-ed->cursor;
	}
	
	if (mb && (ed->focus || !mbq) && mx>=ed->x && mx<ed->x+ed->w && my>=ed->y-2 && my<ed->y+(ed->multiline?ed->h:11))
	{
		ed->focus = 1;
		ed->cursor = textposxy(ed->str, ed->w-14, mx-ed->x, my-ed->y);
		if (!ed->numClicks)
			ed->numClicks = 1;
	}
	else if (mb && !mbq)
	{
		ed->focus = 0;
		ed->cursor = ed->cursorstart = 0;
	}
	else if (mb && (ed->focus || !mbq))
	{
		if (my <= ed->y-2)
			ed->cursor = 0;
		else if (my >= ed->y+ed->h)
			ed->cursor = strlen(ed->str);
	}

	if (ed->focus && sdl_key)
	{
		if(sdl_mod & (KMOD_CTRL) && sdl_key=='c')//copy
		{
			if (ed->highlightlength)
			{
				char highlightstr[1024];
				strncpy(highlightstr, &ed->str[ed->highlightstart], ed->highlightlength);
				highlightstr[ed->highlightlength] = 0;
				clipboard_push_text(highlightstr);
			}
			else
				clipboard_push_text(ed->str);
		}
		else if(sdl_mod & (KMOD_CTRL) && sdl_key=='a')//highlight all
		{
			ed->cursorstart = 0;
			ed->cursor = strlen(ed->str);
		}
	}
	if (mb && !mbq && ed->focus)
	{
		int clickTime = SDL_GetTicks()-ed->lastClick;
		ed->lastClick = SDL_GetTicks();
		if (clickTime < 300)
		{
			ed->clickPosition = ed->cursor;
			if (ed->numClicks == 2)
				ed->numClicks = 3;
			else
				ed->numClicks = 2;
		}
		ed->cursorstart = ed->cursor;
	}
	else if (!mb && SDL_GetTicks()-ed->lastClick > 300)
		ed->numClicks = 0;
	if (ed->numClicks >= 2)
	{
		int start = 0, end = strlen(ed->str);
		char *spaces = " .,!?\n";
		if (ed->numClicks == 3)
			spaces = "\n";
		findWordPosition(ed->str, ed->cursor, &start, &end, spaces);
		if (start < ed->clickPosition)
		{
			ed->cursorstart = start;
			findWordPosition(ed->str, ed->clickPosition, &start, &end, spaces);
			ed->cursor = end;
		}
		else
		{
			ed->cursorstart = end;
			findWordPosition(ed->str, ed->clickPosition, &start, &end, spaces);
			ed->cursor = start;
		}
	}
}

void ui_list_process(pixel * vid_buf, int mx, int my, int mb, ui_list *ed)
{
	int i, ystart, selected = 0;
	if(ed->selected > ed->count || ed->selected < -1)
	{
		ed->selected = -1;
	}
	if(mx > ed->x && mx < ed->x+ed->w && my > ed->y && my < ed->y+ed->h)
	{
		ed->focus = 1;
		if(mb)
		{
			ystart = ed->y-(ed->count*8);
			if(ystart < 5)
				ystart = 5;
			while (!sdl_poll())
			{
				mb = mouse_get_state(&mx, &my);
				if (!mb)
					break;
			}

			while (!sdl_poll() && !selected)
			{
				mb = mouse_get_state(&mx, &my);
				for(i = 0; i < ed->count; i++)
				{
					if(mx > ed->x && mx < ed->x+ed->w && my > (ystart + i*16) && my < (ystart + i * 16) + 16)
					{
						if(mb){
							ed->selected = i;
							selected = 1;
						}
						fillrect(vid_buf, ed->x, ystart + i * 16, ed->w, 16, 255, 255, 255, 25);
						drawtext(vid_buf, ed->x + 4, ystart + i * 16 + 5, ed->items[i], 255, 255, 255, 255);
					}
					else
					{
						drawtext(vid_buf, ed->x + 4, ystart + i * 16 + 5, ed->items[i], 192, 192, 192, 255);
					}
					draw_line(vid_buf, ed->x, ystart + i * 16, ed->x+ed->w, ystart + i * 16, 128, 128, 128, XRES+BARSIZE);
				}
				drawrect(vid_buf, ed->x, ystart, ed->w, ed->count*16, 255, 255, 255, 255);
#ifdef OGLR
				clearScreen(1.0f);
#endif
				sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
				clearrect(vid_buf, ed->x-2, ystart-2, ed->w+4, (ed->count*16)+4);

				if(!selected && mb)
					break;
			}
			while (!sdl_poll())
			{
				mb = mouse_get_state(&mx, &my);
				if (!mb)
					break;
			}
			
			if(ed->selected!=-1)
				strcpy(ed->str, ed->items[ed->selected]);
		}
	}
	else
	{
		ed->focus = 0;
	}
}

void ui_list_draw(pixel *vid_buf, ui_list *ed)
{
	if(ed->selected > ed->count || ed->selected < -1)
	{
		ed->selected = -1;
	}
	if (ed->focus)
	{
		drawrect(vid_buf, ed->x, ed->y, ed->w, ed->h, 255, 255, 255, 255);
	}
	else
	{
		drawrect(vid_buf, ed->x, ed->y, ed->w, ed->h, 192, 192, 192, 255);
	}
	if(ed->selected!=-1)
	{
		drawtext(vid_buf, ed->x+4, ed->y+5, ed->items[ed->selected], 255, 255, 255, 255);
	}
	else
	{
		drawtext(vid_buf, ed->x+4, ed->y+5, ed->def, 192, 192, 192, 255);
	}
}

void ui_checkbox_draw(pixel *vid_buf, ui_checkbox *ed)
{
	int w = 12;
	if (ed->checked)
	{
		drawtext(vid_buf, ed->x+2, ed->y+2, "\xCF", 128, 128, 128, 255);
	}
	if (ed->focus)
	{
		drawrect(vid_buf, ed->x, ed->y, w, w, 255, 255, 255, 255);
	}
	else
	{
		drawrect(vid_buf, ed->x, ed->y, w, w, 128, 128, 128, 255);
	}
}

void ui_checkbox_process(int mx, int my, int mb, int mbq, ui_checkbox *ed)
{
	int w = 12;

	if (mb && !mbq)
	{
		if (mx>=ed->x && mx<=ed->x+w && my>=ed->y && my<=ed->y+w)
		{
			ed->checked = (ed->checked)?0:1;
		}
	}
	else
	{
		if (mx>=ed->x && mx<=ed->x+w && my>=ed->y && my<=ed->y+w)
		{
			ed->focus = 1;
		}
		else
		{
			ed->focus = 0;
		}
	}
}

void ui_radio_draw(pixel *vid_buf, ui_checkbox *ed)
{
	if (ed->checked)
	{
		int nx, ny;
		for(nx=-6; nx<=6; nx++)
			for(ny=-6; ny<=6; ny++)
				if((nx*nx+ny*ny)<10)
					blendpixel(vid_buf, ed->x+6+nx, ed->y+6+ny, 128, 128, 128, 255);
	}
	if (ed->focus)
	{
		int nx, ny;
		for(nx=-6; nx<=6; nx++)
			for(ny=-6; ny<=6; ny++)
				if((nx*nx+ny*ny)<40 && (nx*nx+ny*ny)>28)
					blendpixel(vid_buf, ed->x+6+nx, ed->y+6+ny, 255, 255, 255, 255);
	}
	else
	{
		int nx, ny;
		for(nx=-6; nx<=6; nx++)
			for(ny=-6; ny<=6; ny++)
				if((nx*nx+ny*ny)<40 && (nx*nx+ny*ny)>28)
					blendpixel(vid_buf, ed->x+6+nx, ed->y+6+ny, 128, 128, 128, 255);
	}
}

void ui_radio_process(int mx, int my, int mb, int mbq, ui_checkbox *ed)
{
	int w = 12;

	if (mb && !mbq)
	{
		if (mx>=ed->x && mx<=ed->x+w && my>=ed->y && my<=ed->y+w)
		{
			ed->checked = (ed->checked)?0:1;
		}
	}
	else
	{
		if (mx>=ed->x && mx<=ed->x+w && my>=ed->y && my<=ed->y+w)
		{
			ed->focus = 1;
		}
		else
		{
			ed->focus = 0;
		}
	}
}

void ui_copytext_draw(pixel *vid_buf, ui_copytext *ed)
{
	int g = 180, i = 0;
	if (!ed->state)
	{
		if (ed->hover)
			i = 0;
		else
			i = 100;
		g = 255;
		drawtext(vid_buf, (ed->x+(ed->width/2))-(textwidth("Click the box below to copy the save ID")/2), ed->y-12, "Click the box below to copy the save ID", 255, 255, 255, 255-i);
	}
	else
	{
		i = 0;
		if (ed->state == 2)
			g = 230;
		else
			g = 190;
		drawtext(vid_buf, (ed->x+(ed->width/2))-(textwidth("Copied!")/2), ed->y-12, "Copied!", 255, 255, 255, 255-i);
	}

	drawrect(vid_buf, ed->x, ed->y, ed->width, ed->height, g, 255, g, 255-i);
	drawrect(vid_buf, ed->x+1, ed->y+1, ed->width-2, ed->height-2, g, 255, g, 100-i);
	drawtext(vid_buf, ed->x+6, ed->y+5, ed->text, g, 255, g, 230-i);
}

void ui_copytext_process(int mx, int my, int mb, int mbq, ui_copytext *ed)
{
	if (my>=ed->y && my<=ed->y+ed->height && mx>=ed->x && mx<=ed->x+ed->width)
	{
		if (mb && !mbq)
		{
			clipboard_push_text(ed->text);
			ed->state = 2;
		}
		ed->hover = 1;
	}
	else
		ed->hover = 0;

	if (ed->state == 2 && !(mb && ed->hover))
		ed->state = 1;
}

void ui_richtext_draw(pixel *vid_buf, ui_richtext *ed)
{
	ed->str[511] = 0;
	ed->printstr[511] = 0;
	drawtext(vid_buf, ed->x, ed->y, ed->printstr, 255, 255, 255, 255);
}

int markup_getregion(char *text, char *action, char *data, char *atext){
	int datamarker = 0;
	int terminator = 0;
	int minit;
	if (sregexp(text, "^{a:.*|.*}")==0)
	{
		*action = text[1];
		for (minit=3; text[minit-1] != '|'; minit++)
			datamarker = minit + 1;
		for (minit=datamarker; text[minit-1] != '}'; minit++)
			terminator = minit + 1;
		strncpy(data, text+3, datamarker-4);
		strncpy(atext, text+datamarker, terminator-datamarker-1);
		return terminator;
	}
	else
	{
		return 0;
	}	
}

void ui_richtext_settext(char *text, ui_richtext *ed)
{
	int pos = 0, action = 0, ppos = 0, ipos = 0;
	memset(ed->printstr, 0, 512);
	memset(ed->str, 0, 512);
	strcpy(ed->str, text);
	//strcpy(ed->printstr, text);
	for(action = 0; action < 6; action++){
		ed->action[action] = 0;	
		memset(ed->actiondata[action], 0, 256);
		memset(ed->actiontext[action], 0, 256);
	}
	action = 0;
	for(pos = 0; pos<512; ){
		if(!ed->str[pos])
			break;
		if(ed->str[pos] == '{'){
			int mulen = 0;
			mulen = markup_getregion(ed->str+pos, &ed->action[action], ed->actiondata[action], ed->actiontext[action]);
			if(mulen){
				ed->regionss[action] = ipos;
				ed->regionsf[action] = ipos + strlen(ed->actiontext[action]);
				//printf("%c, %s, %s [%d, %d]\n", ed->action[action], ed->actiondata[action], ed->actiontext[action], ed->regionss[action], ed->regionsf[action]);
				strcpy(ed->printstr+ppos, ed->actiontext[action]);
				ppos+=strlen(ed->actiontext[action]);
				ipos+=strlen(ed->actiontext[action]);
				pos+=mulen;
				action++;			
			} 
			else
			{
				pos++;			
			}
		} else {
			ed->printstr[ppos] = ed->str[pos];
			ppos++;
			pos++;
			ipos++;
			if(ed->str[pos] == '\b'){
				ipos-=2;			
			}
		}
	}
	ed->printstr[ppos] = 0;
	//printf("%s\n", ed->printstr);
}

void ui_richtext_process(int mx, int my, int mb, int mbq, ui_richtext *ed)
{
	int action = 0;
	int currentpos = 0;
	if(mx>ed->x && mx < ed->x+textwidth(ed->printstr) && my > ed->y && my < ed->y + 10 && mb && !mbq){
		currentpos = textwidthx(ed->printstr, mx-ed->x);
		for(action = 0; action < 6; action++){
			if(currentpos >= ed->regionss[action] && currentpos <= ed->regionsf[action])
			{	
				//Do action
				if(ed->action[action]=='a'){
					//Open link
					open_link(ed->actiondata[action]);	
				}
				break;
			}
		}
	}
}

void draw_svf_ui(pixel *vid_buf, int alternate)// all the buttons at the bottom
{
	int c;

	//the open browser button
	if(alternate)
	{
		fillrect(vid_buf, 0, YRES+(MENUSIZE-16)-1, 18, 16, 255, 255, 255, 255);
		drawtext(vid_buf, 4, YRES+(MENUSIZE-14), "\x81", 0, 0, 0, 255);
	} else {
		drawtext(vid_buf, 4, YRES+(MENUSIZE-14), "\x81", 255, 255, 255, 255);
		drawrect(vid_buf, 1, YRES+(MENUSIZE-16), 16, 14, 255, 255, 255, 255);
	}

	// the reload button
	c = (svf_last) ? 255 : 128;
	drawtext(vid_buf, 23, YRES+(MENUSIZE-14), "\x91", c, c, c, 255);
	drawrect(vid_buf, 19, YRES+(MENUSIZE-16), 16, 14, c, c, c, 255);

	// the save sim button
	if(alternate)
	{
		fillrect(vid_buf, 36, YRES+(MENUSIZE-16)-1, 152, 16, 255, 255, 255, 255);
		drawtext(vid_buf, 40, YRES+(MENUSIZE-14), "\x82", 0, 0, 0, 255);
		if(svf_fileopen)
			drawtext(vid_buf, 58, YRES+(MENUSIZE-12), svf_filename, 0, 0, 0, 255);
		else
			drawtext(vid_buf, 58, YRES+(MENUSIZE-12), "[save to disk]", 0, 0, 0, 255);
	} else {
		c = svf_login ? 255 : 128;
		drawtext(vid_buf, 40, YRES+(MENUSIZE-14), "\x82", c, c, c, 255);
		if (svf_open)
			drawtextmax(vid_buf, 58, YRES+(MENUSIZE-12), 125, svf_name, c, c, c, 255);
		else
			drawtext(vid_buf, 58, YRES+(MENUSIZE-12), "[untitled simulation]", c, c, c, 255);
		drawrect(vid_buf, 37, YRES+(MENUSIZE-16), 150, 14, c, c, c, 255);
		if (svf_open && svf_own)
			drawdots(vid_buf, 55, YRES+(MENUSIZE-15), 12, c, c, c, 255);
	}

	c = (svf_login && svf_open) ? 255 : 128;

	//the vote buttons
	drawrect(vid_buf, 189, YRES+(MENUSIZE-16), 14, 14, c, c, c, 255);
	drawrect(vid_buf, 203, YRES+(MENUSIZE-16), 14, 14, c, c, c, 255);

	if (svf_myvote==1 && (svf_login && svf_open))
	{
		fillrect(vid_buf, 189, YRES+(MENUSIZE-16), 14, 14, 0, 108, 10, 255);
	}
	else if (svf_myvote==-1 && (svf_login && svf_open))
	{
		fillrect(vid_buf, 203, YRES+(MENUSIZE-16), 14, 14, 108, 10, 0, 255);
	}
	drawtext(vid_buf, 192, YRES+(MENUSIZE-12), "\xCB", 0, 187, 18, c);
	drawtext(vid_buf, 205, YRES+(MENUSIZE-14), "\xCA", 187, 40, 0, c);

	c = svf_open ? 255 : 128;

	//the tags button
	drawtext(vid_buf, 222, YRES+(MENUSIZE-15), "\x83", c, c, c, 255);
	if (svf_tags[0])
		drawtextmax(vid_buf, 240, YRES+(MENUSIZE-12), XRES+BARSIZE-405, svf_tags, c, c, c, 255);
	else
		drawtext(vid_buf, 240, YRES+(MENUSIZE-12), "[no tags set]", c, c, c, 255);

	drawrect(vid_buf, 219, YRES+(MENUSIZE-16), XRES+BARSIZE-380, 14, c, c, c, 255);

	//the clear sim button------------some of the commented values are wrong
	drawtext(vid_buf, XRES-139+BARSIZE/*371*/, YRES+(MENUSIZE-14), "\x92", 255, 255, 255, 255);
	drawrect(vid_buf, XRES-143+BARSIZE/*367*/, YRES+(MENUSIZE-16), 16, 14, 255, 255, 255, 255);

	//the login button
	drawtext(vid_buf, XRES-122+BARSIZE/*388*/, YRES+(MENUSIZE-13), "\x84", 255, 255, 255, 255);
	if (svf_login)
		drawtextmax(vid_buf, XRES-104+BARSIZE/*406*/, YRES+(MENUSIZE-12), 66, svf_user, 255, 255, 255, 255);
	else
		drawtext(vid_buf, XRES-104+BARSIZE/*406*/, YRES+(MENUSIZE-12), "[sign in]", 255, 255, 255, 255);
	drawrect(vid_buf, XRES-125+BARSIZE/*385*/, YRES+(MENUSIZE-16), 91, 14, 255, 255, 255, 255);

	//te pause button
	if (sys_pause)
	{
		fillrect(vid_buf, XRES-17+BARSIZE/*493*/, YRES+(MENUSIZE-17), 16, 16, 255, 255, 255, 255);
		drawtext(vid_buf, XRES-14+BARSIZE/*496*/, YRES+(MENUSIZE-14), "\x90", 0, 0, 0, 255);
	}
	else
	{
		drawtext(vid_buf, XRES-14+BARSIZE/*496*/, YRES+(MENUSIZE-14), "\x90", 255, 255, 255, 255);
		drawrect(vid_buf, XRES-16+BARSIZE/*494*/, YRES+(MENUSIZE-16), 14, 14, 255, 255, 255, 255);
	}

	//The simulation options button, used to be the heat sim button
	{
		drawtext(vid_buf, XRES-156+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xCF", 255, 255, 255, 255);
		drawrect(vid_buf, XRES-159+BARSIZE/*494*/, YRES+(MENUSIZE-16), 14, 14, 255, 255, 255, 255);
	}

	//the view mode button
	addchar(vid_buf, XRES-29+BARSIZE, YRES+(MENUSIZE-13), 0xD8, 255, 0, 0, 255);
	addchar(vid_buf, XRES-29+BARSIZE, YRES+(MENUSIZE-13), 0xD9, 0, 255, 0, 255);
	addchar(vid_buf, XRES-29+BARSIZE, YRES+(MENUSIZE-13), 0xDA, 0, 0, 255, 255);
	drawrect(vid_buf, XRES-32+BARSIZE/*478*/, YRES+(MENUSIZE-16), 14, 14, 255, 255, 255, 255);

	// special icons for admin/mods
	if (svf_admin)
	{
		drawtext(vid_buf, XRES-45+BARSIZE/*463*/, YRES+(MENUSIZE-14), "\xC9", 232, 127, 35, 255);
		drawtext(vid_buf, XRES-45+BARSIZE/*463*/, YRES+(MENUSIZE-14), "\xC7", 255, 255, 255, 255);
		drawtext(vid_buf, XRES-45+BARSIZE/*463*/, YRES+(MENUSIZE-14), "\xC8", 255, 255, 255, 255);
	}
	else if (svf_mod)
	{
		drawtext(vid_buf, XRES-45+BARSIZE/*463*/, YRES+(MENUSIZE-14), "\xC9", 35, 127, 232, 255);
		drawtext(vid_buf, XRES-45+BARSIZE/*463*/, YRES+(MENUSIZE-14), "\xC7", 255, 255, 255, 255);
	}//else if(amd)
	//	drawtext(vid_buf, XRES-45/*465*/, YRES+(MENUSIZE-15), "\x97", 0, 230, 153, 255); Why is this here?
}

void error_ui(pixel *vid_buf, int err, char *txt)
{
	int x0=(XRES-240)/2,y0=YRES/2,b=1,bq,mx,my,textheight;
	char *msg;

	msg = (char*)malloc(strlen(txt)+16);
	if (err)
		sprintf(msg, "%03d %s", err, txt);
	else
		sprintf(msg, "%s", txt);
	textheight = textwrapheight(msg, 240);
	y0 -= (52+textheight)/2;
	if (y0<2)
		y0 = 2;
	if (y0+50+textheight>YRES)
		textheight = YRES-50-y0;

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		clearrect(vid_buf, x0-2, y0-2, 244, 52+textheight);
		drawrect(vid_buf, x0, y0, 240, 48+textheight, 192, 192, 192, 255);
		if (err)
			drawtext(vid_buf, x0+8, y0+8, "HTTP error:", 255, 64, 32, 255);
		else
			drawtext(vid_buf, x0+8, y0+8, "Error:", 255, 64, 32, 255);
		drawtextwrap(vid_buf, x0+8, y0+26, 224, 0, msg, 255, 255, 255, 255);
		drawtext(vid_buf, x0+5, y0+textheight+37, "Dismiss", 255, 255, 255, 255);
		drawrect(vid_buf, x0, y0+textheight+32, 240, 16, 192, 192, 192, 255);
#ifdef OGLR
		clearScreen(1.0f);
#endif
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		if (b && !bq && mx>=x0 && mx<x0+240 && my>=y0+textheight+32 && my<=y0+textheight+48)
			break;

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
			break;
	}

	free(msg);

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
}

typedef struct int_pair
{
	int first, second;
} int_pair;

int int_pair_cmp (const void * a, const void * b)
{
	int_pair *ap = (int_pair*)a;
	int_pair *bp = (int_pair*)b;
	return ( ap->first - bp->first );
}

void element_search_ui(pixel *vid_buf, Tool ** selectedLeft, Tool ** selectedRight)
{
	int windowHeight = 300, windowWidth = 240;
	int x0 = (XRES-windowWidth)/2, y0 = (YRES-windowHeight)/2, b = 1, bq, mx, my;
	int toolx = 0, tooly = 0, i, xoff, yoff, c, found;
	char tempCompare[512];
	char tempString[512];
	int_pair tempInts[PT_NUM];
	int selectedl = -1;
	int selectedr = -1;
	int firstResult = -1, hover = -1;

	ui_edit ed;
	ui_edit_init(&ed, x0+12, y0+30, windowWidth - 20, 14);
	ed.def = "[element name]";


	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		clearrect(vid_buf, x0-2, y0-2, windowWidth+4, windowHeight+4);
		drawrect(vid_buf, x0, y0, windowWidth, windowHeight, 192, 192, 192, 255);
		
		drawtext(vid_buf, x0+8, y0+8, "\xE6 Element Search", 255, 255, 255, 255);

		drawrect(vid_buf, ed.x-4, ed.y-5, ed.w+4, 16, 192, 192, 192, 255);
		ui_edit_draw(vid_buf, &ed);
		ui_edit_process(mx, my, b, bq, &ed);
		
		drawrect(vid_buf, ed.x-4, (ed.y-5)+20, ed.w+4, windowHeight-((ed.y-5)), 192, 192, 192, 255);
		xoff = (ed.x-4)+6;
		yoff = ((ed.y-5)+20)+4;
		toolx = 0;
		tooly = 0;
		
		drawtext(vid_buf, xoff+toolx+4, yoff+tooly+3, "Matches:", 255, 255, 255, 180);
		draw_line(vid_buf, xoff+toolx+2, yoff+tooly+14, xoff+toolx+5+(ed.w-16), yoff+tooly+14, 180, 180, 180, XRES+BARSIZE);
		tooly += 17;
		
		//Covert input to lower case
		c = 0;
		while (ed.str[c]) { tempString[c] = tolower(ed.str[c]); c++; } tempString[c] = 0;
		
		firstResult = -1;
		hover = -1;
		for(i = 0; i < PT_NUM; i++)
		{
			c = 0;
			while (ptypes[i].name[c]) { tempCompare[c] = tolower(ptypes[i].name[c]); c++; } tempCompare[c] = 0;
			if(strstr(tempCompare, tempString)!=0 && ptypes[i].enabled)
			{
				if(firstResult==-1)
					firstResult = i;
				toolx += draw_tool_xy(vid_buf, toolx+xoff, tooly+yoff, GetToolFromIdentifier(globalSim->elements[i].Identifier))+5;
				if (!bq && mx>=xoff+toolx-32 && mx<xoff+toolx && my>=yoff+tooly && my<yoff+tooly+15)
				{
					drawrect(vid_buf, xoff+toolx-32, yoff+tooly-1, 29, 17, 255, 55, 55, 255);
					hover = i;
				}
				else if (i == selectedl || GetToolFromIdentifier(globalSim->elements[i].Identifier) == *selectedLeft)
				{
					drawrect(vid_buf, xoff+toolx-32, yoff+tooly-1, 29, 17, 255, 55, 55, 255);
				}
				else if (i==selectedr || GetToolFromIdentifier(globalSim->elements[i].Identifier) == *selectedRight)
				{
					drawrect(vid_buf, xoff+toolx-32, yoff+tooly-1, 29, 17, 55, 55, 255, 255);
				}
				if(toolx > ed.w-4)
				{
					tooly += 18;
					toolx = 0;
				}
				if(tooly>windowHeight-((ed.y-5)+20))
					break;;
			}
		}
		
		if(toolx>0)
		{
			toolx = 0;
			tooly += 18;
		}
		
		if(tooly<windowHeight-((ed.y-5)+20))
		{
			drawtext(vid_buf, xoff+toolx+4, yoff+tooly+3, "Related:", 255, 255, 255, 180);
			draw_line(vid_buf, xoff+toolx+2, yoff+tooly+14, xoff+toolx+5+(ed.w-16), yoff+tooly+14, 180, 180, 180, XRES+BARSIZE);
			tooly += 17;
			
			found = 0;
			for(i = 0; i < PT_NUM; i++)
			{
				c = 0;
				while (ptypes[i].descs[c]) { tempCompare[c] = tolower(ptypes[i].descs[c]); c++; } tempCompare[c] = 0;
				if(strstr(tempCompare, tempString)!=0 && ptypes[i].enabled)
				{
					tempInts[found].first = (strstr(tempCompare, tempString)==NULL)?0:1;
					tempInts[found++].second = i;
				}
			}
			
			qsort(tempInts, found, sizeof(int_pair), int_pair_cmp);
			
			for(i = 0; i < found; i++)
			{
				if(firstResult==-1)
					firstResult = tempInts[i].second;
				toolx += draw_tool_xy(vid_buf, toolx+xoff, tooly+yoff, GetToolFromIdentifier(globalSim->elements[tempInts[i].second].Identifier))+5;
				if (!bq && mx>=xoff+toolx-32 && mx<xoff+toolx && my>=yoff+tooly && my<yoff+tooly+15)
				{
					drawrect(vid_buf, xoff+toolx-32, yoff+tooly-1, 29, 17, 255, 55, 55, 255);
					hover = tempInts[i].second;
				}
				else if (tempInts[i].second == selectedl || GetToolFromIdentifier(globalSim->elements[tempInts[i].second].Identifier) == *selectedLeft)
				{
					drawrect(vid_buf, xoff+toolx-32, yoff+tooly-1, 29, 17, 255, 55, 55, 255);
				}
				else if (tempInts[i].second == selectedr || GetToolFromIdentifier(globalSim->elements[tempInts[i].second].Identifier) == *selectedRight)
				{
					drawrect(vid_buf, xoff+toolx-32, yoff+tooly-1, 29, 17, 55, 55, 255, 255);
				}
				if(toolx > ed.w-4)
				{
					tooly += 18;
					toolx = 0;
				}
				if(tooly>windowHeight-((ed.y-5)+18))
					break;
			}
		}
		
		if(b==1 && hover!=-1)
		{
			selectedl = hover;
			break;
		}
		if(b==4 && hover!=-1)
		{
			selectedr = hover;
			break;
		}
		
		drawtext(vid_buf, x0+5, y0+windowHeight-12, "Dismiss", 255, 255, 255, 255);
		drawrect(vid_buf, x0, y0+windowHeight-16, windowWidth, 16, 192, 192, 192, 255);
#ifdef OGLR
		clearScreen(1.0f);
#endif
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		if (b && !bq && mx>=x0 && mx<x0+windowWidth && my>=y0+windowHeight-16 && my<=y0+windowHeight)
			break;

		if (sdl_key==SDLK_RETURN)
		{
			if(selectedl==-1)
				selectedl = firstResult;
			break;
		}
		if (sdl_key==SDLK_ESCAPE)
		{
			selectedl = -1;
			selectedr = -1;
			break;
		}
	}

	if(selectedl!=-1)
		*selectedLeft = GetToolFromIdentifier(globalSim->elements[selectedl].Identifier);
	if(selectedr!=-1)
		*selectedRight = GetToolFromIdentifier(globalSim->elements[selectedr].Identifier);
	
	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
}

char *input_ui(pixel *vid_buf, char *title, char *prompt, char *text, char *shadow)
{
	int xsize = 244;
	int ysize = 90;
	int x0=(XRES-xsize)/2,y0=(YRES-MENUSIZE-ysize)/2,b=1,bq,mx,my;

	ui_edit ed;
	ui_edit_init(&ed, x0+12, y0+50, xsize-20, 14);
	ed.def = shadow;
	ed.focus = 0;
	strncpy(ed.str, text, 254);

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		clearrect(vid_buf, x0-2, y0-2, xsize+4, ysize+4);
		drawrect(vid_buf, x0, y0, xsize, ysize, 192, 192, 192, 255);
		drawtext(vid_buf, x0+8, y0+8, title, 160, 160, 255, 255);
		drawtext(vid_buf, x0+8, y0+26, prompt, 255, 255, 255, 255);
		
		drawrect(vid_buf, ed.x-4, ed.y-5, ed.w+4, 16, 192, 192, 192, 255);

		ui_edit_draw(vid_buf, &ed);
		ui_edit_process(mx, my, b, bq, &ed);

		drawtext(vid_buf, x0+5, y0+ysize-11, "OK", 255, 255, 255, 255);
		drawrect(vid_buf, x0, y0+ysize-16, xsize, 16, 192, 192, 192, 255);

#ifdef OGLR
		clearScreen(1.0f);
#endif
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		if (b && !bq && mx>=x0 && mx<x0+xsize && my>=y0+ysize-16 && my<=y0+ysize)
			break;

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
			break;
	}

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
	return mystrdup(ed.str);
}

void prop_edit_ui(pixel *vid_buf, int x, int y, int flood)
{
	pixel * o_vid_buf;
	float valuef;
	unsigned char valuec;
	int valuei;
	unsigned int valueui;
	int format, pwht_property;
	size_t propoffset;
	char *listitems[] = {"type", "life", "ctype", "temp", "tmp", "tmp2", "vy", "vx", "x", "y", "dcolour", "flags"};
	int listitemscount = 12;
	int xsize = 244;
	int ysize = 87;
	int x0=(XRES-xsize)/2,y0=(YRES-MENUSIZE-ysize)/2,b=1,bq,mx,my;
	ui_list ed;
	ui_edit ed2;

	ed.x = x0+8;
	ed.y = y0+25;
	ed.w = xsize - 16;
	ed.h = 16;
	ed.def = "[property]";
	ed.selected = 0;
	ed.items = listitems;
	ed.count = listitemscount;
	
	ui_edit_init(&ed2, x0+12, y0+50, xsize-20, 14);
	ed2.def = "[value]";
	ed2.focus = 0;
	strncpy(ed2.str, "0", 254);
	strncpy(ed.str, "type", 254);

	if (x >= 0 && y >= 0 && x < XRES && y < YRES && (pmap[y][x]&0xFF) == PT_PWHT)
		flood = 0;

	o_vid_buf = (pixel*)calloc((YRES+MENUSIZE) * (XRES+BARSIZE), PIXELSIZE);
	if (o_vid_buf)
		memcpy(o_vid_buf, vid_buf, ((YRES+MENUSIZE) * (XRES+BARSIZE)) * PIXELSIZE);
	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		if (o_vid_buf)
			memcpy(vid_buf, o_vid_buf, ((YRES+MENUSIZE) * (XRES+BARSIZE)) * PIXELSIZE);
		clearrect(vid_buf, x0-2, y0-2, xsize+4, ysize+4);
		drawrect(vid_buf, x0, y0, xsize, ysize, 192, 192, 192, 255);
		drawtext(vid_buf, x0+8, y0+8, "Change particle property", 160, 160, 255, 255);
		//drawtext(vid_buf, x0+8, y0+26, prompt, 255, 255, 255, 255);
		
		//drawrect(vid_buf, ed.x-4, ed.y-5, ed.w+4, 16, 192, 192, 192, 255);
		drawrect(vid_buf, ed2.x-4, ed2.y-5, ed2.w+4, 16, 192, 192, 192, 255);

		ui_list_draw(vid_buf, &ed);
		ui_list_process(vid_buf, mx, my, b, &ed);
		ui_edit_draw(vid_buf, &ed2);
		ui_edit_process(mx, my, b, bq, &ed2);

		drawtext(vid_buf, x0+5, y0+ysize-11, "OK", 255, 255, 255, 255);
		drawrect(vid_buf, x0, y0+ysize-16, xsize, 16, 192, 192, 192, 255);

#ifdef OGLR
		clearScreen(1.0f);
#endif
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		if (b && !bq && mx>=x0 && mx<x0+xsize && my>=y0+ysize-16 && my<=y0+ysize)
			break;

		if (sdl_key == SDLK_RETURN)
			break;
		else if (sdl_key == SDLK_ESCAPE)
			goto exit;
		else if (sdl_key == SDLK_UP && ed.selected > 0)
		{
			ed.selected--;
			strcpy(ed.str, ed.items[ed.selected]);
		}
		else if (sdl_key == SDLK_DOWN && ed.selected < ed.count-1)
		{
			ed.selected++;
			strcpy(ed.str, ed.items[ed.selected]);
		}
	}

	if(ed.selected!=-1)
	{
		if (strcmp(ed.str,"type")==0){
			propoffset = offsetof(particle, type);
			format = 1;
			pwht_property = 0;
		} else if (strcmp(ed.str,"life")==0){
			propoffset = offsetof(particle, life);
			format = 0;
			pwht_property = 1;
		} else if (strcmp(ed.str,"ctype")==0){
			propoffset = offsetof(particle, ctype);
			format = 1;
			pwht_property = 2;
		} else if (strcmp(ed.str,"temp")==0){
			propoffset = offsetof(particle, temp);
			format = 2;
			pwht_property = 3;
		} else if (strcmp(ed.str,"tmp")==0){
			propoffset = offsetof(particle, tmp);
			format = 0;
			pwht_property = 4;
		} else if (strcmp(ed.str,"tmp2")==0){
			propoffset = offsetof(particle, tmp2);
			format = 0;
			pwht_property = 5;
		} else if (strcmp(ed.str,"vy")==0){
			propoffset = offsetof(particle, vy);
			format = 2;
			pwht_property = 6;
		} else if (strcmp(ed.str,"vx")==0){
			propoffset = offsetof(particle, vx);
			format = 2;
			pwht_property = 7;
		} else if (strcmp(ed.str,"x")==0){
			propoffset = offsetof(particle, x);
			format = 2;
			pwht_property = 8;
		} else if (strcmp(ed.str,"y")==0){
			propoffset = offsetof(particle, y);
			format = 2;
			pwht_property = 9;
		} else if (strcmp(ed.str,"dcolour")==0){
			propoffset = offsetof(particle, dcolour);
			format = 3;
			pwht_property = 10;
		} else if (strcmp(ed.str,"flags")==0){
			propoffset = offsetof(particle, flags);
			format = 3;
			pwht_property = 11;
		}
	} else {
		error_ui(vid_buf, 0, "Invalid property");
		goto exit;
	}
	
	if(format==0){
		sscanf(ed2.str, "%d", &valuei);
		if (flood)
			flood_prop(x, y, propoffset, &valuei, format);
	}
	if(format==1){
		int isint = 1, i;
		//Check if it's an element name
		for(i = 0; i < strlen(ed2.str); i++)
		{
			if(!(ed2.str[i] >= '0' && ed2.str[i] <= '9'))
			{
				isint = 0;
				break;
			}
		}
		if(isint)
		{
			sscanf(ed2.str, "%u", &valuei);
		}
		else
		{
			if(!console_parse_type(ed2.str, &valuei, NULL))
			{
				error_ui(vid_buf, 0, "Invalid element name");
				goto exit;
			}
		}
		if (pwht_property == 0 && (valuei < 0 || valuei > PT_NUM || !ptypes[valuei].enabled))
		{
			error_ui(vid_buf, 0, "Invalid element number");
			goto exit;
		}
		valuec = (unsigned char)valuei;
		if (flood)
			flood_prop(x, y, propoffset, &valuec, format);
	}
	if(format==2){
		sscanf(ed2.str, "%f", &valuef);
		if (flood)
			flood_prop(x, y, propoffset, &valuef, format);
	}
	if(format==3){
		int j;
		if(ed2.str[0] == '#') // #FFFFFFFF
		{
			//Convert to lower case
			for(j = 0; j < strlen(ed2.str); j++)
				ed2.str[j] = tolower(ed2.str[j]);
			sscanf(ed2.str, "#%x", &valueui);
			printf("%s, %u\n", ed2.str, valueui);
		}
		else if(ed2.str[0] == '0' && ed2.str[1] == 'x') // 0xFFFFFFFF
		{
			//Convert to lower case
			for(j = 0; j < strlen(ed2.str); j++)
				ed2.str[j] = tolower(ed2.str[j]);
			sscanf(ed2.str, "0x%x", &valueui);
			printf("%s, %u\n", ed2.str, valueui);
		}
		else
		{
			sscanf(ed2.str, "%d", &valueui);
		}
		if (flood)
			flood_prop(x, y, propoffset, &valueui, 0);
	}
	if (!flood)
	{
		prop_offset = propoffset;
		prop_format = format;
		if (prop_value)
			free(prop_value);
		if (format == 0)
		{
			prop_value = malloc(sizeof(int));
			memcpy(prop_value,&valuei,sizeof(int));
			if (x >= 0 && y >= 0 && x < XRES && y < YRES && (pmap[y][x]&0xFF) == PT_PWHT)
			{
				parts[pmap[y][x]>>8].ctype = pwht_property;
				parts[pmap[y][x]>>8].tmp2 = format;
				parts[pmap[y][x]>>8].tmp = valuei;
			}
		}
		else if (format == 1)
		{
			prop_value = malloc(sizeof(unsigned char));
			memcpy(prop_value,&valuec,sizeof(unsigned char));
			if (x >= 0 && y >= 0 && x < XRES && y < YRES && (pmap[y][x]&0xFF) == PT_PWHT)
			{
				parts[pmap[y][x]>>8].ctype = pwht_property;
				parts[pmap[y][x]>>8].tmp2 = format;
				parts[pmap[y][x]>>8].tmp = valuei;
			}
		}
		else if (format == 2)
		{
			prop_value = malloc(sizeof(float));
			memcpy(prop_value,&valuef,sizeof(float));
			if (x >= 0 && y >= 0 && x < XRES && y < YRES && (pmap[y][x]&0xFF) == PT_PWHT)
			{
				parts[pmap[y][x]>>8].ctype = pwht_property;
				parts[pmap[y][x]>>8].tmp2 = format;
				parts[pmap[y][x]>>8].temp = valuef;
			}
		}
		else if (format == 3)
		{
			prop_format = 0;
			prop_value = malloc(sizeof(unsigned int));
			memcpy(prop_value,&valueui,sizeof(unsigned int));
			if (x >= 0 && y >= 0 && x < XRES && y < YRES && (pmap[y][x]&0xFF) == PT_PWHT)
			{
				parts[pmap[y][x]>>8].ctype = pwht_property;
				parts[pmap[y][x]>>8].tmp2 = 3;
				parts[pmap[y][x]>>8].dcolour = valueui;
			}
		}
	}
exit:
	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
}

void info_ui(pixel *vid_buf, char *top, char *txt)
{
	int x0=(XRES-240)/2,y0=(YRES-MENUSIZE)/2,b=1,bq,mx,my;

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		clearrect(vid_buf, x0-2, y0-2, 244, 64);
		drawrect(vid_buf, x0, y0, 240, 60, 192, 192, 192, 255);
		drawtext(vid_buf, x0+8, y0+8, top, 160, 160, 255, 255);
		drawtext(vid_buf, x0+8, y0+26, txt, 255, 255, 255, 255);
		drawtext(vid_buf, x0+5, y0+49, "OK", 255, 255, 255, 255);
		drawrect(vid_buf, x0, y0+44, 240, 16, 192, 192, 192, 255);
#ifdef OGLR
		clearScreen(1.0f);
#endif
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		if (b && !bq && mx>=x0 && mx<x0+240 && my>=y0+44 && my<=y0+60)
			break;

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
			break;
	}

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
}

void info_box(pixel *vid_buf, char *msg)
{
	int w = textwidth(msg)+16;
	int x0=(XRES-w)/2,y0=(YRES-24)/2;
	
	clearrect(vid_buf, x0-2, y0-2, w+4, 28);
	drawrect(vid_buf, x0, y0, w, 24, 192, 192, 192, 255);
	drawtext(vid_buf, x0+8, y0+8, msg, 192, 192, 240, 255);
#ifndef RENDERER
#ifdef OGLR
	clearScreen(1.0f);
#endif
	sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
#endif
}

void info_box_overlay(pixel *vid_buf, char *msg)
{
	int w = textwidth(msg)+16;
	int x0=(XRES-w)/2,y0=(YRES-24)/2;
	
	clearrect(vid_buf, x0-2, y0-2, w+4, 28);
	drawrect(vid_buf, x0, y0, w, 24, 192, 192, 192, 255);
	drawtext(vid_buf, x0+8, y0+8, msg, 192, 192, 240, 255);
}

void copytext_ui(pixel *vid_buf, char *top, char *txt, char *copytxt)
{
	int state = 0;
	int i;
	int g = 255;
	int xsize = 244;
	int ysize = 90;
	int x0=(XRES-xsize)/2,y0=(YRES-MENUSIZE-ysize)/2,b=1,bq,mx,my;
	int buttonx = 0;
	int buttony = 0;
	int buttonwidth = 0;
	int buttonheight = 0;
	ui_copytext ed;

	buttonwidth = textwidth(copytxt)+12;
	buttonheight = 10+8;
	buttony = y0+50;
	buttonx = x0+(xsize/2)-(buttonwidth/2);

	ed.x = buttonx;
	ed.y = buttony;
	ed.width = buttonwidth;
	ed.height = buttonheight;
	ed.hover = 0;
	ed.state = 0;
	strcpy(ed.text, copytxt);

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		clearrect(vid_buf, x0-2, y0-2, xsize+4, ysize+4);
		drawrect(vid_buf, x0, y0, xsize, ysize, 192, 192, 192, 255);
		drawtext(vid_buf, x0+8, y0+8, top, 160, 160, 255, 255);
		drawtext(vid_buf, x0+8, y0+26, txt, 255, 255, 255, 255);

		ui_copytext_draw(vid_buf, &ed);
		ui_copytext_process(mx, my, b, bq, &ed);

		drawtext(vid_buf, x0+5, y0+ysize-11, "OK", 255, 255, 255, 255);
		drawrect(vid_buf, x0, y0+ysize-16, xsize, 16, 192, 192, 192, 255);

#ifdef OGLR
		clearScreen(1.0f);
#endif
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		if (b && !bq && mx>=x0 && mx<x0+xsize && my>=y0+ysize-16 && my<=y0+ysize)
			break;

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
			break;
	}

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
}
int confirm_ui(pixel *vid_buf, char *top, char *msg, char *btn)
{
	int x0=(XRES-240)/2,y0=YRES/2,b=1,bq,mx,my,textheight;
	int ret = 0;

	textheight = textwrapheight(msg, 224);
	y0 -= (52+textheight)/2;
	if (y0<2)
		y0 = 2;
	if (y0+50+textheight>YRES)
		textheight = YRES-50-y0;

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		clearrect(vid_buf, x0-2, y0-2, 244, 52+textheight);
		drawrect(vid_buf, x0, y0, 240, 48+textheight, 192, 192, 192, 255);
		drawtext(vid_buf, x0+8, y0+8, top, 255, 216, 32, 255);
		drawtextwrap(vid_buf, x0+8, y0+26, 224, 0, msg, 255, 255, 255, 255);
		drawtext(vid_buf, x0+5, y0+textheight+37, "Cancel", 255, 255, 255, 255);
		drawtext(vid_buf, x0+165, y0+textheight+37, btn, 255, 216, 32, 255);
		drawrect(vid_buf, x0, y0+textheight+32, 160, 16, 192, 192, 192, 255);
		drawrect(vid_buf, x0+160, y0+textheight+32, 80, 16, 192, 192, 192, 255);
		
#ifdef OGLR
		clearScreen(1.0f);
#endif
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		if (b && !bq && mx>=x0+160 && mx<x0+240 && my>=y0+textheight+32 && my<=y0+textheight+48)
		{
			ret = 1;
			break;
		}
		
		if (b && !bq && mx>=x0 && mx<x0+160 && my>=y0+textheight+32 && my<=y0+textheight+48)
			break;

		if (sdl_key==SDLK_RETURN)
		{
			ret = 1;
			break;
		}
		if (sdl_key==SDLK_ESCAPE)
			break;
	}

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
	
	return ret;
}

void login_ui(pixel *vid_buf)
{
	int x0=(XRES+BARSIZE-192)/2,y0=(YRES+MENUSIZE-80)/2,b=1,bq,mx,my,err;
	ui_edit ed1,ed2;
	char *res;
	char passwordHash[33];
	char totalHash[33];
	char hashStream[99]; //not really a stream ...

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	ui_edit_init(&ed1, x0+25, y0+25, 158, 14);
	ed1.def = "[user name]";
	ed1.cursor = ed1.cursorstart = strlen(svf_user);
	strcpy(ed1.str, svf_user);
	if (ed1.cursor)
		ed1.focus = 0;

	ui_edit_init(&ed2, x0+25, y0+45, 158, 14);
	ed2.def = "[password]";
	ed2.hide = 1;
	if (!ed1.cursor)
		ed2.focus = 0;

	fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		drawrect(vid_buf, x0, y0, 192, 80, 192, 192, 192, 255);
		clearrect(vid_buf, x0, y0, 192, 80);
		drawtext(vid_buf, x0+8, y0+8, "Server login:", 255, 255, 255, 255);
		drawtext(vid_buf, x0+12, y0+23, "\x8B", 32, 64, 128, 255);
		drawtext(vid_buf, x0+12, y0+23, "\x8A", 255, 255, 255, 255);
		drawrect(vid_buf, x0+8, y0+20, 176, 16, 192, 192, 192, 255);
		drawtext(vid_buf, x0+11, y0+44, "\x8C", 160, 144, 32, 255);
		drawtext(vid_buf, x0+11, y0+44, "\x84", 255, 255, 255, 255);
		drawrect(vid_buf, x0+8, y0+40, 176, 16, 192, 192, 192, 255);
		ui_edit_draw(vid_buf, &ed1);
		ui_edit_draw(vid_buf, &ed2);
		drawtext(vid_buf, x0+5, y0+69, "Sign out", 255, 255, 255, 255);
		drawtext(vid_buf, x0+187-textwidth("Sign in"), y0+69, "Sign in", 255, 255, 55, 255);
		drawrect(vid_buf, x0, y0+64, 192, 16, 192, 192, 192, 255);
		drawrect(vid_buf, x0, y0+64, 96, 16, 192, 192, 192, 255);
		
#ifdef OGLR
		clearScreen(1.0f);
#endif
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		ui_edit_process(mx, my, b, bq, &ed1);
		ui_edit_process(mx, my, b, bq, &ed2);

		if (b && !bq && mx>=x0+9 && mx<x0+23 && my>=y0+22 && my<y0+36)
			break;
		if (b && !bq && mx>=x0+9 && mx<x0+23 && my>=y0+42 && my<y0+46)
			break;
		if (b && !bq && mx>=x0 && mx<x0+96 && my>=y0+64 && my<=y0+80)
			goto fail;
		if (b && !bq && mx>=x0+97 && mx<x0+192 && my>=y0+64 && my<=y0+80)
			break;

		if (sdl_key==SDLK_RETURN || sdl_key==SDLK_TAB)
		{
			if (!ed1.focus)
				break;
			ed1.focus = 0;
			ed2.focus = 1;
		}
		if (sdl_key==SDLK_ESCAPE)
		{
			if (!ed1.focus && !ed2.focus)
				return;
			ed1.focus = 0;
			ed2.focus = 0;
		}
	}

	strcpy(svf_user, ed1.str);
	strcpy(svf_pass, ed2.str);
	//md5_ascii(svf_pass, (unsigned char *)ed2.str, 0);

	md5_ascii(passwordHash, (unsigned char *)svf_pass, strlen(svf_pass));
	passwordHash[32] = 0;
	sprintf(hashStream, "%s-%s", svf_user, passwordHash);
	//hashStream << username << "-" << passwordHash;
	md5_ascii(totalHash, (const unsigned char *)hashStream, strlen(hashStream));
	totalHash[32] = 0;
	if (totalHash)
	{
		int dataStatus, dataLength;
		char * postNames[] = { "Username", "Hash", NULL };
		char * postDatas[] = { svf_user, totalHash };
		int postLengths[] = { strlen(svf_user), 32 };
		char * data;
		data = http_multipart_post(
				  "http://" SERVER "/Login.json",
				  postNames, postDatas, postLengths,
				  NULL, NULL, NULL,
				  &dataStatus, &dataLength);
		if(dataStatus == 200 && data)
		{
			cJSON *root, *tmpobj;//, *notificationarray, *notificationobj;
			int i = 0;
			if (root = cJSON_Parse((const char*)data))
			{
				tmpobj = cJSON_GetObjectItem(root, "Status");
				if (tmpobj && tmpobj->type == cJSON_Number && tmpobj->valueint == 1)
				{
					if((tmpobj = cJSON_GetObjectItem(root, "UserID")) && tmpobj->type == cJSON_Number)
						sprintf(svf_user_id, "%i", tmpobj->valueint);
					if((tmpobj = cJSON_GetObjectItem(root, "SessionID")) && tmpobj->type == cJSON_String)
						strcpy(svf_session_id, tmpobj->valuestring);
					if((tmpobj = cJSON_GetObjectItem(root, "SessionKey")) && tmpobj->type == cJSON_String)
						strcpy(svf_session_key, tmpobj->valuestring);
					if((tmpobj = cJSON_GetObjectItem(root, "Elevation")) && tmpobj->type == cJSON_String)
					{
						char * elevation = tmpobj->valuestring;
						if (!strcmp(elevation, "Mod"))
						{
							svf_admin = 0;
							svf_mod = 1;
						}
						else if (!strcmp(elevation, "Admin"))
						{
							svf_admin = 1;
							svf_mod = 0;
						}
						else
						{
							svf_admin = 0;
							svf_mod = 0;
						}
					}
					/*notificationarray = cJSON_GetObjectItem(root, "Notifications");
					notificationobj = cJSON_GetArrayItem(notificationarray, 0);
					while (notificationobj)
					{
						i++;
						if((tmpobj = cJSON_GetObjectItem(notificationarray, "Text")) && tmpobj->type == cJSON_String)
							if (strstr(tmpobj->valuestring, "message"))
								svf_messages++;
						notificationobj = cJSON_GetArrayItem(notificationarray, i);
					}*/

					svf_login = 1;
					save_presets(0);
				}
				else
				{
					tmpobj = cJSON_GetObjectItem(root, "Error");
					if (tmpobj && tmpobj->type == cJSON_String)
					{
						char * banstring = strstr(tmpobj->valuestring, ". Ban expire in");
						if (banstring) //TODO: temporary, remove this when the ban message is fixed
						{
							char banreason[256] = "Account banned. Login at http://powdertoy.co.uk in order to see the full ban reason. Ban expires";
							strappend(banreason, strstr(tmpobj->valuestring, " in"));
							error_ui(vid_buf, 0, banreason);
						}
						else
							error_ui(vid_buf, 0, tmpobj->valuestring);
					}
					else
						error_ui(vid_buf, 0, "Could not read Error response");
					if (data)
						free(data);
					goto fail;
				}
			}
			else
				error_ui(vid_buf, 0, "Could not read response");
		}
		else
			error_ui(vid_buf, dataStatus, http_ret_text(dataStatus));
		if (data)
			free(data);
		return;
	}

fail:
	strcpy(svf_user, "");
	strcpy(svf_pass, "");
	strcpy(svf_user_id, "");
	strcpy(svf_session_id, "");
	svf_login = 0;
	svf_own = 0;
	svf_admin = 0;
	svf_mod = 0;
	svf_messages = 0;
	save_presets(0);
}

int stamp_ui(pixel *vid_buf, int *reorder)
{
	int b=1,bq,mx,my,d=-1,i,j,k,x,gx,gy,y,w,h,r=-1,stamp_page=0,per_page=GRID_X*GRID_Y,page_count,numdelete=0,lastdelete;
	char page_info[64];
	// stamp_count-1 to avoid an extra page when there are per_page stamps on each page
	page_count = (stamp_count-1)/per_page+1;

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		clearrect(vid_buf, -1, -1, XRES+BARSIZE+1, YRES+MENUSIZE+1);
		k = stamp_page*per_page;//0;
		r = -1;
		d = -1;
		for (j=0; j<GRID_Y; j++)
			for (i=0; i<GRID_X; i++)
			{
				if (stamps[k].name[0])
				{
					gx = ((XRES/GRID_X)*i) + (XRES/GRID_X-XRES/GRID_S)/2;
					gy = ((((YRES-MENUSIZE+20)+15)/GRID_Y)*j) + ((YRES-MENUSIZE+20)/GRID_Y-(YRES-MENUSIZE+20)/GRID_S+10)/2 + 18;
					x = (XRES*i)/GRID_X + XRES/(GRID_X*2);
					y = (YRES*j)/GRID_Y + YRES/(GRID_Y*2);
					gy -= 20;
					w = stamps[k].thumb_w;
					h = stamps[k].thumb_h;
					x -= w/2;
					y -= h/2;
					if (stamps[k].thumb)
					{
						draw_image(vid_buf, stamps[k].thumb, gx+(((XRES/GRID_S)/2)-(w/2)), gy+(((YRES/GRID_S)/2)-(h/2)), w, h, 255);
						xor_rect(vid_buf, gx+(((XRES/GRID_S)/2)-(w/2)), gy+(((YRES/GRID_S)/2)-(h/2)), w, h);
					}
					else
					{
						drawtext(vid_buf, gx+8, gy+((YRES/GRID_S)/2)-4, "Error loading stamp", 255, 255, 255, 255);
					}
					if ((mx>=gx+XRES/GRID_S-4 && mx<(gx+XRES/GRID_S)+6 && my>=gy-6 && my<gy+4) || stamps[k].dodelete)
					{
						if (mx>=gx+XRES/GRID_S-4 && mx<(gx+XRES/GRID_S)+6 && my>=gy-6 && my<gy+4)
							d = k;
						drawrect(vid_buf, gx-2, gy-2, XRES/GRID_S+3, YRES/GRID_S+3, 128, 128, 128, 255);
						drawtext(vid_buf, gx+XRES/GRID_S-4, gy-6, "\x86", 255, 48, 32, 255);
					}
					else
					{
						if (mx>=gx && mx<gx+(XRES/GRID_S) && my>=gy && my<gy+(YRES/GRID_S) && stamps[k].thumb)
						{
							r = k;
							drawrect(vid_buf, gx-2, gy-2, XRES/GRID_S+3, YRES/GRID_S+3, 128, 128, 210, 255);
						}
						else
						{
							drawrect(vid_buf, gx-2, gy-2, XRES/GRID_S+3, YRES/GRID_S+3, 128, 128, 128, 255);
						}
						drawtext(vid_buf, gx+XRES/GRID_S-4, gy-6, "\x86", 150, 48, 32, 255);
					}
					drawtext(vid_buf, gx+XRES/(GRID_S*2)-textwidth(stamps[k].name)/2, gy+YRES/GRID_S+7, stamps[k].name, 192, 192, 192, 255);
					drawtext(vid_buf, gx+XRES/GRID_S-4, gy-6, "\x85", 255, 255, 255, 255);
				}
				k++;
			}

		if (numdelete)
		{
			drawrect(vid_buf,(XRES/2)-19,YRES+MENUSIZE-18,37,16,255,255,255,255);
			drawtext(vid_buf, (XRES/2)-14, YRES+MENUSIZE-14, "Delete", 255, 255, 255, 255);
			if (b == 1 && bq == 0 && mx > (XRES/2)-20 && mx < (XRES/2)+19 && my > YRES+MENUSIZE-19 && my < YRES+MENUSIZE-1)
			{
				sprintf(page_info, "%d stamp%s", numdelete, (numdelete == 1)?"":"s");
				if (confirm_ui(vid_buf, "Do you want to delete?", page_info, "Delete"))
					del_stamp(lastdelete);
				for (i=0; i<STAMP_MAX; i++)
					stamps[i].dodelete = 0;
				numdelete = 0;
				d = lastdelete = -1;
			}
		}
		else
		{
			sprintf(page_info, "Page %d of %d", stamp_page+1, page_count);

			drawtext(vid_buf, (XRES/2)-(textwidth(page_info)/2), YRES+MENUSIZE-14, page_info, 255, 255, 255, 255);
		}

		if (stamp_page)
		{
			drawtext(vid_buf, 4, YRES+MENUSIZE-14, "\x96", 255, 255, 255, 255);
			drawrect(vid_buf, 1, YRES+MENUSIZE-18, 16, 16, 255, 255, 255, 255);
		}
		if (stamp_page<page_count-1)
		{
			drawtext(vid_buf, XRES-15, YRES+MENUSIZE-14, "\x95", 255, 255, 255, 255);
			drawrect(vid_buf, XRES-18, YRES+MENUSIZE-18, 16, 16, 255, 255, 255, 255);
		}

		if (b==1&&bq==0&&d!=-1)
		{
			if (sdl_mod & KMOD_CTRL)
			{
				if (!stamps[d].dodelete)
				{
					stamps[d].dodelete = 1;
					numdelete++;
					lastdelete=d;
				}
				else
				{
					stamps[d].dodelete = 0;
					numdelete--;
				}
			}
			else if (!numdelete && confirm_ui(vid_buf, "Do you want to delete?", stamps[d].name, "Delete"))
			{
				del_stamp(d);
			}
		}

#ifdef OGLR
		clearScreen(1.0f);
#endif
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		if (b==1&&r!=-1)
			break;
		if (b==4&&r!=-1)
		{
			r = -1;
			break;
		}

		if ((b && !bq && mx>=1 && mx<=17 && my>=YRES+MENUSIZE-18 && my<YRES+MENUSIZE-2) || sdl_wheel>0)
		{
			if (stamp_page)
			{
				stamp_page --;
			}
			sdl_wheel = 0;
		}
		if ((b && !bq && mx>=XRES-18 && mx<=XRES-1 && my>=YRES+MENUSIZE-18 && my<YRES+MENUSIZE-2) || sdl_wheel<0)
		{
			if (stamp_page<page_count-1)
			{
				stamp_page ++;
			}
			sdl_wheel = 0;
		}

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
		{
			r = -1;
			break;
		}
	}
	if (sdl_mod & KMOD_CTRL)
		*reorder = 0;

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	for (i=0; i<STAMP_MAX; i++)
		stamps[i].dodelete = 0;
	return r;
}

void tag_list_ui(pixel *vid_buf)
{
	int y,d,x0=(XRES-192)/2,y0=(YRES-256)/2,b=1,bq,mx,my,vp,vn;
	char *p,*q,s;
	char *tag=NULL, *op=NULL;
	struct strlist *vote=NULL,*down=NULL;

	ui_edit ed;
	ui_edit_init(&ed, x0+25, y0+221, 158, 14);
	ed.def = "[new tag]";
	ed.focus = 0;

	fillrect(vid_buf, -1, -1, XRES, YRES+MENUSIZE, 0, 0, 0, 192);
	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		op = tag = NULL;

		drawrect(vid_buf, x0, y0, 192, 256, 192, 192, 192, 255);
		clearrect(vid_buf, x0, y0, 192, 256);
		drawtext(vid_buf, x0+8, y0+8, "Manage tags:    \bgTags are only to \nbe used to improve search results", 255, 255, 255, 255);
		p = svf_tags;
		s = svf_tags[0] ? ' ' : 0;
		y = 36 + y0;
		while (s)
		{
			q = strchr(p, ' ');
			if (!q)
				q = p+strlen(p);
			s = *q;
			*q = 0;
			if (svf_own || svf_admin || svf_mod)
			{
				drawtext(vid_buf, x0+20, y-1, "\x86", 160, 48, 32, 255);
				drawtext(vid_buf, x0+20, y-1, "\x85", 255, 255, 255, 255);
				d = 14;
				if (b && !bq && mx>=x0+18 && mx<x0+32 && my>=y-2 && my<y+12)
				{
					op = "delete";
					tag = mystrdup(p);
				}
			}
			else
				d = 0;
			if (svf_login)
			{
				vp = strlist_find(&vote, p);
				vn = strlist_find(&down, p);
				if ((!vp && !vn && !svf_own) || svf_admin || svf_mod)
				{
					drawtext(vid_buf, x0+d+20, y-1, "\x88", 32, 144, 32, 255);
					drawtext(vid_buf, x0+d+20, y-1, "\x87", 255, 255, 255, 255);
					if (b && !bq && mx>=x0+d+18 && mx<x0+d+32 && my>=y-2 && my<y+12)
					{
						op = "vote";
						tag = mystrdup(p);
						strlist_add(&vote, p);
					}
					drawtext(vid_buf, x0+d+34, y-1, "\x88", 144, 48, 32, 255);
					drawtext(vid_buf, x0+d+34, y-1, "\xA2", 255, 255, 255, 255);
					if (b && !bq && mx>=x0+d+32 && mx<x0+d+46 && my>=y-2 && my<y+12)
					{
						op = "down";
						tag = mystrdup(p);
						strlist_add(&down, p);
					}
				}
				if (vp)
					drawtext(vid_buf, x0+d+48+textwidth(p), y, " - voted!", 48, 192, 48, 255);
				if (vn)
					drawtext(vid_buf, x0+d+48+textwidth(p), y, " - voted.", 192, 64, 32, 255);
			}
			drawtext(vid_buf, x0+d+48, y, p, 192, 192, 192, 255);
			*q = s;
			p = q+1;
			y += 16;
		}
		
		drawtext(vid_buf, x0+11, y0+219, "\x86", 32, 144, 32, 255);
		drawtext(vid_buf, x0+11, y0+219, "\x89", 255, 255, 255, 255);
		drawrect(vid_buf, x0+8, y0+216, 176, 16, 192, 192, 192, 255);
		ui_edit_draw(vid_buf, &ed);
		drawtext(vid_buf, x0+5, y0+245, "Close", 255, 255, 255, 255);
		drawrect(vid_buf, x0, y0+240, 192, 16, 192, 192, 192, 255);
#ifdef OGLR
		clearScreen(1.0f);
#endif
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		ui_edit_process(mx, my, b, bq, &ed);

		if (b && mx>=x0 && mx<=x0+192 && my>=y0+240 && my<y0+256)
			break;

		if (op)
		{
			d = execute_tagop(vid_buf, op, tag);
			free(tag);
			op = tag = NULL;
			if (d)
				goto finish;
		}

		if (b && !bq && mx>=x0+9 && mx<x0+23 && my>=y0+218 && my<y0+232)
		{
			d = execute_tagop(vid_buf, "add", ed.str);
			strcpy(ed.str, "");
			ed.cursor = ed.cursorstart = 0;
			if (d)
				goto finish;
		}

		if (sdl_key == SDLK_RETURN)
		{
			if (svf_login)
			{
				if (!ed.focus)
					break;
				d = execute_tagop(vid_buf, "add", ed.str);
				strcpy(ed.str, "");
				ed.cursor = ed.cursorstart = 0;
				if (d)
					goto finish;
			}
			else
			{
				error_ui(vid_buf, 0, "Not Authenticated");
				goto finish;
			}
		}
		if (sdl_key == SDLK_ESCAPE)
		{
			if (!ed.focus)
				break;
			strcpy(ed.str, "");
			ed.cursor = ed.cursorstart = 0;
			ed.focus = 0;
		}
	}
	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
	sdl_key = 0;

finish:
	strlist_free(&vote);
}

int save_name_ui(pixel *vid_buf)
{
	int x0=(XRES-420)/2,y0=(YRES-78-YRES/4)/2,b=1,bq,mx,my,ths,idtxtwidth,nd=0;
	int can_publish = 1;
	void *th;
	pixel *old_vid=(pixel *)calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	ui_edit ed;
	ui_edit ed2;
	ui_checkbox cbPublish;
	ui_checkbox cbMod;
	ui_checkbox cbPaused;
	ui_copytext ctb;

	th = build_thumb(&ths, 0);
	if (check_save(2,0,0,XRES,YRES,0))
		can_publish = 0;

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	ui_edit_init(&ed, x0+25, y0+25, 158, 14);
	ed.def = "[simulation name]";
	ed.cursor = ed.cursorstart = strlen(svf_name);
	strcpy(ed.str, svf_name);

	ui_edit_init(&ed2, x0+13, y0+45, 170, 85);
	ed2.def = "[simulation description]";
	ed2.focus = 0;
	ed2.cursor = ed2.cursorstart = strlen(svf_description);
	ed2.multiline = 1;
	strcpy(ed2.str, svf_description);

	ctb.x = 0;
	ctb.y = YRES+MENUSIZE-20;
	ctb.width = textwidth(svf_id)+12;
	ctb.height = 10+7;
	ctb.hover = 0;
	ctb.state = 0;
	strcpy(ctb.text, svf_id);

	cbPublish.x = x0+10;
	cbPublish.y = y0+53+YRES/4;
	cbPublish.focus = 0;
	cbPublish.checked = (svf_publish && can_publish);

	cbPaused.x = x0+10;
	cbPaused.y = y0+73+YRES/4;
	cbPaused.focus = 0;
	cbPaused.checked = sys_pause || framerender;

	cbMod.x = x0+110;
	cbMod.y = y0+53+YRES/4;
	cbMod.focus = 0;
	cbMod.checked = !can_publish;
	
	fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
	draw_rgba_image(vid_buf, (unsigned char*)save_to_server_image, 0, 0, 0.7f);
	
	memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);

	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		drawrect(vid_buf, x0, y0, 420, 110+YRES/4, 192, 192, 192, 255); // rectangle around entire thing
		clearrect(vid_buf, x0, y0, 420, 110+YRES/4);
		drawtext(vid_buf, x0+8, y0+8, "New simulation name:", 255, 255, 255, 255);
		drawtext(vid_buf, x0+10, y0+23, "\x82", 192, 192, 192, 255);
		drawrect(vid_buf, x0+8, y0+20, 176, 16, 192, 192, 192, 255); //rectangle around title box

		drawrect(vid_buf, x0+8, y0+40, 176, 100, 192, 192, 192, 255); //rectangle around description box

		ui_edit_draw(vid_buf, &ed);
		ui_edit_draw(vid_buf, &ed2);

		drawrect(vid_buf, x0+(205-XRES/3)/2-2+205, y0+40, XRES/3+3, YRES/3+3, 128, 128, 128, 255); //rectangle around thumbnail
		render_thumb(th, ths, 0, vid_buf, x0+(205-XRES/3)/2+205, y0+42, 3);

		if (can_publish)
		{
			ui_checkbox_draw(vid_buf, &cbPublish);
			drawtext(vid_buf, x0+34, y0+56+YRES/4, "Publish?", 192, 192, 192, 255);

			ui_checkbox_draw(vid_buf, &cbMod);
			drawtext(vid_buf, x0+134, y0+56+YRES/4, "Mod save?", 192, 192, 192, 255);
		}
		else
		{
			drawtext(vid_buf, x0+10, y0+53+YRES/4, "\xE4", 255, 255, 0, 255);
			drawtext(vid_buf, x0+26, y0+55+YRES/4, "Uses mod elements, can't publish", 192, 192, 192, 255);
		}

		ui_checkbox_draw(vid_buf, &cbPaused);
		drawtext(vid_buf, x0+34, y0+76+YRES/4, "Paused?", 192, 192, 192, 255);

		drawtext(vid_buf, x0+5, y0+99+YRES/4, "Save simulation", 255, 255, 255, 255);
		drawrect(vid_buf, x0, y0+94+YRES/4, 192, 16, 192, 192, 192, 255);

		draw_line(vid_buf, x0+192, y0, x0+192, y0+110+YRES/4, 150, 150, 150, XRES+BARSIZE);

		if (svf_id[0])
		{
			//Save ID text and copybox
			idtxtwidth = textwidth("Current save ID: ");
			idtxtwidth += ctb.width;
			ctb.x = textwidth("Current save ID: ")+(XRES+BARSIZE-idtxtwidth)/2;
			drawtext(vid_buf, (XRES+BARSIZE-idtxtwidth)/2, YRES+MENUSIZE-15, "Current save ID: ", 255, 255, 255, 255);

			ui_copytext_draw(vid_buf, &ctb);
			ui_copytext_process(mx, my, b, bq, &ctb);
		}
#ifdef OGLR
		clearScreen(1.0f);
#endif
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		memcpy(vid_buf, old_vid, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);

		ui_edit_process(mx, my, b, bq, &ed);
		ui_edit_process(mx, my, b, bq, &ed2);
		if (can_publish)
		{
			ui_checkbox_process(mx, my, b, bq, &cbPublish);
			ui_checkbox_process(mx, my, b, bq, &cbMod);
		}
		ui_checkbox_process(mx, my, b, bq, &cbPaused);
		if (cbMod.checked)
			cbPublish.checked = 0;

		if ((b && !bq && ((mx>=x0+9 && mx<x0+23 && my>=y0+22 && my<y0+36) ||
		                 (mx>=x0 && mx<x0+192 && my>=y0+94+YRES/4 && my<y0+110+YRES/4)))
			|| sdl_key==SDLK_RETURN)
		{
			if (th) free(th);
			if (!ed.str[0])
				return 0;
			nd = strcmp(svf_name, ed.str) || !svf_own;
			strncpy(svf_name, ed.str, 63);
			svf_name[63] = 0;
			strncpy(svf_description, ed2.str, 254);
			strncpy(svf_author, svf_user, 63);
			svf_description[254] = 0;
			if (nd)
			{
				strcpy(svf_id, "");
				strcpy(svf_tags, "");
			}
			svf_open = 1;
			svf_own = 1;
			svf_publish = cbPublish.checked;
			svf_modsave = cbMod.checked;
			sys_pause = cbPaused.checked;
			framerender = 0;
			svf_filename[0] = 0;
			svf_fileopen = 0;
			free(old_vid);
			return nd+1;
		}
		if (sdl_key==SDLK_ESCAPE)
		{
			if (!ed.focus)
				break;
			ed.focus = 0;
		}
	}
	if (th) free(th);
	free(old_vid);
	return 0;
}

//calls menu_ui_v2 when needed, draws fav. menu on bottom
void old_menu_v2(int active_menu, int x, int y, int b, int bq)
{
	if (active_menu > SC_FAV) {
		Tool *over = menu_draw(x, y, b, bq, active_menu);
		if (over)
			lastOver = over;
		menu_draw_text(lastOver, active_menu);
		menu_select_element(b, over);
	} else {
		Tool *over = menu_draw(x, y, b, bq, SC_FAV);
		if (over)
			lastOver = over;
		menu_draw_text(lastOver, SC_FAV);
		menu_select_element(b, over);
	}
	int numMenus = GetNumMenus();
	for (int i = 0; i < numMenus; i++)
	{
		if (i < SC_FAV)
			drawchar(vid_buf, XRES+1, /*(12*i)+2*/((YRES/numMenus)*i)+((YRES/numMenus)/2)+5, menuSections[i]->icon, 255, 255, 255, 255);
	}
	//check mouse position to see if it is on a menu section
	for (int i = 0; i < numMenus; i++)
	{
		if (!b && i < SC_FAV && x>= XRES+1 && x< XRES+BARSIZE-1 && y>= ((YRES/numMenus)*i)+((YRES/numMenus)/2)+3 && y<((YRES/numMenus)*i)+((YRES/numMenus)/2)+17)
			menu_ui_v2(vid_buf, i); //draw the elements in the current menu if mouse inside
	}
}

//old menu function, with the elements drawn on the side
void menu_ui_v2(pixel *vid_buf, int i)
{
	int b=1, bq, mx, my, y, sy, numMenus = GetNumMenus(), someStrangeYValue = (((YRES/numMenus)*i)+((YRES/numMenus)/2))+3;
	int rows = (int)ceil((float)menuSections[i]->tools.size()/16.0f);
	int height = (int)(ceil((float)menuSections[i]->tools.size()/16.0f)*18);
	int width = (int)restrict_flt(menuSections[i]->tools.size()*31.0f, 0, 16*31);
	pixel *old_vid=(pixel *)calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	if (!old_vid)
		return;
	fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
	memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);
	active_menu = i;
	sy = y = (((YRES/numMenus)*i)+((YRES/numMenus)/2))-(height/2)+(FONT_H/2)+6;

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
	while (!sdl_poll())
	{
		SEC = SEC2;
		bq = b;
		b = mouse_get_state(&mx, &my);
		fillrect(vid_buf, (XRES-BARSIZE-width)-4, y-5, width+16, height+16+rows, 0, 0, 0, 100);
		drawrect(vid_buf, (XRES-BARSIZE-width)-4, y-5, width+16, height+16+rows, 255, 255, 255, 255);
		fillrect(vid_buf, (XRES-BARSIZE)+14, someStrangeYValue, 15, FONT_H+4, 0, 0, 0, 100);
		drawrect(vid_buf, (XRES-BARSIZE)+13, someStrangeYValue, 16, FONT_H+4, (SEC==i&&SEC!=0)?0:255, 255, 255, 255);
		drawrect(vid_buf, (XRES-BARSIZE)+12, someStrangeYValue+1, 1, FONT_H+2, 0, 0, 0, 255);
		if (i) //if not in walls
			drawtext(vid_buf, 12, 12, "\bgPress 'o' to return to the old menu", 255, 255, 255, 255);
		
		Tool *over = menu_draw(mx, my, b, bq, active_menu);
		if (over)
			lastOver = over;
		if (!bq && mx>=((XRES+BARSIZE)-16) ) //highlight menu section
			if (sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_CTRL))
				if (i > 0 && i < SC_TOTAL)
					SEC = i;
		menu_draw_text(lastOver, active_menu);

		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
		memcpy(vid_buf, old_vid, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);
		if (!(mx>=(XRES-BARSIZE-width)-4 && my>=sy-5 && my<sy+height+14) || (mx >= XRES-BARSIZE && !(my >= someStrangeYValue && my <= someStrangeYValue+FONT_H+4)))
		{
			break;
		}

		menu_select_element(b, over);

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
			break;
	}

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
	free(old_vid);
}

//current menu function
void menu_ui_v3(pixel *vid_buf, int i, int b, int bq, int mx, int my)
{
	SEC = SEC2;

	Tool* over = menu_draw(mx, my, b, bq, i);
	if (over)
		lastOver = over;

	if (!bq && mx>=((XRES+BARSIZE)-16) ) //highlight menu section
		if (sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_CTRL))
			if (i > 0 && i < SC_TOTAL)
				SEC = i;

	menu_draw_text(lastOver, i);
	menu_select_element(b, over);
}

int scrollbar(int fwidth, int mx, int y)
{
	int scrollSize, scrollbarx;
	float overflow, location;
	if (mx > XRES)
		mx = XRES;
	//if (mx < 15) //makes scrolling a little nicer at edges but apparently if you put hundreds of elements in a menu it makes the end not show ...
	//	mx = 15;
	scrollSize = (int)(((float)(XRES-BARSIZE))/((float)fwidth) * ((float)XRES-BARSIZE));
	scrollbarx = (int)(((float)mx/((float)XRES))*(float)(XRES-scrollSize));
	if (scrollSize+scrollbarx>XRES)
		scrollbarx = XRES-scrollSize;
	fillrect(vid_buf, scrollbarx, y+19, scrollSize, 3, 200, 200, 200, 255);

	overflow = (float)fwidth-XRES+10;
	location = ((float)XRES-3)/((float)(mx-(XRES-2)));
	return (int)(overflow / location);
}

Tool* menu_draw(int mx, int my, int b, int bq, int i)
{
	int x, y, n=0, xoff=0, fwidth = menuSections[i]->tools.size()*31;
	Tool* over = NULL;
	if (!old_menu || i >= SC_FAV)
	{
		x = XRES-BARSIZE-18;
		y = YRES+1;
	}
	else
	{
		int i2 = i;
		int height = (int)(ceil((float)menuSections[i]->tools.size()/16.0f)*18);

		if (i == SC_FAV2 || i == SC_HUD)
			i2 = SC_FAV;
		x = XRES-BARSIZE-23;
		y = (((YRES/GetNumMenus())*i2)+((YRES/GetNumMenus())/2))-(height/2)+(FONT_H/2)+11;
	}

	//draw everything in the decoration editor
	if (i == SC_DECO)
	{
		decoration_editor(vid_buf, b, bq, mx, my);

		int presetx = 6;
		for (n = DECO_PRESET_START; n < DECO_PRESET_START+NUM_COLOR_PRESETS; n++)
		{
			if (!bq && mx>=presetx-1 && mx<presetx+27 && my>=y && my< y+15)
			{
				drawrect(vid_buf, presetx-1, y-1, 29, 17, 255, 55, 55, 255);
				std::stringstream identifier;
				identifier << "DEFAULT_DECOUR_COLOUR_" << colorlist[n-DECO_PRESET_START].descs;
				over =  new Tool(DECO_TOOL, n, identifier.str());
			}
			draw_tool_button(vid_buf, presetx, y, colorlist[n-DECO_PRESET_START].colour, "");
			presetx += 31;
		}
	}
	else if (i == SC_HUD)
	{
		fwidth = 0;
		for (n = 0; n < HUD_NUM; n++)
		{
			if (hud_menu[n].menunum == hud_menunum || n == 0)
			{
				fwidth += 31;
			}
		}
	}

	//fancy scrolling
	if ((!old_menu || i >= SC_FAV) && fwidth > XRES-BARSIZE)
	{
		xoff = scrollbar(fwidth, mx, y);
	}

	//main loop, draws the tools and figures out which tool you are hovering over / selecting
	for (std::vector<Tool*>::iterator iter = menuSections[i]->tools.begin(), end = menuSections[i]->tools.end(); iter != end; ++iter)
	{
		Tool* current = *iter;
		//special cases for fake menu things in my mod
		if (i == SC_FAV || i == SC_FAV2 || i == SC_HUD)
		{
			last_fav_menu = i;
			if (i == SC_HUD && hud_menu[current->GetID()-HUD_START].menunum != hud_menunum && current->GetID() != HUD_START)
				continue;
			else if (i == SC_FAV && iter != menuSections[i]->tools.begin())
			{
				current = GetToolFromIdentifier(favMenu[iter-menuSections[i]->tools.begin()-1]);
				if (!current)
					continue;
			}
		}
		//if it's offscreen to the right or left, draw nothing
		if (x-xoff > XRES-26 || x-xoff < 0)
		{
			x -= 31;
			continue;
		}
		if (old_menu && x-26<=60 && i < SC_FAV)
		{
			x = XRES-BARSIZE-23;
			y += 19;
		}
		x -= draw_tool_xy(vid_buf, x-xoff, y, current)+5;

		if (i == SC_HUD) //HUD menu is special and should hopefully be removed someday ...
		{
			if (!bq && mx>=x+32-xoff && mx<x+58-xoff && my>=y && my< y+15)
			{
				drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 255, 55, 55, 255);
				over = current;
			}
			else if (current->GetID() >= HUD_REALSTART && currentHud[current->GetID()-HUD_REALSTART] && !strstr(hud_menu[current->GetID()-HUD_START].name, "#"))
			{
				drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 0, 255, 0, 255);
			}
		}
		else
		{
			//if mouse inside button
			if (!bq && mx>=x+32-xoff && mx<x+58-xoff && my>=y && my<y+15)
			{
				over = current;
				//draw rectangles around hovered on tools
				if (sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_CTRL))
					drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 0, 255, 255, 255);
				else if (sdl_mod & (KMOD_SHIFT) && sdl_mod & (KMOD_CTRL))
					drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 0, 255, 0, 255);
				else
					drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 255, 55, 55, 255);

				if (b && current->GetID() == SPC_PROP) //TODO: compare to identifier once tool identifiers are set up
					prop_edit_ui(vid_buf, -1, -1, 0);
			}
			//draw rectangles around selected tools
			else if (activeTools[0]->GetIdentifier() == current->GetIdentifier())
				drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 255, 55, 55, 255);
			else if (activeTools[1]->GetIdentifier() == current->GetIdentifier())
				drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 55, 55, 255, 255);
			else if (activeTools[2]->GetIdentifier() == current->GetIdentifier())
				drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 0, 255, 255, 255);
			else if (i == SC_FAV && menuSections[i]->tools.end()-iter <= locked)
				fillrect(vid_buf, x+31-xoff, y, 27, 15, 0, 0, 255, 127);
		}
	}

	if (over && dae < 51)
		dae = dae + 2;
	else if (dae)
		dae = dae - 2; //Fade away element descriptions
	if (dae > 51)
		dae = 51;
	if (dae < 0)
		dae = 0;
	return over;
}

void menu_draw_text(Tool* lastOver, int i)
{
	int sy = YRES+1, toolID;
	if (lastOver)
		toolID = lastOver->GetID();
	if (old_menu && i < SC_FAV)
	{
		int height = (int)(ceil((float)menuSections[i]->tools.size()/16.0f)*18);
		sy = (((YRES/GetNumMenus())*i)+((YRES/GetNumMenus())/2))-(height/2)+(FONT_H/2)+35+height;
	}
	drawtext(vid_buf, XRES-textwidth(menuSections[i]->name.c_str())-BARSIZE, sy-10, menuSections[i]->name.c_str(), 255, 255, 255, (51-dae)*5);
	if (!lastOver)
	{
		//drawtext(vid_buf, XRES-textwidth(msections[i].name.c_str())-BARSIZE, sy-10, msections[i].name.c_str(), 255, 255, 255, (51-dae)*5);
	}
	else if (lastOver->GetType() == ELEMENT_TOOL)
	{
		drawtext(vid_buf, XRES-textwidth((char *)ptypes[toolID].descs)-BARSIZE, sy-10, (char *)ptypes[toolID].descs, 255, 255, 255, dae*5);
	}
	else if (toolID >= HUD_START && toolID < HUD_START+HUD_NUM)
	{
		if (!strstr(hud_menu[toolID-HUD_START].name,"#"))
			drawtext(vid_buf, XRES-textwidth((char *)hud_menu[toolID-HUD_START].description)-BARSIZE, sy-10, (char *)hud_menu[toolID-HUD_START].description, 255, 255, 255, dae*5);
		else
		{
			char description[512] = "";
			sprintf(description,"%s %i decimal places",hud_menu[toolID-HUD_START].description,currentHud[toolID-HUD_REALSTART]);
			drawtext(vid_buf, XRES-textwidth(description)-BARSIZE, sy-10, description, 255, 255, 255, dae*5);
		}
	}
	else if (toolID >= FAV_START && toolID < FAV_END)
	{
		char favtext[512] = "";
		sprintf(favtext, fav[toolID-FAV_START].description);
		if (toolID == FAV_ROTATE)
		{
			if (ms_rotation)
				strappend(favtext, "on");
			else
				strappend(favtext, "off");
		}
		if (toolID == FAV_HEAT)
		{
			if (!heatmode)
				strappend(favtext, "normal: -273.15C - 9725.85C");
			else if (heatmode == 1)
				sprintf(favtext, "%sautomatic: %iC - %iC",fav[toolID-FAV_START].description,lowesttemp-273,highesttemp-273);
			else
				sprintf(favtext, "%smanual: %iC - %iC",fav[toolID-FAV_START].description,lowesttemp-273,highesttemp-273);
		}
		/*else if (toolID == FAV_SAVE)
		{
			if (save_as%3 == 0)
				strappend(favtext, "Jacob's Mod ver. " MTOS(MOD_VERSION));
			else if (save_as%3 == 1)
				strappend(favtext, "Powder Toy beta ver. " MTOS(BETA_VERSION));
			else
				strappend(favtext, "Powder Toy release ver. " MTOS(RELEASE_VERSION));
		}*/
		else if (toolID == FAV_AUTOSAVE)
		{
			if (!autosave)
				strappend(favtext, "off");
			else
				sprintf(favtext, "%severy %d seconds",fav[toolID-FAV_START].description, autosave);
		}
		else if (toolID == FAV_REAL)
		{
			if (realistic)
				strappend(favtext, "on");
			else
				strappend(favtext, "off");
		}
		else if (toolID == FAV_FIND2)
		{
			if (finding &0x8)
				strappend(favtext, "on");
			else
				strappend(favtext, "off");
		}
		else if (toolID == FAV_DATE)
		{
			char *time;
			converttotime("1300000000",&time, -1, -1, -1);
			strappend(favtext, time);
		}
		drawtext(vid_buf, XRES-textwidth(favtext)-BARSIZE, sy-10, favtext, 255, 255, 255, dae*5);
	}
	else if (lastOver->GetType() == GOL_TOOL)
	{
		drawtext(vid_buf, XRES-textwidth((char *)gmenu[toolID].description)-BARSIZE, sy-10, (char *)gmenu[toolID].description, 255, 255, 255, dae*5);
	}
	else if (toolID >= DECO_PRESET_START && toolID < DECO_PRESET_START + NUM_COLOR_PRESETS)
	{
		drawtext(vid_buf, XRES-textwidth((char *)colorlist[toolID-DECO_PRESET_START].descs)-BARSIZE, sy-10, (char *)colorlist[toolID-DECO_PRESET_START].descs, 255, 255, 255, dae*5);
	}
	else //if (lastOver->GetType() == WALL_TOOL)
	{
		drawtext(vid_buf, XRES-textwidth((char *)wtypes[toolID-UI_WALLSTART].descs)-BARSIZE, sy-10, (char *)wtypes[toolID-UI_WALLSTART].descs, 255, 255, 255, dae*5);
	}
}

void menu_select_element(int b, Tool* over)
{
	//these are click events, b=1 is left click, b=4 is right
	//h has the value of the element it is over, and -1 if not over an element
	int toolID;
	if (over)
		toolID = over->GetID();
	if (b==1 && !over)
	{
		if (sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_CTRL) && SEC>=0)
		{
			activeTools[2] = NULL;
			SEC2 = SEC;
		}
	}
	if (b==1 && over)
	{
		int j;
		if (toolID >= FAV_START && toolID <= FAV_END)
		{
			if (toolID == FAV_MORE)
				active_menu = SC_FAV2;
			else if (toolID == FAV_BACK)
				active_menu = SC_FAV;
			else if (toolID == FAV_FIND)
			{
				if (finding & 0x4)
					finding &= 0x8;
				else if (finding &	0x2)
					finding |= 0x4;
				else if (finding &	0x1)
					finding |= 0x2;
				else
					finding |= 0x1;
			}
			else if (toolID == FAV_INFO)
				drawinfo = !drawinfo;
			else if (toolID == FAV_ROTATE)
				ms_rotation = !ms_rotation;
			else if (toolID == FAV_HEAT)
				heatmode = (heatmode + 1)%3;
			/*else if (toolID == FAV_SAVE)
			{
				save_as = 3+(save_as + 1)%3;
#ifndef BETA
				if (save_as%3 == 1)
					save_as++;
#endif
			}*/
			else if (toolID == FAV_LUA)
#ifdef LUACONSOLE
				addluastuff();
#else
				error_ui(vid_buf, 0, "Lua console not enabled");
#endif
			else if (toolID == FAV_CUSTOMHUD)
				active_menu = SC_HUD;
			else if (toolID == FAV_AUTOSAVE)
			{
				autosave = atoi(input_ui(vid_buf,"Autosave","Input number of seconds between saves, 0 = off","",""));
				if (autosave < 0)
					autosave = 0;
			}
			else if (toolID == FAV_REAL)
			{
				realistic = !realistic;
				if (realistic)
					ptypes[PT_FIRE].hconduct = 1;
				else
					ptypes[PT_FIRE].hconduct = 88;
			}
			else if (toolID == FAV_FIND2)
			{
				if (finding & 0x8)
					finding &= ~0x8;
				else
					finding |= 0x8;
			}
			else if (toolID == FAV_DATE)
			{
				if (dateformat%6 == 5)
					dateformat -= 5;
				else
					dateformat = dateformat + 1;
			}
			else if (toolID == FAV_SECR)
			{
				secret_els = !secret_els;
				menu_count();
			}
		}
		else if (toolID >= HUD_START && toolID < HUD_START+HUD_NUM)
		{
			if (toolID == HUD_START)
			{
				if (hud_menunum != 0)
					hud_menunum = 0;
				else
					active_menu = SC_FAV2;
			}
			else if (toolID == HUD_START + 4)
			{
				HudDefaults();
				SetCurrentHud();
			}
			else if (toolID < HUD_REALSTART)
			{
				hud_menunum = toolID - HUD_START;
			}
			else
			{
				char hud_curr[16];
				sprintf(hud_curr,"%i",currentHud[toolID-HUD_REALSTART]);
				if (strstr(hud_menu[toolID-HUD_START].name,"#"))
					currentHud[toolID-HUD_REALSTART] = atoi(input_ui(vid_buf,(char*)hud_menu[toolID-HUD_START].name,"Enter number of decimal places",hud_curr,""));
				else
					currentHud[toolID-HUD_REALSTART] = !currentHud[toolID-HUD_REALSTART];
				if (DEBUG_MODE)
					memcpy(debugHud,currentHud,sizeof(debugHud));
				else
					memcpy(normalHud,currentHud,sizeof(normalHud));
			}
		}
		else if (toolID >= DECO_PRESET_START && toolID < DECO_PRESET_START+NUM_COLOR_PRESETS)
		{
			int newDecoColor = (255<<24)|colorlist[toolID-DECO_PRESET_START].colour;
			if (newDecoColor != decocolor)
				decocolor = newDecoColor;
			else
			{
				if (activeTools[0]->GetIdentifier() != "DEFAULT_WL_26")
				{
					activeTools[0] = GetToolFromIdentifier("DEFAULT_WL_26");
				}
			}
			currR = PIXR(decocolor), currG = PIXG(decocolor), currB = PIXB(decocolor), currA = decocolor>>24;
			RGB_to_HSV(currR, currG, currB, &currH, &currS, &currV);
		}
		else if (sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_CTRL))
		{
			activeTools[2] = over;
			SEC2 = -1;
		}
		else
		{
			int j, pos = 0, pos2 = 0, last = 17-locked, lock = 0;
			for (j = 0; j < 18; j++)
				if (favMenu[j] == over->GetIdentifier())
					pos = j; // If el is alredy on list, don't put it there twice
			pos2 = pos;
			if (sdl_mod & (KMOD_SHIFT) && sdl_mod & (KMOD_CTRL) && locked < 18 && pos < 18-locked)
			{
				lock = 1;
			}
			if (pos > 18-locked && locked != 18) lock = 1;
			if (lock) last = 17;
			while (pos < last)
			{
				favMenu[pos] = favMenu[pos+1];
				pos = pos + 1;
			}
			if (last != 0)
				favMenu[last] = over->GetIdentifier();
			if (sdl_mod & (KMOD_SHIFT) && sdl_mod & (KMOD_CTRL) && locked < 18 && pos2 <= 18-locked)
			{
				locked = locked + 1;
				save_presets(0);
			}
			else
			{

				activeTools[0] = over;
				activeToolID = 0;
				dae = 51;
			}
		}
	}
	if (b==4 && !over)
	{
		if (sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_CTRL) && SEC>=0)
		{
			activeTools[2] = NULL;
			SEC2 = SEC;
		}
	}
	if (b==4 && over)
	{
		int j;
		if (toolID >= FAV_START && toolID <= FAV_END)
		{
			if (toolID == FAV_HEAT)
			{
				heatmode = 2;
				lowesttemp = atoi(input_ui(vid_buf,"Manual Heat Display","Enter a Minimum Temperature in Celcius","",""))+273;
				highesttemp = atoi(input_ui(vid_buf,"Manual Heat Display","Enter a Maximum Temperature in Celcius","",""))+273;
			}
			else if (toolID == FAV_DATE)
			{
				if (dateformat < 6)
					dateformat += 6;
				else
					dateformat -= 6;
			}
		}
		else if (toolID >= HUD_START && toolID < HUD_START+HUD_NUM)
		{
		}
		else if (sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_CTRL))
		{
			activeTools[2] = over;
			SEC2 = -1;
		}
		else
		{
			int j, pos = 0, last = 17-locked, lock = 0;
			for (j = 0; j < 18; j++)
				if (favMenu[j] == over->GetIdentifier())
					pos = j; // If el is alredy on list, don't put it there twice
			if (pos > 18-locked && locked != 18) lock = 1;
			if (lock) last = 17;
			if (pos > 17-locked && sdl_mod & (KMOD_SHIFT) && sdl_mod & (KMOD_CTRL) && locked > 0 && active_menu == SC_FAV)
			{
				std::string temp = favMenu[pos];
				for (j = pos; j > 18-locked; j--)
					favMenu[j] = favMenu[j-1];
				favMenu[18-locked] = temp;
				locked = locked - 1;
				save_presets(0);
			}
			else
			{
				activeTools[1] = over;
				activeToolID = 1;
				dae = 51;
				while (pos < last)
				{
					favMenu[pos] = favMenu[pos+1];
					pos = pos + 1;
				}
				if (last != 0)
					favMenu[last] = over->GetIdentifier();
			}
		}
	}
}

char tabNames[10][255];
pixel* tabThumbnails[10];
int quickoptionsToolTipFade = 0, quickoptionsThumbnailFade = 0;
int clickedQuickoption = -1, hoverQuickoption = -1;
void quickoptions_menu(pixel *vid_buf, int b, int bq, int x, int y)
{
	int i = 0;
	char isQuickoptionClicked = 0, *quickoptionsToolTip = "";
	int quickoptionsToolTipY = 0;
	//normal quickoptions
	if (!show_tabs && !(sdl_mod & KMOD_CTRL))
	{
		while(quickmenu[i].icon!=NULL)
		{
			if(quickmenu[i].type == QM_TOGGLE)
			{
				drawrect(vid_buf, (XRES+BARSIZE)-16, (i*16)+1, 14, 14, 255, 255, 255, 255);
				if(*(quickmenu[i].variable) || clickedQuickoption == i)
				{
					fillrect(vid_buf, (XRES+BARSIZE)-16, (i*16)+1, 14, 14, 255, 255, 255, 255);
					drawtext(vid_buf, (XRES+BARSIZE)-11, (i*16)+5, quickmenu[i].icon, 0, 0, 0, 255);
				}
				else
				{
					fillrect(vid_buf, (XRES+BARSIZE)-16, (i*16)+1, 14, 14, 0, 0, 0, 255);
					drawtext(vid_buf, (XRES+BARSIZE)-11, (i*16)+5, quickmenu[i].icon, 255, 255, 255, 255);
				}
				if(x >= (XRES+BARSIZE)-16 && x <= (XRES+BARSIZE)-2 && y >= (i*16)+1 && y <= (i*16)+15)
				{
					quickoptionsToolTipFade += 2;
					if(quickoptionsToolTipFade > 12)
						quickoptionsToolTipFade = 12;
					quickoptionsToolTip = (char*)quickmenu[i].name;
					quickoptionsToolTipY = (i*16)+5;

					if (b == 1 && !bq)
					{
						if (clickedQuickoption == -1)
							clickedQuickoption = i;
						isQuickoptionClicked = 1;
					}
					else if (b == 1 || bq == 1)
					{
						if (clickedQuickoption == i)
							isQuickoptionClicked = 1;
					}
				}
			}
			i++;
		}
	}
	//tab menu
	else
	{
		while(i < num_tabs + 2 && i < 25-GetNumMenus())
		{
			char num[8];
			sprintf(num,"%d",i);
			drawrect(vid_buf, (XRES+BARSIZE)-16, (i*16)+1, 14, 14, 255, 255, 255, 255);
			if (i == 0)
			{
				fillrect(vid_buf, (XRES+BARSIZE)-16, (i*16)+1, 14, 14, 255, 255, 255, 255);
				drawtext(vid_buf, (XRES+BARSIZE)-11, (i*16)+5, quickmenu[i].icon, 0, 0, 0, 255);
			}
			else if (i == num_tabs + 1)
			{
				fillrect(vid_buf, (XRES+BARSIZE)-16, (i*16)+1, 14, 14, 0, 0, 0, 255);
				drawtext(vid_buf, (XRES+BARSIZE)-13, (i*16)+3, "\x89", 255, 255, 255, 255);
			}
			else if(tab_num == i)
			{
				fillrect(vid_buf, (XRES+BARSIZE)-16, (i*16)+1, 14, 14, 255, 255, 255, 255);
				drawtext(vid_buf, (XRES+BARSIZE)-11, (i*16)+5, num, 0, 0, 0, 255);
			}
			else
			{
				fillrect(vid_buf, (XRES+BARSIZE)-16, (i*16)+1, 14, 14, 0, 0, 0, 255);
				drawtext(vid_buf, (XRES+BARSIZE)-11, (i*16)+5, num, 255, 255, 255, 255);
			}
			if(x >= (XRES+BARSIZE)-16 && x <= (XRES+BARSIZE)-2 && y >= (i*16)+1 && y <= (i*16)+15)
			{
				quickoptionsToolTipFade += 2;
				if(quickoptionsToolTipFade > 12)
					quickoptionsToolTipFade = 12;

				if (i == 0)
					quickoptionsToolTip = (char*)quickmenu[i].name;
				else if (i == num_tabs + 1)
					quickoptionsToolTip = "Add tab \bg(ctrl+n)";
				else if (tab_num == i)
				{
					if (strlen(svf_name))
						quickoptionsToolTip = svf_name;
					else if (strlen(svf_filename))
						quickoptionsToolTip = svf_filename;
					else
						quickoptionsToolTip = "Untitled Simulation (current)";
				}
				else
					quickoptionsToolTip = tabNames[i-1];
				quickoptionsToolTipY = (i*16)+5;

				if (i > 0 && i <= num_tabs && tab_num != i)
				{
					quickoptionsThumbnailFade += 2;
					if (quickoptionsThumbnailFade > 12)
						quickoptionsThumbnailFade = 12;
					hoverQuickoption = i;
				}
				if (b && !bq)
				{
					if (clickedQuickoption == -1)
						clickedQuickoption = i;
					isQuickoptionClicked = 1;
				}
				else if (b || bq)
				{
					if (clickedQuickoption == i)
						isQuickoptionClicked = 1;
				}
			}
			i++;
		}
	}
	if (!isQuickoptionClicked)
		clickedQuickoption = -1;
	if (clickedQuickoption >= 0 && !b && bq)
	{
		if (!show_tabs && !(sdl_mod & KMOD_CTRL))
		{
			if (bq == 1)
			{
				if (clickedQuickoption == 3)
				{
					if(!ngrav_enable)
						start_grav_async();
					else
						stop_grav_async();
				}
				else
					*(quickmenu[clickedQuickoption].variable) = !(*(quickmenu[clickedQuickoption].variable));
			}
		}
		else
		{
			if (bq == 1)
			{
				//toggle show_tabs
				if (clickedQuickoption == 0)
					*(quickmenu[clickedQuickoption].variable) = !(*(quickmenu[clickedQuickoption].variable));
				//start a new tab
				else if (clickedQuickoption == num_tabs + 1)
				{
					tab_save(tab_num, 0);
					num_tabs++;
					tab_num = num_tabs;
					if (sdl_mod & KMOD_CTRL)
						NewSim();
					tab_save(tab_num, 1);
				}
				//load an different tab
				else if (tab_num != clickedQuickoption)
				{
					tab_save(tab_num, 0);
					tab_load(clickedQuickoption);
					tab_num = clickedQuickoption;
				}
				//click current tab, do nothing
				else
				{
				}
			}
			else if (bq == 4 && clickedQuickoption > 0 && clickedQuickoption <= num_tabs)
			{
				if (num_tabs > 1)
				{
					char name[30], newname[30];
					//delete the tab that was closed, free thumbnail
					sprintf(name, "tabs%s%d.stm", PATH_SEP, clickedQuickoption);
					remove(name);
					free(tabThumbnails[clickedQuickoption-1]);
					tabThumbnails[clickedQuickoption-1] = NULL;

					//rename all the tabs and move other variables around
					for (i = clickedQuickoption; i < num_tabs; i++)
					{
						sprintf(name, "tabs%s%d.stm", PATH_SEP, i+1);
						sprintf(newname, "tabs%s%d.stm", PATH_SEP, i);
						rename(name, newname);
						strncpy(tabNames[i-1], tabNames[i], 254);
						tabThumbnails[i-1] = tabThumbnails[i];
					}

					num_tabs--;
					tabThumbnails[num_tabs] = NULL;
					if (clickedQuickoption == tab_num)
					{
						if (tab_num > 1)
							tab_num --;
						//if you deleted current tab, load the new one
						tab_load(tab_num);
					}
					else if (tab_num > clickedQuickoption && tab_num > 1)
						tab_num--;
				}
				else
				{
					NewSim();
					tab_save(tab_num, 1);
				}
			}
		}
	}

	if (quickoptionsToolTipFade && quickoptionsToolTip)
	{
		//drawtext_outline(vid_buf, (XRES - 5) - textwidth(quickoptionsToolTip), quickoptionsToolTipY, quickoptionsToolTip, 255, 255, 255, quickoptionsToolTipFade*20, 0, 0, 0, quickoptionsToolTipFade*15);
		drawtext(vid_buf, (XRES - 5) - textwidth(quickoptionsToolTip), quickoptionsToolTipY, quickoptionsToolTip, 255, 255, 255, quickoptionsToolTipFade*20);
		quickoptionsToolTipFade--;
	}
	if (quickoptionsThumbnailFade && tabThumbnails[hoverQuickoption-1])
	{
		drawrect(vid_buf, (XRES+BARSIZE)/3-1, YRES/3-1, (XRES+BARSIZE)/3+2, YRES/3+1, 0, 0, 255, quickoptionsThumbnailFade*21);
		draw_image(vid_buf, tabThumbnails[hoverQuickoption-1], (XRES+BARSIZE)/3, YRES/3, (XRES+BARSIZE)/3+1, YRES/3, quickoptionsThumbnailFade*21);
		quickoptionsThumbnailFade--;
	}
}

int FPSwait = 0, fastquit = 0;
int sdl_poll(void)
{
	SDL_Event event;
	sdl_key=sdl_rkey=sdl_wheel=sdl_ascii=0;
	if (has_quit)
		return 1;
	loop_time = SDL_GetTicks();
	if (main_loop > 0)
	{
		main_loop--;
		if (main_loop == 0)
		{
			SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
			FPSwait = 2;
		}
		else
			SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
	}
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_KEYDOWN:
			sdl_key=event.key.keysym.sym;
			sdl_ascii=event.key.keysym.unicode;
			if (event.key.keysym.sym == SDLK_PLUS)
			{
				sdl_wheel++;
			}
			if (event.key.keysym.sym == SDLK_MINUS)
			{
				sdl_wheel--;
			}
			if (event.key.keysym.sym=='q' && (sdl_mod & KMOD_CTRL))
			{
				if (confirm_ui(vid_buf, "You are about to quit", "Are you sure you want to quit?", "Quit"))
				{
					has_quit = 1;
					return 1;
				}
			}
			break;

		case SDL_KEYUP:
			sdl_rkey=event.key.keysym.sym;
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button == SDL_BUTTON_WHEELUP)
				sdl_wheel++;
			if (event.button.button == SDL_BUTTON_WHEELDOWN)
				sdl_wheel--;
			break;
		case SDL_QUIT:
			if (fastquit)
				has_quit = 1;
			return 1;
		case SDL_SYSWMEVENT:
#if (defined(LIN32) || defined(LIN64)) && defined(SDL_VIDEO_DRIVER_X11)
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
					Atom targets[] = {XA_TARGETS, XA_STRING};
					xr.xselection.property = xe.xselectionrequest.property;
					XChangeProperty(sdl_wminfo.info.x11.display, xe.xselectionrequest.requestor, xe.xselectionrequest.property, XA_ATOM, 32, PropModeReplace, (unsigned char*)targets, (int)(sizeof(targets)/sizeof(Atom)));
				}
				// TODO: Supporting more targets would be nice
				else if (xe.xselectionrequest.target==XA_STRING && clipboard_text)
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
#elif defined(WIN32)
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
#endif
			continue;
		}
	}
	sdl_mod = SDL_GetModState();
	limit_fps();
	return 0;
}

void stickmen_keys()
{
	//  4
	//1 8 2
	if (sdl_key == SDLK_RIGHT)
	{
		player.comm = (int)(player.comm)|0x02;  //Go right command
	}
	if (sdl_key == SDLK_LEFT)
	{
		player.comm = (int)(player.comm)|0x01;  //Go left command
	}
	if (sdl_key == SDLK_DOWN && ((int)(player.comm)&0x08)!=0x08)
	{
		player.comm = (int)(player.comm)|0x08;  //Use element command
	}
	if (sdl_key == SDLK_UP && ((int)(player.comm)&0x04)!=0x04)
	{
		player.comm = (int)(player.comm)|0x04;  //Jump command
	}

	if (sdl_key == SDLK_d)
	{
		player2.comm = (int)(player2.comm)|0x02;  //Go right command
	}
	if (sdl_key == SDLK_a)
	{
		player2.comm = (int)(player2.comm)|0x01;  //Go left command
	}
	if (sdl_key == SDLK_s && ((int)(player2.comm)&0x08)!=0x08)
	{
		player2.comm = (int)(player2.comm)|0x08;  //Use element command
	}
	if (sdl_key == SDLK_w && ((int)(player2.comm)&0x04)!=0x04)
	{
		player2.comm = (int)(player2.comm)|0x04;  //Jump command
	}

	if (sdl_rkey == SDLK_RIGHT || sdl_rkey == SDLK_LEFT)
	{
		player.pcomm = player.comm;  //Saving last movement
		player.comm = (int)(player.comm)&12;  //Stop command
	}
	if (sdl_rkey == SDLK_UP)
	{
		player.comm = (int)(player.comm)&11;
	}
	if (sdl_rkey == SDLK_DOWN)
	{
		player.comm = (int)(player.comm)&7;
	}

	if (sdl_rkey == SDLK_d || sdl_rkey == SDLK_a)
	{
		player2.pcomm = player2.comm;  //Saving last movement
		player2.comm = (int)(player2.comm)&12;  //Stop command
	}
	if (sdl_rkey == SDLK_w)
	{
		player2.comm = (int)(player2.comm)&11;
	}
	if (sdl_rkey == SDLK_s)
	{
		player2.comm = (int)(player2.comm)&7;
	}
}

int FPS = 0, pastFPS = 0;
float FPSB2 = 0;
float frameTime;
float frameTimeAvg = 0.0f, correctedFrameTimeAvg = 0.0f;
void limit_fps()
{
	frameTime = (float)(SDL_GetTicks() - currentTime);

	frameTimeAvg = (frameTimeAvg*(1.0f-0.2f)) + (0.2f*frameTime);
	if(limitFPS > 2)
	{
		float targetFrameTime = 1000.0f/((float)limitFPS);
		if(targetFrameTime - frameTimeAvg > 0)
		{
			SDL_Delay((Uint32)((targetFrameTime - frameTimeAvg) + 0.5f));
			frameTime = (float)(SDL_GetTicks() - currentTime);//+= (int)(targetFrameTime - frameTimeAvg);
		}
	}

	correctedFrameTimeAvg = (correctedFrameTimeAvg*(1.0f-0.05f)) + (0.05f*frameTime);
	elapsedTime = currentTime-pastFPS;
	if (elapsedTime>=500)
	{
		if (!FPSwait)
			FPSB2 = 1000.0f/correctedFrameTimeAvg;
		if (main_loop && FPSwait > 0)
			FPSwait--;
		pastFPS = currentTime;
	}
	currentTime = SDL_GetTicks();
}

void set_cmode(int cm) // sets to given view mode
{
	int cmode = cm;
	colour_mode = COLOUR_DEFAULT;
	
	free(render_modes);
	render_modes = (unsigned int*)calloc(2, sizeof(unsigned int));
	render_modes[0] = RENDER_BASC;
	render_modes[1] = 0;
	
	free(display_modes);
	display_modes = (unsigned int*)calloc(1, sizeof(unsigned int));
	display_modes[0] = 0;
	
	itc = 51;
	if (cmode==CM_VEL)
	{
		free(render_modes);
		render_modes = (unsigned int*)calloc(3, sizeof(unsigned int));
		render_modes[0] = RENDER_EFFE;
		render_modes[1] = RENDER_BASC;
		render_modes[2] = 0;
		free(display_modes);
		display_modes = (unsigned int*)calloc(2, sizeof(unsigned int));
		display_modes[0] = DISPLAY_AIRV;
		display_modes[1] = 0;
		strcpy(itc_msg, "Velocity Display");
	}
	else if (cmode==CM_PRESS)
	{
		free(render_modes);
		render_modes = (unsigned int*)calloc(3, sizeof(unsigned int));
		render_modes[0] = RENDER_EFFE;
		render_modes[1] = RENDER_BASC;
		render_modes[2] = 0;
		free(display_modes);
		display_modes = (unsigned int*)calloc(2, sizeof(unsigned int));
		display_modes[0] = DISPLAY_AIRP;
		display_modes[1] = 0;
		strcpy(itc_msg, "Pressure Display");
	}
	else if (cmode==CM_PERS)
	{
		free(render_modes);
		render_modes = (unsigned int*)calloc(3, sizeof(unsigned int));
		render_modes[0] = RENDER_EFFE;
		render_modes[1] = RENDER_BASC;
		render_modes[2] = 0;
		free(display_modes);
		display_modes = (unsigned int*)calloc(2, sizeof(unsigned int));
		display_modes[0] = DISPLAY_PERS;
		display_modes[1] = 0;
		memset(pers_bg, 0, (XRES+BARSIZE)*YRES*PIXELSIZE);
		strcpy(itc_msg, "Persistent Display");
	}
	else if (cmode==CM_FIRE)
	{
		free(render_modes);
		render_modes = (unsigned int*)calloc(4, sizeof(unsigned int));
		render_modes[0] = RENDER_FIRE;
		render_modes[1] = RENDER_EFFE;
		render_modes[2] = RENDER_BASC;
		render_modes[3] = 0;
		memset(fire_r, 0, sizeof(fire_r));
		memset(fire_g, 0, sizeof(fire_g));
		memset(fire_b, 0, sizeof(fire_b));
		strcpy(itc_msg, "Fire Display");
	}
	else if (cmode==CM_BLOB)
	{
		free(render_modes);
		render_modes = (unsigned int*)calloc(4, sizeof(unsigned int));
		render_modes[0] = RENDER_FIRE;
		render_modes[1] = RENDER_EFFE;
		render_modes[2] = RENDER_BLOB;
		render_modes[3] = 0;
		memset(fire_r, 0, sizeof(fire_r));
		memset(fire_g, 0, sizeof(fire_g));
		memset(fire_b, 0, sizeof(fire_b));
		strcpy(itc_msg, "Blob Display");
	}
	else if (cmode==CM_HEAT)
	{
		colour_mode = COLOUR_HEAT;
		strcpy(itc_msg, "Heat Display");
		free(display_modes);
		display_modes = (unsigned int*)calloc(2, sizeof(unsigned int));
		display_mode |= DISPLAY_AIRH;
		display_modes[0] = DISPLAY_AIRH;
		display_modes[1] = 0;
	}
	else if (cmode==CM_FANCY)
	{
		free(render_modes);
		render_modes = (unsigned int*)calloc(6, sizeof(unsigned int));
		render_modes[0] = RENDER_FIRE;
		render_modes[1] = RENDER_GLOW;
		render_modes[2] = RENDER_BLUR;
		render_modes[3] = RENDER_EFFE;
		render_modes[4] = RENDER_BASC;
		render_modes[5] = 0;
		free(display_modes);
		display_modes = (unsigned int*)calloc(2, sizeof(unsigned int));
		display_modes[0] = DISPLAY_WARP;
		display_modes[1] = 0;
		memset(fire_r, 0, sizeof(fire_r));
		memset(fire_g, 0, sizeof(fire_g));
		memset(fire_b, 0, sizeof(fire_b));
		strcpy(itc_msg, "Fancy Display");
	}
	else if (cmode==CM_NOTHING)
	{
		strcpy(itc_msg, "Nothing Display");
	}
	else if (cmode==CM_GRAD)
	{
		colour_mode = COLOUR_GRAD;
		strcpy(itc_msg, "Heat Gradient Display");
	}
	else if (cmode==CM_LIFE)
	{
		colour_mode = COLOUR_LIFE;
		strcpy(itc_msg, "Life Gradient Display");
	}
	else if (cmode==CM_CRACK)
	{
		free(render_modes);
		render_modes = (unsigned int*)calloc(3, sizeof(unsigned int));
		render_modes[0] = RENDER_EFFE;
		render_modes[1] = RENDER_BASC;
		render_modes[2] = 0;
		free(display_modes);
		display_modes = (unsigned int*)calloc(2, sizeof(unsigned int));
		display_modes[0] = DISPLAY_AIRC;
		display_modes[1] = 0;
		strcpy(itc_msg, "Alternate Velocity Display");
	}
	else //if no special text given, it will display this.
	{
		strcpy(itc_msg, "Error: Incorrect Display Number");
	}

	update_display_modes();// Update render_mode and display_mode from the relevant arrays
	save_presets(0);
}

char *download_ui(pixel *vid_buf, char *uri, int *len)
{
	int dstate = 0;
	void *http = http_async_req_start(NULL, uri, NULL, 0, 0);
	int x0=(XRES-240)/2,y0=(YRES-MENUSIZE)/2;
	int done, total, i, ret, zlen, ulen;
	char str[16], *tmp, *res;
	
	//if (svf_login) {
	//	http_auth_headers(http, svf_user_id, NULL, svf_session_id);
	//}

	while (!http_async_req_status(http))
	{
		sdl_poll();

		http_async_get_length(http, &total, &done);

		clearrect(vid_buf, x0-2, y0-2, 244, 64);
		drawrect(vid_buf, x0, y0, 240, 60, 192, 192, 192, 255);
		drawtext(vid_buf, x0+8, y0+8, "Please wait", 255, 216, 32, 255);
		drawtext(vid_buf, x0+8, y0+26, "Downloading update...", 255, 255, 255, 255);

		if (total)
		{
			i = (236*done)/total;
			fillrect(vid_buf, x0+1, y0+45, i+1, 14, 255, 216, 32, 255);
			i = (100*done)/total;
			sprintf(str, "%d%%", i);
			if (i<50)
				drawtext(vid_buf, x0+120-textwidth(str)/2, y0+48, str, 192, 192, 192, 255);
			else
				drawtext(vid_buf, x0+120-textwidth(str)/2, y0+48, str, 0, 0, 0, 255);
		}
		else
			drawtext(vid_buf, x0+120-textwidth("Waiting...")/2, y0+48, "Waiting...", 255, 216, 32, 255);

		drawrect(vid_buf, x0, y0+44, 240, 16, 192, 192, 192, 255);
#ifdef OGLR
		clearScreen(1.0f);
#endif
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
	}

	tmp = http_async_req_stop(http, &ret, &zlen);
	if (ret!=200)
	{
		error_ui(vid_buf, ret, http_ret_text(ret));
		if (tmp)
			free(tmp);
		return NULL;
	}
	if (!tmp)
	{
		error_ui(vid_buf, 0, "Server did not return data");
		return NULL;
	}

	if (zlen<16)
	{
		printf("ZLen is not 16!\n");
		goto corrupt;
	}
	if (tmp[0]!=0x42 || tmp[1]!=0x75 || tmp[2]!=0x54 || tmp[3]!=0x54)
	{
		printf("Tmperr %d, %d, %d, %d\n", tmp[0], tmp[1], tmp[2], tmp[3]);
		goto corrupt;
	}

	ulen  = (unsigned char)tmp[4];
	ulen |= ((unsigned char)tmp[5])<<8;
	ulen |= ((unsigned char)tmp[6])<<16;
	ulen |= ((unsigned char)tmp[7])<<24;

	res = (char *)malloc(ulen);
	if (!res)
	{
		printf("No res!\n");
		goto corrupt;
	}
	dstate = BZ2_bzBuffToBuffDecompress((char *)res, (unsigned *)&ulen, (char *)(tmp+8), zlen-8, 0, 0);
	if (dstate)
	{
		printf("Decompression failure: %d, %d, %d!\n", dstate, ulen, zlen);
		free(res);
		goto corrupt;
	}
	
	free(tmp);
	if (len)
		*len = ulen;
	return res;

corrupt:
	error_ui(vid_buf, 0, "Downloaded update is corrupted");
	free(tmp);
	return NULL;
}

int search_ui(pixel *vid_buf)
{
	int nmp=-1,uih=0,nyu,nyd,b=1,bq,mx=0,my=0,mxq=0,myq=0,mmt=0,gi,gj,gx,gy,pos,i,mp,dp,dap,own,last_own=search_own,last_fav=search_fav,page_count=0,last_page=0,last_date=0,j,w,h,st=0,lv;
	int is_p1=0, exp_res=GRID_X*GRID_Y, tp, view_own=0, last_p1_extra=0, motdswap = rand()%2;
	int thumb_drawn[GRID_X*GRID_Y];
	pixel *v_buf = (pixel *)malloc(((YRES+MENUSIZE)*(XRES+BARSIZE))*PIXELSIZE);
	pixel *bthumb_rsdata = NULL;
	float ry;
	time_t http_last_use=HTTP_TIMEOUT;
	ui_edit ed;
	ui_richtext motd;


	void *http = NULL;
	int active = 0;
	char *last = NULL;
	int search = 0;
	int lasttime = TIMEOUT;
	char *uri;
	int status;
	char *results;
	char *tmp, ts[64];

	void *img_http[IMGCONNS];
	char *img_id[IMGCONNS];
	void *thumb, *data;
	int thlen, dlen;

	if (!v_buf)
		return 0;
	memset(v_buf, 0, ((YRES+MENUSIZE)*(XRES+BARSIZE))*PIXELSIZE);

	memset(img_http, 0, sizeof(img_http));
	memset(img_id, 0, sizeof(img_id));

	memset(search_ids, 0, sizeof(search_ids));
	memset(search_dates, 0, sizeof(search_dates));
	memset(search_names, 0, sizeof(search_names));
	memset(search_scoreup, 0, sizeof(search_scoreup));
	memset(search_scoredown, 0, sizeof(search_scoredown));
	memset(search_publish, 0, sizeof(search_publish));
	memset(search_owners, 0, sizeof(search_owners));
	memset(search_thumbs, 0, sizeof(search_thumbs));
	memset(search_thsizes, 0, sizeof(search_thsizes));

	memset(thumb_drawn, 0, sizeof(thumb_drawn));

	do_open = 0;

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	ui_edit_init(&ed, 65, 13, XRES-200, 14);
	ed.def = "[search terms]";
	ed.cursor = ed.cursorstart = strlen(search_expr);
	strcpy(ed.str, search_expr);

	motd.x = 20;
	motd.y = 33;
	motd.str[0] = 0;
	motd.printstr[0] = 0;

	sdl_wheel = 0;

	while (!sdl_poll())
	{
		uih = 0;
		bq = b;
		mxq = mx;
		myq = my;
		b = mouse_get_state(&mx, &my);

		if (mx!=mxq || my!=myq || sdl_wheel || b)
			mmt = 0;
		else if (mmt<TIMEOUT)
			mmt++;

		clearrect(vid_buf, -1, -1, (XRES+BARSIZE)+1, YRES+MENUSIZE+1);

		memcpy(vid_buf, v_buf, ((YRES+MENUSIZE)*(XRES+BARSIZE))*PIXELSIZE);

		drawtext(vid_buf, 11, 13, "Search:", 192, 192, 192, 255);
		if (!last || (!active && strcmp(last, ed.str)))
			drawtext(vid_buf, 51, 11, "\x8E", 192, 160, 32, 255);
		else
			drawtext(vid_buf, 51, 11, "\x8E", 32, 64, 160, 255);
		drawtext(vid_buf, 51, 11, "\x8F", 255, 255, 255, 255);
		drawrect(vid_buf, 48, 8, XRES-182, 16, 192, 192, 192, 255);

		if (!svf_login || search_fav)
		{
			search_own = 0;
			drawrect(vid_buf, XRES-64+16, 8, 56, 16, 96, 96, 96, 255);
			drawtext(vid_buf, XRES-61+16, 11, "\x94", 96, 80, 16, 255);
			drawtext(vid_buf, XRES-61+16, 11, "\x93", 128, 128, 128, 255);
			drawtext(vid_buf, XRES-46+16, 13, "My Own", 128, 128, 128, 255);
		}
		else if (search_own)
		{
			fillrect(vid_buf, XRES-65+16, 7, 58, 18, 255, 255, 255, 255);
			drawtext(vid_buf, XRES-61+16, 11, "\x94", 192, 160, 64, 255);
			drawtext(vid_buf, XRES-61+16, 11, "\x93", 32, 32, 32, 255);
			drawtext(vid_buf, XRES-46+16, 13, "My Own", 0, 0, 0, 255);
		}
		else
		{
			drawrect(vid_buf, XRES-64+16, 8, 56, 16, 192, 192, 192, 255);
			drawtext(vid_buf, XRES-61+16, 11, "\x94", 192, 160, 32, 255);
			drawtext(vid_buf, XRES-61+16, 11, "\x93", 255, 255, 255, 255);
			drawtext(vid_buf, XRES-46+16, 13, "My Own", 255, 255, 255, 255);
		}

		if(!svf_login)
		{
			search_fav = 0;
			drawrect(vid_buf, XRES-134, 8, 16, 16, 192, 192, 192, 255);
			drawtext(vid_buf, XRES-130, 11, "\xCC", 120, 120, 120, 255);
		}
		else if (search_fav)
		{
			fillrect(vid_buf, XRES-134, 7, 18, 18, 255, 255, 255, 255);
			drawtext(vid_buf, XRES-130, 11, "\xCC", 192, 160, 64, 255);
		}
		else
		{
			drawrect(vid_buf, XRES-134, 8, 16, 16, 192, 192, 192, 255);
			drawtext(vid_buf, XRES-130, 11, "\xCC", 192, 160, 32, 255);
		}

		if(search_fav)
		{
			search_date = 0;
			drawrect(vid_buf, XRES-129+16, 8, 60, 16, 96, 96, 96, 255);
			drawtext(vid_buf, XRES-126+16, 11, "\xA9", 44, 48, 32, 255);
			drawtext(vid_buf, XRES-126+16, 11, "\xA8", 32, 44, 32, 255);
			drawtext(vid_buf, XRES-126+16, 11, "\xA7", 128, 128, 128, 255);
			drawtext(vid_buf, XRES-111+16, 13, "By votes", 128, 128, 128, 255);
		}
		else if (search_date)
		{
			fillrect(vid_buf, XRES-130+16, 7, 62, 18, 255, 255, 255, 255);
			drawtext(vid_buf, XRES-126+16, 11, "\xA6", 32, 32, 32, 255);
			drawtext(vid_buf, XRES-111+16, 13, "By date", 0, 0, 0, 255);
		}
		else
		{
			drawrect(vid_buf, XRES-129+16, 8, 60, 16, 192, 192, 192, 255);
			drawtext(vid_buf, XRES-126+16, 11, "\xA9", 144, 48, 32, 255);
			drawtext(vid_buf, XRES-126+16, 11, "\xA8", 32, 144, 32, 255);
			drawtext(vid_buf, XRES-126+16, 11, "\xA7", 255, 255, 255, 255);
			drawtext(vid_buf, XRES-111+16, 13, "By votes", 255, 255, 255, 255);
		}

		if (search_page)
		{
			drawtext(vid_buf, 4, YRES+MENUSIZE-16, "\x96", 255, 255, 255, 255);
			drawrect(vid_buf, 1, YRES+MENUSIZE-20, 16, 16, 255, 255, 255, 255);
		}
		else if (page_count > exp_res && !(search_own || search_fav || search_date) && !strcmp(ed.str,""))
		{
			if (p1_extra)
				drawtext(vid_buf, 4, YRES+MENUSIZE-17, "\x85", 255, 255, 255, 255);
			else
				drawtext(vid_buf, 4, YRES+MENUSIZE-17, "\x89", 255, 255, 255, 255);
			drawrect(vid_buf, 1, YRES+MENUSIZE-20, 15, 15, 255, 255, 255, 255);
		}
		if (page_count > exp_res)
		{
			drawtext(vid_buf, XRES-15, YRES+MENUSIZE-16, "\x95", 255, 255, 255, 255);
			drawrect(vid_buf, XRES-18, YRES+MENUSIZE-20, 16, 16, 255, 255, 255, 255);
		}
		if (page_count)
		{
			char pagecount[16];
			sprintf(pagecount,"Page %i",search_page+1);
			drawtext(vid_buf, (XRES-textwidth(pagecount))/2, YRES+MENUSIZE-10, pagecount, 255, 255, 255, 255);
		}

		ui_edit_draw(vid_buf, &ed);

		if ((b && !bq && mx>=1 && mx<=17 && my>=YRES+MENUSIZE-20 && my<YRES+MENUSIZE-4) || sdl_wheel>0)
		{
			if (search_page)
				search_page --;
			else if (!(search_own || search_fav || search_date) && !sdl_wheel)
				p1_extra = !p1_extra;
			lasttime = TIMEOUT;
			sdl_wheel = 0;
			uih = 1;
		}
		if ((b && !bq && mx>=XRES-18 && mx<=XRES-1 && my>=YRES+MENUSIZE-20 && my<YRES+MENUSIZE-4) || sdl_wheel<0)
		{
			if (page_count>exp_res)
			{
				lasttime = TIMEOUT;
				search_page ++;
				page_count = exp_res;
			}
			sdl_wheel = 0;
			uih = 1;
		}

		tp = -1;
		if (is_p1)
		{	
			//Message of the day
			ui_richtext_process(mx, my, b, bq, &motd);
			ui_richtext_draw(vid_buf, &motd);
			//Popular tags
			drawtext(vid_buf, (XRES-textwidth("Popular tags:"))/2, 49, "Popular tags:", 255, 192, 64, 255);
			for (gj=0; gj<((GRID_Y-GRID_P)*YRES)/(GRID_Y*14); gj++)
				for (gi=0; gi<(GRID_X+1); gi++)
				{
					pos = gi+(GRID_X+1)*gj;
					if (pos>TAG_MAX || !tag_names[pos])
						break;
					if (tag_votes[0])
						i = 127+(128*tag_votes[pos])/tag_votes[0];
					else
						i = 192;
					w = textwidth(tag_names[pos]);
					if (w>XRES/(GRID_X+1)-5)
						w = XRES/(GRID_X+1)-5;
					gx = (XRES/(GRID_X+1))*gi;
					gy = gj*13 + 62;
					if (mx>=gx && mx<gx+(XRES/((GRID_X+1)+1)) && my>=gy && my<gy+14)
					{
						j = (i*5)/6;
						tp = pos;
					}
					else
						j = i;
					drawtextmax(vid_buf, gx+(XRES/(GRID_X+1)-w)/2, gy, XRES/(GRID_X+1)-5, tag_names[pos], j, j, i, 255);
				}
		}

		mp = dp = -1;
		dap = -1;
		st = 0;
		for (gj=0; gj<GRID_Y; gj++)
			for (gi=0; gi<GRID_X; gi++)
			{
				if (is_p1)
				{
					pos = gi+GRID_X*(gj-GRID_Y+GRID_P);
					if (pos<0)
						break;
				}
				else
					pos = gi+GRID_X*gj;
				if (!search_ids[pos])
					break;
				gx = ((XRES/GRID_X)*gi) + (XRES/GRID_X-XRES/GRID_S)/2;
				gy = ((((YRES-(MENUSIZE-20))+15)/GRID_Y)*gj) + ((YRES-(MENUSIZE-20))/GRID_Y-(YRES-(MENUSIZE-20))/GRID_S+10)/2 + 18;
				if (textwidth(search_names[pos]) > XRES/GRID_X-10)
				{
					tmp = (char*)malloc(strlen(search_names[pos])+4);
					strcpy(tmp, search_names[pos]);
					j = textwidthx(tmp, XRES/GRID_X-15);
					strcpy(tmp+j, "...");
					drawtext(vid_buf, gx+XRES/(GRID_S*2)-textwidth(tmp)/2, gy+YRES/GRID_S+7, tmp, 192, 192, 192, 255);
					free(tmp);
				}
				else
					drawtext(vid_buf, gx+XRES/(GRID_S*2)-textwidth(search_names[pos])/2, gy+YRES/GRID_S+7, search_names[pos], 192, 192, 192, 255);
				j = textwidth(search_owners[pos]);
				if (mx>=gx+XRES/(GRID_S*2)-j/2 && mx<=gx+XRES/(GRID_S*2)+j/2 &&
				        my>=gy+YRES/GRID_S+18 && my<=gy+YRES/GRID_S+29)
				{
					st = 1;
					drawtext(vid_buf, gx+XRES/(GRID_S*2)-j/2, gy+YRES/GRID_S+20, search_owners[pos], 128, 128, 160, 255);
				}
				else
					drawtext(vid_buf, gx+XRES/(GRID_S*2)-j/2, gy+YRES/GRID_S+20, search_owners[pos], 128, 128, 128, 255);
				if (search_thumbs[pos]&&thumb_drawn[pos]==0)
				{
					//render_thumb(search_thumbs[pos], search_thsizes[pos], 1, v_buf, gx, gy, GRID_S);
					int finh, finw;
					pixel *thumb_rsdata = NULL;
					pixel *thumb_imgdata = ptif_unpack(search_thumbs[pos], search_thsizes[pos], &finw, &finh);
					if(thumb_imgdata!=NULL){
						thumb_rsdata = resample_img(thumb_imgdata, finw, finh, XRES/GRID_S, YRES/GRID_S);
						draw_image(v_buf, thumb_rsdata, gx, gy, XRES/GRID_S, YRES/GRID_S, 255);					
						free(thumb_imgdata);
						free(thumb_rsdata);
					}
					thumb_drawn[pos] = 1;
				}
				own = (svf_login && (!strcmp(svf_user, search_owners[pos]) || svf_admin || svf_mod));
				if (mx>=gx-2 && mx<=gx+XRES/GRID_S+3 && my>=gy && my<=gy+YRES/GRID_S+29)
					mp = pos;
				if ((own || search_fav) && mx>=gx+XRES/GRID_S-4 && mx<=gx+XRES/GRID_S+6 && my>=gy-6 && my<=gy+4)
				{
					mp = -1;
					dp = pos;
				}
				if ((own || (unlockedstuff & 0x08)) && !search_dates[pos] && mx>=gx-6 && mx<=gx+4 && my>=gy+YRES/GRID_S-4 && my<=gy+YRES/GRID_S+6)
				{
					mp = -1;
					dap = pos;
				}
				drawrect(vid_buf, gx-2+(XRES/GRID_S)+5, gy-2, 6, YRES/GRID_S+3, 128, 128, 128, 255);
				fillrect(vid_buf, gx-2+(XRES/GRID_S)+5, gy-2, 6, 1+(YRES/GRID_S+3)/2, 0, 107, 10, 255);
				fillrect(vid_buf, gx-2+(XRES/GRID_S)+5, gy-2+((YRES/GRID_S+3)/2), 6, 1+(YRES/GRID_S+3)/2, 107, 10, 0, 255);

				if (mp==pos && !st)
					drawrect(vid_buf, gx-2, gy-2, XRES/GRID_S+3, YRES/GRID_S+3, 160, 160, 192, 255);
				else
					drawrect(vid_buf, gx-2, gy-2, XRES/GRID_S+3, YRES/GRID_S+3, 128, 128, 128, 255);
				if (own || search_fav)
				{
					if (dp == pos)
						drawtext(vid_buf, gx+XRES/GRID_S-4, gy-6, "\x86", 255, 48, 32, 255);
					else
						drawtext(vid_buf, gx+XRES/GRID_S-4, gy-6, "\x86", 160, 48, 32, 255);
					drawtext(vid_buf, gx+XRES/GRID_S-4, gy-6, "\x85", 255, 255, 255, 255);
				}
				if (!search_publish[pos])
				{
					drawtext(vid_buf, gx-6, gy-6, "\xCD", 255, 255, 255, 255);
					drawtext(vid_buf, gx-6, gy-6, "\xCE", 212, 151, 81, 255);
				}
				if (!search_dates[pos] && (own || (unlockedstuff & 0x08)))
				{
					fillrect(vid_buf, gx-5, gy+YRES/GRID_S-3, 7, 8, 255, 255, 255, 255);
					if (dap == pos) {
						drawtext(vid_buf, gx-6, gy+YRES/GRID_S-4, "\xA6", 200, 100, 80, 255);
					} else {
						drawtext(vid_buf, gx-6, gy+YRES/GRID_S-4, "\xA6", 160, 70, 50, 255);
					}
					//drawtext(vid_buf, gx-6, gy-6, "\xCE", 212, 151, 81, 255);
				}
				if (view_own || svf_admin || svf_mod || (unlockedstuff & 0x08))
				{
					sprintf(ts+1, "%d", search_votes[pos]);
					ts[0] = (char)0xBB;
					for (j=1; ts[j]; j++)
						ts[j] = (char)0xBC;
					ts[j-1] = (char)0xB9;
					ts[j] = (char)0xBA;
					ts[j+1] = 0;
					w = gx+XRES/GRID_S-2-textwidth(ts);
					h = gy+YRES/GRID_S-11;
					drawtext(vid_buf, w, h, ts, 16, 72, 16, 255);
					for (j=0; ts[j]; j++)
						ts[j] -= 14;
					drawtext(vid_buf, w, h, ts, 192, 192, 192, 255);
					sprintf(ts, "%d", search_votes[pos]);
					for (j=0; ts[j]; j++)
						if (ts[j] != '-')
							ts[j] += 127;
					drawtext(vid_buf, w+3, h, ts, 255, 255, 255, 255);
				}
				if (search_scoreup[pos]>0||search_scoredown[pos]>0)
				{
					lv = (search_scoreup[pos]>search_scoredown[pos]?search_scoreup[pos]:search_scoredown[pos]);

					if (((YRES/GRID_S+3)/2)>lv)
					{
						ry = ((float)((YRES/GRID_S+3)/2)/(float)lv);
						if (lv<8)
						{
							ry =  ry/(8-lv);
						}
						nyu = (int)(search_scoreup[pos]*ry);
						nyd = (int)(search_scoredown[pos]*ry);
					}
					else
					{
						ry = ((float)lv/(float)((YRES/GRID_S+3)/2));
						nyu = (int)(search_scoreup[pos]/ry);
						nyd = (int)(search_scoredown[pos]/ry);
					}


					fillrect(vid_buf, gx-1+(XRES/GRID_S)+5, gy-1+((YRES/GRID_S+3)/2)-nyu, 4, nyu, 57, 187, 57, 255);
					fillrect(vid_buf, gx-1+(XRES/GRID_S)+5, gy-2+((YRES/GRID_S+3)/2), 4, nyd, 187, 57, 57, 255);
					//drawrect(vid_buf, gx-2+(XRES/GRID_S)+5, gy-2+((YRES/GRID_S+3)/2)-nyu, 4, nyu, 0, 107, 10, 255);
					//drawrect(vid_buf, gx-2+(XRES/GRID_S)+5, gy-2+((YRES/GRID_S+3)/2)+1, 4, nyd, 107, 10, 0, 255);
				}
			}

		if (mp!=-1 && mmt>=TIMEOUT/5 && !st && my<YRES+MENUSIZE-25)
		{
			gi = mp % GRID_X;
			gj = mp / GRID_X;
			if (is_p1)
				gj += GRID_Y-GRID_P;
			gx = ((XRES/GRID_X)*gi) + (XRES/GRID_X-XRES/GRID_S)/2;
			gy = (((YRES+15)/GRID_Y)*gj) + (YRES/GRID_Y-YRES/GRID_S+10)/2 + 18;
			i = w = textwidth(search_names[mp]);
			h = YRES/GRID_Z+30;
			if (w<XRES/GRID_Z) w=XRES/GRID_Z;
			gx += XRES/(GRID_S*2)-w/2;
			gy += YRES/(GRID_S*2)-h/2;
			if (gx<2) gx=2;
			if (gx+w>=XRES-2) gx=XRES-3-w;
			if (gy<32) gy=32;
			if (gy+h>=YRES+(MENUSIZE-2)) gy=YRES+(MENUSIZE-3)-h;
			clearrect(vid_buf, gx-2, gy-3, w+4, h);
			drawrect(vid_buf, gx-2, gy-3, w+4, h, 160, 160, 192, 255);
			if (search_thumbs[mp]){
				if(mp != nmp && bthumb_rsdata){
					free(bthumb_rsdata);
					bthumb_rsdata = NULL;
				}
				if(!bthumb_rsdata){
					int finh, finw;
					pixel *thumb_imgdata = ptif_unpack(search_thumbs[mp], search_thsizes[mp], &finw, &finh);
					if(thumb_imgdata!=NULL){
						bthumb_rsdata = resample_img(thumb_imgdata, finw, finh, XRES/GRID_Z, YRES/GRID_Z);				
						free(thumb_imgdata);
					}
				}
				draw_image(vid_buf, bthumb_rsdata, gx+(w-(XRES/GRID_Z))/2, gy, XRES/GRID_Z, YRES/GRID_Z, 255);
				nmp = mp;
			}
			drawtext(vid_buf, gx+(w-i)/2, gy+YRES/GRID_Z+4, search_names[mp], 192, 192, 192, 255);
			drawtext(vid_buf, gx+(w-textwidth(search_owners[mp]))/2, gy+YRES/GRID_Z+16, search_owners[mp], 128, 128, 128, 255);
		}
#ifdef OGLR
		clearScreen(1.0f);
#endif
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		ui_edit_process(mx, my, b, bq, &ed);

		if (sdl_key==SDLK_RETURN)
		{
			if (!last || (!active && (strcmp(last, ed.str) || last_own!=search_own || last_date!=search_date || last_page!=search_page)))
				lasttime = TIMEOUT;
			else if (search_ids[0] && !search_ids[1])
			{
				bq = 0;
				b = 1;
				mp = 0;
			}
		}
		if (sdl_key==SDLK_ESCAPE)
		{
			strcpy(search_expr, ed.str);
			goto finish;
		}

		if (b && !bq && mx>=XRES-64+16 && mx<=XRES-8+16 && my>=8 && my<=24 && svf_login && !search_fav)
		{
			search_own = !search_own;
			lasttime = TIMEOUT;
		}
		if (b && !bq && mx>=XRES-129+16 && mx<=XRES-65+16 && my>=8 && my<=24 && !search_fav)
		{
			search_date = !search_date;
			lasttime = TIMEOUT;
		}
		if (b && !bq && mx>=XRES-134 && mx<=XRES-134+16 && my>=8 && my<=24 && svf_login)
		{
			search_fav = !search_fav;
			search_own = 0;
			search_date = 0;
			lasttime = TIMEOUT;
		}

		if (b && !bq && dp!=-1)
		{
			if (search_fav){
				if(confirm_ui(vid_buf, "Remove from favorites?", search_names[dp], "Remove")){
					execute_unfav(vid_buf, search_ids[dp]);
					lasttime = TIMEOUT;
					if (last)
					{
						free(last);
						last = NULL;
					}
				}
			} else {
				if (confirm_ui(vid_buf, "Do you want to delete?", search_names[dp], "Delete"))
				{
					execute_delete(vid_buf, search_ids[dp]);
					lasttime = TIMEOUT;
					if (last)
					{
						free(last);
						last = NULL;
					}
				}
			}
		}
		if (b && !bq && dap!=-1)
		{
			sprintf(ed.str, "history:%s", search_ids[dap]);
			lasttime = TIMEOUT;
		}

		if (b && !bq && tp!=-1)
		{
			strncpy(ed.str, tag_names[tp], 255);
			lasttime = TIMEOUT;
		}

		if (b && !bq && mp!=-1 && st)
		{
			sprintf(ed.str, "user:%s", search_owners[mp]);
			lasttime = TIMEOUT;
		}

		if (do_open==1)
		{
			mp = 0;
		}

		if ((b && !bq && mp!=-1 && !st && !uih) || do_open==1)
		{
			strcpy(search_expr, ed.str);
			if (open_ui(vid_buf, search_ids[mp], search_dates[mp]?search_dates[mp]:NULL, sdl_mod&KMOD_CTRL) || do_open==1) {
				goto finish;
			}
		}

		if (!last)
		{
			search = 1;
		}
		else if (!active && (strcmp(last, ed.str) || last_own!=search_own || last_date!=search_date || last_page!=search_page || last_fav!=search_fav || last_p1_extra!=p1_extra))
		{
			search = 1;
			if (strcmp(last, ed.str) || last_own!=search_own || last_fav!=search_fav || last_date!=search_date)
			{
				search_page = 0;
				page_count = 0;
			}
			free(last);
			last = NULL;
		}
		else
			search = 0;

		if (search && lasttime>=TIMEOUT)
		{
			lasttime = 0;
			last = mystrdup(ed.str);
			last_own = search_own;
			last_date = search_date;
			last_page = search_page;
			last_fav = search_fav;
			last_p1_extra = p1_extra;
			active = 1;
			uri = (char*)malloc(strlen(last)*3+180+strlen(SERVER)+strlen(svf_user)+20); //Increase "padding" from 80 to 180 to fix the search memory corruption bug
			if (search_own || svf_admin || svf_mod || (unlockedstuff&0x08))
				tmp = "&ShowVotes=true";
			else
				tmp = "";
			if (!search_own && !search_date && !search_fav && !*last)
			{
				if (search_page)
				{
					exp_res = GRID_X*GRID_Y;
					sprintf(uri, "http://" SERVER "/Search.api?Start=%d&Count=%d%s&Query=", (search_page-1)*GRID_X*GRID_Y+GRID_X*GRID_P, exp_res+1, tmp);
				}
				else
				{
					exp_res = p1_extra?GRID_X*GRID_Y:GRID_X*GRID_P;
					sprintf(uri, "http://" SERVER "/Search.api?Start=%d&Count=%d&t=%d%s&Query=", 0, exp_res+1, ((GRID_Y-GRID_P)*YRES)/(GRID_Y*14)*GRID_X, tmp);
				}
			}
			else
			{
				exp_res = GRID_X*GRID_Y;
				sprintf(uri, "http://" SERVER "/Search.api?Start=%d&Count=%d%s&Query=", search_page*GRID_X*GRID_Y, exp_res+1, tmp);
			}
			strcaturl(uri, last);
			if (search_own)
			{
				strcaturl(uri, " user:");
				strcaturl(uri, svf_user);
			}
			if (search_fav)
			{
				strcaturl(uri, " cat:favs");
			}
			if (search_date)
				strcaturl(uri, " sort:date");

			http = http_async_req_start(http, uri, NULL, 0, 1);
			if (svf_login)
			{
				//http_auth_headers(http, svf_user, svf_pass);
				http_auth_headers(http, svf_user_id, NULL, svf_session_id);
			}
			http_last_use = time(NULL);
			free(uri);
		}

		if (active && http_async_req_status(http))
		{
			http_last_use = time(NULL);
			results = http_async_req_stop(http, &status, NULL);
			view_own = last_own;
			is_p1 = (exp_res < GRID_X*GRID_Y);
			if (status == 200)
			{
				page_count = search_results(results, last_own||svf_admin||svf_mod||(unlockedstuff&0x08));
				memset(thumb_drawn, 0, sizeof(thumb_drawn));
				memset(v_buf, 0, ((YRES+MENUSIZE)*(XRES+BARSIZE))*PIXELSIZE);
				nmp = -1;
				
				if (is_p1)
				{
					if (motdswap)
						sprintf(server_motd,"Links: \bt{a:http://powdertoy.co.uk|Powder Toy main page}\bg, \bt{a:http://powdertoy.co.uk/Discussions/Categories/Index.html|Forums}\bg, \bt{a:https://github.com/FacialTurd/The-Powder-Toy|TPT github}\bg, \bt{a:https://github.com/jacob1/The-Powder-Toy/tree/jacob1's_mod|Jacob1's Mod github}");
					motdswap = !motdswap;
				}
				ui_richtext_settext(server_motd, &motd);
				motd.x = (XRES-textwidth(motd.printstr))/2;
			}
			if (results)
				free(results);
			active = 0;
		}

		if (http && !active && (time(NULL)>http_last_use+HTTP_TIMEOUT))
		{
			http_hackyclosefreezefix(http);
			http_async_req_close(http);
			http = NULL;
		}

		for (i=0; i<IMGCONNS; i++)
		{
			if (img_http[i] && http_async_req_status(img_http[i]))
			{
				thumb = http_async_req_stop(img_http[i], &status, &thlen);
				if (status != 200)
				{
					if (thumb)
						free(thumb);
					thumb = NULL;
				}
				else
					thumb_cache_add(img_id[i], thumb, thlen);
				for (pos=0; pos<GRID_X*GRID_Y; pos++) {
					if (search_dates[pos]) {
						char *id_d_temp = (char*)malloc(strlen(search_ids[pos])+strlen(search_dates[pos])+2);
						if (id_d_temp == 0)
						{
							break;
						}
						strcpy(id_d_temp, search_ids[pos]);
						strappend(id_d_temp, "_");
						strappend(id_d_temp, search_dates[pos]);
						//img_id[i] = mystrdup(id_d_temp);
						if (!strcmp(id_d_temp, img_id[i]) && !search_thumbs[pos]) {
							break;
						}
						free(id_d_temp);
					} else {
						if (search_ids[pos] && !strcmp(search_ids[pos], img_id[i])) {
							break;
						}
					}
				}
				if (thumb)
				{
					if (pos<GRID_X*GRID_Y)
					{
						search_thumbs[pos] = thumb;
						search_thsizes[pos] = thlen;
					}
					else
						free(thumb);
				}
				free(img_id[i]);
				img_id[i] = NULL;
			}
			if (!img_id[i])
			{
				for (pos=0; pos<GRID_X*GRID_Y; pos++)
					if (search_ids[pos] && !search_thumbs[pos])
					{
						if (search_dates[pos])
						{
							char *id_d_temp = (char*)malloc(strlen(search_ids[pos])+strlen(search_dates[pos])+2);
							strcpy(id_d_temp, search_ids[pos]);
							strappend(id_d_temp, "_");
							strappend(id_d_temp, search_dates[pos]);
							
							for (gi=0; gi<IMGCONNS; gi++)
								if (img_id[gi] && !strcmp(id_d_temp, img_id[gi]))
									break;
									
							free(id_d_temp);
						}
						else
						{
							for (gi=0; gi<IMGCONNS; gi++)
								if (img_id[gi] && !strcmp(search_ids[pos], img_id[gi]))
									break;
						}
						if (gi<IMGCONNS)
							continue;
						break;
					}
				if (pos<GRID_X*GRID_Y)
				{
					if (search_dates[pos]) {
						char *id_d_temp = (char*)malloc(strlen(search_ids[pos])+strlen(search_dates[pos])+2);
						uri = (char*)malloc(strlen(search_ids[pos])*3+strlen(search_dates[pos])*3+strlen(STATICSERVER)+71);
						strcpy(uri, "http://" STATICSERVER "/");
						strcaturl(uri, search_ids[pos]);
						strappend(uri, "_");
						strcaturl(uri, search_dates[pos]);
						strappend(uri, "_small.pti");

						strcpy(id_d_temp, search_ids[pos]);
						strappend(id_d_temp, "_");
						strappend(id_d_temp, search_dates[pos]);
						img_id[i] = mystrdup(id_d_temp);
						free(id_d_temp);
					} else {
						uri = (char*)malloc(strlen(search_ids[pos])*3+strlen(SERVER)+64);
						strcpy(uri, "http://" STATICSERVER "/");
						strcaturl(uri, search_ids[pos]);
						strappend(uri, "_small.pti");
						img_id[i] = mystrdup(search_ids[pos]);
					}
					img_http[i] = http_async_req_start(img_http[i], uri, NULL, 0, 1);
					free(uri);
				}
			}
			if (!img_id[i] && img_http[i])
			{
				http_hackyclosefreezefix(img_http[i]);
				http_async_req_close(img_http[i]);
				img_http[i] = NULL;
			}
		}

		if (lasttime<TIMEOUT)
			lasttime++;
	}

	strcpy(search_expr, ed.str);
finish:
	if (last)
		free(last);
	if (http)
	{
		http_hackyclosefreezefix(http);
		http_async_req_close(http);
	}
	for (i=0; i<IMGCONNS; i++)
		if (img_http[i])
		{
			http_hackyclosefreezefix(img_http[i]);
			http_async_req_close(img_http[i]);
		}
			
	if(bthumb_rsdata){
		free(bthumb_rsdata);
		bthumb_rsdata = NULL;
	}

	search_results("", 0);

	free(v_buf);
	return 0;
}

int report_ui(pixel* vid_buf, char *save_id)
{
	int b=1,bq,mx,my;
	ui_edit ed;
	ui_edit_init(&ed, 209, 159, (XRES+BARSIZE-400)-18, (YRES+MENUSIZE-300)-36);
	ed.def = "Report details";
	ed.focus = 0;
	ed.multiline = 1;

	fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
	while (!sdl_poll()) {
		fillrect(vid_buf, 200, 150, (XRES+BARSIZE-400), (YRES+MENUSIZE-300), 0,0,0, 255);
		drawrect(vid_buf, 200, 150, (XRES+BARSIZE-400), (YRES+MENUSIZE-300), 255, 255, 255, 255);

		drawrect(vid_buf, 205, 155, (XRES+BARSIZE-400)-10, (YRES+MENUSIZE-300)-28, 255, 255, 255, 170);

		bq = b;
		b = mouse_get_state(&mx, &my);


		drawrect(vid_buf, 200, (YRES+MENUSIZE-150)-18, 50, 18, 255, 255, 255, 255);
		drawtext(vid_buf, 213, (YRES+MENUSIZE-150)-13, "Cancel", 255, 255, 255, 255);

		drawrect(vid_buf, (XRES+BARSIZE-400)+150, (YRES+MENUSIZE-150)-18, 50, 18, 255, 255, 255, 255);
		drawtext(vid_buf, (XRES+BARSIZE-400)+163, (YRES+MENUSIZE-150)-13, "Report", 255, 255, 255, 255);
		if (mx>(XRES+BARSIZE-400)+150 && my>(YRES+MENUSIZE-150)-18 && mx<(XRES+BARSIZE-400)+200 && my<(YRES+MENUSIZE-150)) {
			fillrect(vid_buf, (XRES+BARSIZE-400)+150, (YRES+MENUSIZE-150)-18, 50, 18, 255, 255, 255, 40);
			if (b) {
				if (execute_report(vid_buf, save_id, ed.str)) {
					info_ui(vid_buf, "Success", "This save has been reported");
					return 1;
				} else {
					return 0;
				}
			}
		}
		if (mx>200 && my>(YRES+MENUSIZE-150)-18 && mx<250 && my<(YRES+MENUSIZE-150)) {
			fillrect(vid_buf, 200, (YRES+MENUSIZE-150)-18, 50, 18, 255, 255, 255, 40);
			if (b)
				return 0;
		}
		ui_edit_draw(vid_buf, &ed);
#ifdef OGLR
		clearScreen(1.0f);
#endif
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
		ui_edit_process(mx, my, b, bq, &ed);
	}
	return 0;
}

void converttotime(char *timestamp, char **timestring, int show_day, int show_year, int show_time)
{
	int curr_tm_year, curr_tm_yday;
	char *tempstring = (char*)calloc(63,sizeof(char*));
	struct tm * stamptime, * currtime;
	time_t stamptime2 = atoi(timestamp), currtime2 = time(NULL);
	currtime = localtime(&currtime2);
	curr_tm_year = currtime->tm_year; curr_tm_yday = currtime->tm_yday;
	stamptime = localtime(&stamptime2);
	*timestring = (char*)calloc(63,sizeof(char*));

	if (show_day == 1 || show_day != 0 && (stamptime->tm_yday != curr_tm_yday || stamptime->tm_year != curr_tm_year)) //Different day or year, show date
	{
		if (dateformat%6 < 3) //Show weekday
		{
			sprintf(tempstring,asctime(stamptime));
			tempstring[4] = 0;
			strappend(*timestring,tempstring);
		}
		if (dateformat%3 == 1) // MM/DD/YY
		{
			sprintf(tempstring,"%i/%i",stamptime->tm_mon+1,stamptime->tm_mday);

		}
		else if (dateformat%3 == 2) // DD/MM/YY
		{
			sprintf(tempstring,"%i/%i",stamptime->tm_mday,stamptime->tm_mon+1);
		}
		else //Ex. Sun Jul 4
		{
			//sprintf(tempstring,asctime(stamptime));
			//tempstring[7] = 0;
			strncpy(tempstring,asctime(stamptime)+4,4);
			strappend(*timestring,tempstring);
			sprintf(tempstring,"%i",stamptime->tm_mday);
		}
		strappend(*timestring,tempstring);
	}
	if (show_year == 1 || show_year != 0 && stamptime->tm_year != curr_tm_year) //Show year
	{
		if (dateformat%3 != 0)
		{
			sprintf(tempstring,"/%i",(stamptime->tm_year+1900)%100);
			strappend(*timestring,tempstring);
		}
		else
		{
			sprintf(tempstring," %i",stamptime->tm_year+1900);
			strappend(*timestring,tempstring);
		}
	}
	if (show_time == 1 || show_time != 0 && (dateformat < 6 || (stamptime->tm_yday == curr_tm_yday && stamptime->tm_year == curr_tm_year))) //Show time
	{
		int hour = stamptime->tm_hour%12;
		char *ampm = "AM";
		if (stamptime->tm_hour > 11)
			ampm = "PM";
		if (hour == 0)
			hour = 12;
		sprintf(tempstring,"%s%i:%.2i:%.2i %s",strlen(*timestring)==0?"":" ",hour,stamptime->tm_min,stamptime->tm_sec,ampm);
		strappend(*timestring,tempstring);
	}
	free(tempstring);
	//strncpy(*timestring, asctime(stamptime), 63);
}

int open_ui(pixel *vid_buf, char *save_id, char *save_date, int instant_open)
{
	int b=1,bq,mx,my,ca=0,thumb_w,thumb_h,active=0,active_2=0,active_3=0,active_4=0,cc=0,ccy=0,cix=0;
	int hasdrawninfo=0,hasdrawncthumb=0,hasdrawnthumb=0,authoritah=0,myown=0,queue_open=0,data_size=0,full_thumb_data_size=0,retval=0,bc=255,openable=1;
	int comment_scroll = 0, comment_page = 0, redraw_comments = 1, dofocus = 0, disable_scrolling = 0;
	int nyd,nyu,ry,lv;
	float ryf, scroll_velocity = 0.0f;

	char *uri, *uri_2, *o_uri, *uri_3, *uri_4;
	void *data = NULL, *info_data, *thumb_data_full, *comment_data;
	save_info *info = (save_info*)calloc(sizeof(save_info), 1);
	void *http = NULL, *http_2 = NULL, *http_3 = NULL, *http_4 = NULL;
	int lasttime = TIMEOUT, saveTotal, saveDone, infoTotal, infoDone, downloadDone, downloadTotal;
	int status, status_2, status_4, info_ready = 0, data_ready = 0, thumb_data_ready = 0;
	time_t http_last_use = HTTP_TIMEOUT,  http_last_use_2 = HTTP_TIMEOUT,  http_last_use_3 = HTTP_TIMEOUT,  http_last_use_4 = HTTP_TIMEOUT;
	pixel *save_pic;// = malloc((XRES/2)*(YRES/2));
	pixel *save_pic_thumb = NULL;
	char *thumb_data = NULL;
	char viewcountbuffer[11];
	int thumb_data_size = 0;
	ui_edit ed;
	ui_copytext ctb;

	pixel *old_vid=(pixel *)calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	if (!old_vid || !info)
		return 0;
	fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
	viewcountbuffer[0] = 0;

	fillrect(vid_buf, 50, 50, XRES+BARSIZE-100, YRES+MENUSIZE-100, 0, 0, 0, 255);
	drawrect(vid_buf, 50, 50, XRES+BARSIZE-100, YRES+MENUSIZE-100, 255, 255, 255, 255);
	drawrect(vid_buf, 50, 50, (XRES/2)+1, (YRES/2)+1, 255, 255, 255, 155);
	drawrect(vid_buf, 50+(XRES/2)+1, 50, XRES+BARSIZE-100-((XRES/2)+1), YRES+MENUSIZE-100, 155, 155, 155, 255);
	drawtext(vid_buf, 50+(XRES/4)-textwidth("Loading...")/2, 50+(YRES/4), "Loading...", 255, 255, 255, 128);

	ui_edit_init(&ed, 57+(XRES/2)+1, YRES+MENUSIZE-83, XRES+BARSIZE-114-((XRES/2)+1), 14);
	ed.def = "Add comment";
	ed.focus = svf_login?1:0;
	ed.multiline = 1;
	ed.resizable = 1;
	ed.limit = 1023;

	ctb.x = 100;
	ctb.y = YRES+MENUSIZE-20;
	ctb.width = textwidth(save_id)+12;
	ctb.height = 10+7;
	ctb.hover = 0;
	ctb.state = 0;
	strcpy(ctb.text, save_id);

	memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
	
	//Try to load the thumbnail from the cache
	if(save_date)
	{
		char * id_d_temp = (char*)malloc(strlen(save_id)+strlen(save_date)+2);
		strcpy(id_d_temp, save_id);
		strappend(id_d_temp, "_");
		strappend(id_d_temp, save_date);
		
		status = thumb_cache_find(id_d_temp, (void**)(&thumb_data), &thumb_data_size);
		free(id_d_temp);
	}
	else
	{
		status = thumb_cache_find(save_id, (void**)(&thumb_data), &thumb_data_size);
	}
	if(!status){
		thumb_data = NULL;	
	} else {
		//We found a thumbnail in the cache, we'll draw this one while we wait for the full image to load.
		int finw, finh;
		pixel *thumb_imgdata = ptif_unpack(thumb_data, thumb_data_size, &finw, &finh);
		if(thumb_imgdata!=NULL){
			save_pic_thumb = resample_img(thumb_imgdata, finw, finh, XRES/2, YRES/2);
			//draw_image(vid_buf, save_pic_thumb, 51, 51, XRES/2, YRES/2, 255);	
		}
		free(thumb_imgdata);
		//rescale_img(full_save, imgw, imgh, &thumb_w, &thumb_h, 2);
	}

	//Begin Async loading of data
	if (save_date) {
		// We're loading an historical save
		uri = (char*)malloc(strlen(save_id)*3+strlen(save_date)*3+strlen(STATICSERVER)+71);
		strcpy(uri, "http://" STATICSERVER "/");
		strcaturl(uri, save_id);
		strappend(uri, "_");
		strcaturl(uri, save_date);
		strappend(uri, ".cps");

		uri_2 = (char*)malloc(strlen(save_id)*3+strlen(save_date)*3+strlen(STATICSERVER)+71);
		strcpy(uri_2, "http://" STATICSERVER "/");
		strcaturl(uri_2, save_id);
		strappend(uri_2, "_");
		strcaturl(uri_2, save_date);
		strappend(uri_2, ".info");

		uri_3 = (char*)malloc(strlen(save_id)*3+strlen(save_date)*3+strlen(STATICSERVER)+71);
		strcpy(uri_3, "http://" STATICSERVER "/");
		strcaturl(uri_3, save_id);
		strappend(uri_3, "_");
		strcaturl(uri_3, save_date);
		strappend(uri_3, "_large.pti");
	} else {
		//We're loading a normal save
		uri = (char*)malloc(strlen(save_id)*3+strlen(STATICSERVER)+64);
		strcpy(uri, "http://" STATICSERVER "/");
		strcaturl(uri, save_id);
		strappend(uri, ".cps");

		uri_2 = (char*)malloc(strlen(save_id)*3+strlen(STATICSERVER)+64);
		strcpy(uri_2, "http://" STATICSERVER "/");
		strcaturl(uri_2, save_id);
		strappend(uri_2, ".info");

		uri_3 = (char*)malloc(strlen(save_id)*3+strlen(STATICSERVER)+64);
		strcpy(uri_3, "http://" STATICSERVER "/");
		strcaturl(uri_3, save_id);
		strappend(uri_3, "_large.pti");
	}
	uri_4 = (char*)malloc(strlen(save_id)*3+strlen(STATICSERVER)+64);
	strcpy(uri_4, "http://" SERVER "/Browse/Comments.json?ID=");
	strcaturl(uri_4, save_id);
	strappend(uri_4, "&Start=0&Count=20");

	http = http_async_req_start(http, uri, NULL, 0, 1);
	http_2 = http_async_req_start(http_2, uri_2, NULL, 0, 1);
	if (!instant_open)
	{
		http_3 = http_async_req_start(http_3, uri_3, NULL, 0, 1);
		http_4 = http_async_req_start(http_4, uri_4, NULL, 0, 1);
	}
	if (svf_login)
	{
		http_auth_headers(http, svf_user_id, NULL, svf_session_id);
		http_auth_headers(http_2, svf_user_id, NULL, svf_session_id);
	}
	http_last_use = time(NULL);
	http_last_use_2 = time(NULL);
	free(uri);
	free(uri_2);
	active = 1;
	active_2 = 1;
	if (!instant_open)
	{
		http_last_use_3 = time(NULL);
		free(uri_3);
		active_3 = 1;
		http_last_use_4 = time(NULL);
		free(uri_4);
		active_4 = 1;
	}
	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);
		if (b == 1)
			redraw_comments = 1;

		if (active)
		{
			http_async_get_length(http, &saveTotal, &saveDone);
		}
		if (active && http_async_req_status(http))
		{
			int imgh, imgw, nimgh, nimgw;
			http_last_use = time(NULL);
			data = http_async_req_stop(http, &status, &data_size);
			saveDone = data_size;
			saveTotal = data_size;
			if (status == 200)
			{
				pixel *full_save;
				if (!data||!data_size) {
					error_ui(vid_buf, 0, "Save data is empty (may be corrupt)");
					break;
				}
				full_save = prerender_save(data, data_size, &imgw, &imgh);
				if (full_save!=NULL) {
					//save_pic = rescale_img(full_save, imgw, imgh, &thumb_w, &thumb_h, 2);
					data_ready = 1;
					free(full_save);
				} else {
					error_ui(vid_buf, 0, "Save may be from a newer version");
					break;
				}
			}
			active = 0;
			free(http);
			http = NULL;
		}
		if (active_2)
		{
			http_async_get_length(http_2, &infoTotal, &infoDone);
		}
		if (active_2 && http_async_req_status(http_2))
		{
			http_last_use_2 = time(NULL);
			info_data = http_async_req_stop(http_2, &status_2, &infoTotal);
			infoDone = infoTotal;
			if (status_2 == 200 || !info_data)
			{
				info_ready = info_parse((char*)info_data, info);
				sprintf(viewcountbuffer, "%d", info->downloadcount);
				if (info_ready<=0) {
					error_ui(vid_buf, 0, "Save info not found");
					break;
				}
			}
			if (info_data)
				free(info_data);
			active_2 = 0;
			free(http_2);
			http_2 = NULL;
		}
		if (!instant_open && active_3 && http_async_req_status(http_3))
		{
			int imgh, imgw, nimgh, nimgw;
			http_last_use_3 = time(NULL);
			thumb_data_full = http_async_req_stop(http_3, &status, &full_thumb_data_size);
			if (status == 200)
			{
				pixel *full_thumb;
				if (!thumb_data_full||!full_thumb_data_size) {
					//error_ui(vid_buf, 0, "Save data is empty (may be corrupt)");
					//break;
				} else {
					full_thumb = ptif_unpack(thumb_data_full, full_thumb_data_size, &imgw, &imgh);//prerender_save(data, data_size, &imgw, &imgh);
					if (full_thumb!=NULL) {
						save_pic = resample_img(full_thumb, imgw, imgh, XRES/2, YRES/2);
						thumb_data_ready = 1;
						free(full_thumb);
					}
				}
			}
			if(thumb_data_full)
				free(thumb_data_full);
			active_3 = 0;
			free(http_3);
			http_3 = NULL;
		}
		if (!instant_open && active_4 && http_async_req_status(http_4) && info_ready)
		{
			http_last_use_4 = time(NULL);
			comment_data = http_async_req_stop(http_4, &status_4, NULL);
			if (status_4 == 200)
			{
				int i;
				cJSON *root, *commentobj, *tmpobj;
				for (i=comment_page*20;i<comment_page*20+20&&i<NUM_COMMENTS;i++)
				{
					if (info->comments[i].str) { info->comments[i].str[0] = 0; }
					if (info->commentauthors[i]) { free(info->commentauthors[i]); info->commentauthors[i] = NULL; }
					if (info->commentauthorsunformatted[i]) { free(info->commentauthorsunformatted[i]); info->commentauthorsunformatted[i] = NULL; }
					if (info->commentauthorIDs[i]) { free(info->commentauthorIDs[i]); info->commentauthorIDs[i] = NULL; }
					if (info->commenttimestamps[i]) { free(info->commenttimestamps[i]); info->commenttimestamps[i] = NULL; }
				}
				if(comment_data && (root = cJSON_Parse((const char*)comment_data)))
				{
					if (comment_page == 0)
						info->comment_count = cJSON_GetArraySize(root);
					else
						info->comment_count += cJSON_GetArraySize(root);
					if (info->comment_count > NUM_COMMENTS)
						info->comment_count = NUM_COMMENTS;
					for (i = comment_page*20; i < info->comment_count; i++)
					{
						commentobj = cJSON_GetArrayItem(root, i%20);
						if(commentobj){
							if((tmpobj = cJSON_GetObjectItem(commentobj, "FormattedUsername")) && tmpobj->type == cJSON_String) { info->commentauthors[i] = (char*)calloc(63,sizeof(char*)); strncpy(info->commentauthors[i], tmpobj->valuestring, 63); }
							if((tmpobj = cJSON_GetObjectItem(commentobj, "Username")) && tmpobj->type == cJSON_String) { info->commentauthorsunformatted[i] = (char*)calloc(63,sizeof(char*)); strncpy(info->commentauthorsunformatted[i], tmpobj->valuestring, 63); }
							if((tmpobj = cJSON_GetObjectItem(commentobj, "UserID")) && tmpobj->type == cJSON_String) { info->commentauthorIDs[i] = (char*)calloc(16,sizeof(char*)); strncpy(info->commentauthorIDs[i], tmpobj->valuestring, 16); }
							//if((tmpobj = cJSON_GetObjectItem(commentobj, "Gravatar")) && tmpobj->type == cJSON_String) { info->commentauthors[i] = (char*)calloc(63,sizeof(char*)); strncpy(info->commentauthors[i], tmpobj->valuestring, 63); }
							if((tmpobj = cJSON_GetObjectItem(commentobj, "Text")) && tmpobj->type == cJSON_String)  { strncpy(info->comments[i].str, tmpobj->valuestring, 1023); }
							if((tmpobj = cJSON_GetObjectItem(commentobj, "Timestamp")) && tmpobj->type == cJSON_String) { converttotime(tmpobj->valuestring, &info->commenttimestamps[i], -1, -1, -1); }
						}
					}
					cJSON_Delete(root);
					redraw_comments = 1;
				}
			}
			if (comment_data)
				free(comment_data);
			active_4 = 0;
			free(http_4);
			http_4 = NULL;
			disable_scrolling = 0;
		}
		if (!instant_open)
		{
			if (save_pic_thumb!=NULL && !hasdrawncthumb) {
				draw_image(vid_buf, save_pic_thumb, 51, 51, XRES/2, YRES/2, 255);
				free(save_pic_thumb);
				save_pic_thumb = NULL;
				hasdrawncthumb = 1;
				memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);
			}
			if (thumb_data_ready && !hasdrawnthumb) {
				draw_image(vid_buf, save_pic, 51, 51, XRES/2, YRES/2, 255);
				free(save_pic);
				save_pic = NULL;
				hasdrawnthumb = 1;
				memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);
			}
			if (info_ready && !hasdrawninfo) {
				//Render all the save information
				cix = drawtext(vid_buf, 60, (YRES/2)+60, info->name, 255, 255, 255, 255);
				cix = drawtext(vid_buf, 60, (YRES/2)+72, "Author:", 255, 255, 255, 155);
				cix = drawtext(vid_buf, cix+4, (YRES/2)+72, info->author, 255, 255, 255, 255);
				cix = drawtext(vid_buf, cix+4, (YRES/2)+72, "Date:", 255, 255, 255, 155);
				cix = drawtext(vid_buf, cix+4, (YRES/2)+72, info->date, 255, 255, 255, 255);
				if(info->downloadcount){
					drawtext(vid_buf, 48+(XRES/2)-textwidth(viewcountbuffer)-textwidth("Views:")-4, (YRES/2)+72, "Views:", 255, 255, 255, 155);
					drawtext(vid_buf, 48+(XRES/2)-textwidth(viewcountbuffer), (YRES/2)+72, viewcountbuffer, 255, 255, 255, 255);
				}
				drawtextwrap(vid_buf, 62, (YRES/2)+86, (XRES/2)-24, 0, info->description, 255, 255, 255, 200);

				//Draw the score bars
				if (info->voteup>0||info->votedown>0)
				{
					lv = (info->voteup>info->votedown)?info->voteup:info->votedown;
					lv = (lv>10)?lv:10;

					if (50>lv)
					{
						ryf = 50.0f/((float)lv);
						//if(lv<8)
						//{
						//	ry =  ry/(8-lv);
						//}
						nyu = (int)(info->voteup*ryf);
						nyd = (int)(info->votedown*ryf);
					}
					else
					{
						ryf = ((float)lv)/50.0f;
						nyu = (int)(info->voteup/ryf);
						nyd = (int)(info->votedown/ryf);
					}
					nyu = nyu>50?50:nyu;
					nyd = nyd>50?50:nyd;

					fillrect(vid_buf, 48+(XRES/2)-51, (YRES/2)+53, 52, 6, 0, 107, 10, 255);
					fillrect(vid_buf, 48+(XRES/2)-51, (YRES/2)+59, 52, 6, 107, 10, 0, 255);
					drawrect(vid_buf, 48+(XRES/2)-51, (YRES/2)+53, 52, 6, 128, 128, 128, 255);
					drawrect(vid_buf, 48+(XRES/2)-51, (YRES/2)+59, 52, 6, 128, 128, 128, 255);

					fillrect(vid_buf, 48+(XRES/2)-nyu, (YRES/2)+54, nyu, 4, 57, 187, 57, 255);
					fillrect(vid_buf, 48+(XRES/2)-nyd, (YRES/2)+60, nyd, 4, 187, 57, 57, 255);
				}

				hasdrawninfo = 1;
				myown = svf_login && !strcmp(info->author, svf_user);
				authoritah = svf_login && (!strcmp(info->author, svf_user) || svf_admin || svf_mod);
				memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);
			}
			if (info_ready)// && redraw_comments) // draw the comments
			{
				ccy = 0;
				info->comments[0].y = 72+comment_scroll;
				clearrect(vid_buf, 50+(XRES/2)+1, 50, XRES+BARSIZE-100-((XRES/2)+1), YRES+MENUSIZE-100);
				for (cc=0; cc<info->comment_count; cc++) {
					if (ccy + 72 + comment_scroll<YRES+MENUSIZE-56 && info->comments[cc].str) { //Try not to draw off the screen
						if (ccy+comment_scroll >= 0 && info->commentauthors[cc]) //Don't draw above the screen either
						{
							int r = 255, g = 255, bl = 255;
							if (!strcmp(info->commentauthors[cc], svf_user))
							{
								bl = 100;
							}
							else if (!strcmp(info->commentauthors[cc], info->author))
							{
								g = 100;
								bl = 100;
							}

							if (show_ids && info->commentauthorIDs[cc]) //Draw author id
							{
								drawtext(vid_buf, 265+(XRES/2)-textwidth(info->commentauthorIDs[cc]), ccy+60+comment_scroll, info->commentauthorIDs[cc], 255, 255, 0, 255);
								if (b && !bq && mx > 265+(XRES/2)-textwidth(info->commentauthorIDs[cc]) && mx < 265+(XRES/2) && my > ccy+58+comment_scroll && my < ccy+70+comment_scroll && my < YRES+MENUSIZE-76-ed.h+2)
									show_ids = 0;
							}
							else if (info->commenttimestamps[cc]) //, or draw timestamp
							{
								drawtext(vid_buf, 265+(XRES/2)-textwidth(info->commenttimestamps[cc]), ccy+60+comment_scroll, info->commenttimestamps[cc], 255, 255, 0, 255);
								if (b && !bq && mx > 265+(XRES/2)-textwidth(info->commenttimestamps[cc]) && mx < 265+(XRES/2) && my > ccy+58+comment_scroll && my < ccy+70+comment_scroll && my < YRES+MENUSIZE-76-ed.h+2)
									show_ids = 1;
							}
							drawtext(vid_buf, 61+(XRES/2), ccy+60+comment_scroll, info->commentauthors[cc], r, g, bl, 255); //Draw author

							if (b && !bq && mx > 61+(XRES/2) && mx < 61+(XRES/2)+textwidth(info->commentauthors[cc]) && my > ccy+58+comment_scroll && my < ccy+70+comment_scroll && my < YRES+MENUSIZE-76-ed.h+2)
								if (sdl_mod & KMOD_CTRL) //open profile
								{
									char link[128];
									strcpy(link, "http://" SERVER "/User.html?Name=");
									strcaturl(link, info->commentauthorsunformatted[cc]);
									open_link(link);
								}
								else if (sdl_mod & KMOD_SHIFT) //, or search for a user's saves
								{
									sprintf(search_expr,"user:%s", info->commentauthorsunformatted[cc]);
									search_own = 0;
									search_ui(vid_buf);
									retval = 0;
									goto finish;
								}
								else //copy name to comment box
								{
									if (strlen(ed.str) + strlen(info->commentauthorsunformatted[cc]) < 1023)
									{
										strappend(ed.str, info->commentauthorsunformatted[cc]);
										strappend(ed.str, ": ");
										dofocus = 1;
									}
								}
						}

						ccy += 12;
						if (ccy + 72 + comment_scroll<YRES+MENUSIZE-56) // Check again if the comment is off the screen, incase the author line made it too long
						{
							int change, commentboxy = YRES+MENUSIZE-70-ed.h+2-5;
							if (ccy+comment_scroll < 0) // if above screen set height to negative, how long until it can start being drawn
								info->comments[cc].maxHeight = ccy+comment_scroll-10;
							else                        // else set how much can be drawn until it goes off the screen
								info->comments[cc].maxHeight = YRES+MENUSIZE-41 - (ccy + 72 + comment_scroll);

							change = ui_label_draw(vid_buf, &info->comments[cc]); // draw the comment
							ui_label_process(mx, my, b, bq, &info->comments[cc]); // process copying

							if (svf_login && b && !bq && mx > 50+(XRES/2)+1 && mx < 50 + XRES+BARSIZE-100 && my > commentboxy - 2 && my < commentboxy + ed.h+2) // defocus comments that are under textbox
								info->comments[cc].focus = 0;

							ccy += change + 10;
							if (cc < NUM_COMMENTS-1)
								info->comments[cc+1].y = info->comments[cc].y + change + 22;

							if (ccy+comment_scroll < 100 && cc == info->comment_count-1 && active_4) // disable scrolling until more comments have loaded
								disable_scrolling = 1;
							if (ccy+comment_scroll < 0 && cc == info->comment_count-1 && !active_4) // reset to top of comments
							{
								comment_scroll = 0;
								scroll_velocity = 0.0f;
							}

							if (ccy+52+comment_scroll<YRES+MENUSIZE-50 && ccy+comment_scroll>-3) { //draw the line that separates comments
								draw_line(vid_buf, 50+(XRES/2)+2, ccy+52+comment_scroll, XRES+BARSIZE-51, ccy+52+comment_scroll, 100, 100, 100, XRES+BARSIZE);
							}
						}
					}
					else
						break;
					if (cc == info->comment_count-1 && !http_4 && comment_page < NUM_COMMENTS/10 && !(info->comment_count%20))
					{
						comment_page++;
						uri_4 = (char*)malloc(strlen(save_id)*3+strlen(STATICSERVER)+64);
						sprintf(uri_4,"http://%s/Browse/Comments.json?ID=%s&Start=%i&Count=20",SERVER,save_id,comment_page*20);
						http_4 = http_async_req_start(http_4, uri_4, NULL, 0, 1);
						http_last_use_4 = time(NULL);
						free(uri_4);
						active_4 = 1;
					}
				}
				memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);
				redraw_comments = 0;
			}
			if (info_ready && svf_login) {
				//Render the comment box.
				fillrect(vid_buf, 51+(XRES/2), ed.y-6, XRES+BARSIZE-100-((XRES/2)+1), ed.h+25, 0, 0, 0, 255);
				drawrect(vid_buf, 51+(XRES/2), ed.y-6, XRES+BARSIZE-100-((XRES/2)+1), ed.h+25, 200, 200, 200, 255);

				drawrect(vid_buf, 55+(XRES/2), ed.y-3, XRES+BARSIZE-108-((XRES/2)+1), ed.h, 255, 255, 255, 200);

				ed.y = YRES+MENUSIZE-71-ui_edit_draw(vid_buf, &ed);

				drawrect(vid_buf, XRES+BARSIZE-100, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, 255);
				drawtext(vid_buf, XRES+BARSIZE-90, YRES+MENUSIZE-63, "Submit", 255, 255, 255, 255);
			}

			//Save ID text and copybox
			cix = textwidth("Save ID: ");
			cix += ctb.width;
			ctb.x = textwidth("Save ID: ")+(XRES+BARSIZE-cix)/2;
			//ctb.x =
			drawtext(vid_buf, (XRES+BARSIZE-cix)/2, YRES+MENUSIZE-15, "Save ID: ", 255, 255, 255, 255);
			ui_copytext_draw(vid_buf, &ctb);
			ui_copytext_process(mx, my, b, bq, &ctb);

			//Open Button
			bc = openable?255:150;
			drawrect(vid_buf, 50, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, bc);
			drawtext(vid_buf, 73, YRES+MENUSIZE-63, "Open", 255, 255, 255, bc);
			drawtext(vid_buf, 58, YRES+MENUSIZE-64, "\x81", 255, 255, 255, bc);
			//Fav Button
			bc = svf_login?255:150;
			drawrect(vid_buf, 100, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, bc);
			if(info->myfav && svf_login){
				drawtext(vid_buf, 122, YRES+MENUSIZE-63, "Unfav.", 255, 230, 230, bc);
			} else {
				drawtext(vid_buf, 122, YRES+MENUSIZE-63, "Fav.", 255, 255, 255, bc);
			}
			drawtext(vid_buf, 107, YRES+MENUSIZE-64, "\xCC", 255, 255, 255, bc);
			//Report Button
			bc = (svf_login && info_ready)?255:150;
			drawrect(vid_buf, 150, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, bc);
			drawtext(vid_buf, 168, YRES+MENUSIZE-63, "Report", 255, 255, 255, bc);
			drawtext(vid_buf, 155, YRES+MENUSIZE-64, "\xE4", 255, 255, 255, bc);
			//Delete Button
			bc = authoritah?255:150;
			drawrect(vid_buf, 200, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, bc);
			drawtext(vid_buf, 218, YRES+MENUSIZE-63, "Delete", 255, 255, 255, bc);
			drawtext(vid_buf, 206, YRES+MENUSIZE-64, "\xAA", 255, 255, 255, bc);
			//Open in browser button
			bc = 255;
			drawrect(vid_buf, 250, YRES+MENUSIZE-68, 107, 18, 255, 255, 255, bc);
			drawtext(vid_buf, 273, YRES+MENUSIZE-63, "Open in Browser", 255, 255, 255, bc);
			drawtext(vid_buf, 258, YRES+MENUSIZE-64, "\x81", 255, 255, 255, bc);

			//Open Button
			//if (sdl_key==SDLK_RETURN && openable) {
			//	queue_open = 1;
			//}
			if (mx > 50 && mx < 50+50 && my > YRES+MENUSIZE-68 && my < YRES+MENUSIZE-50 && openable && !queue_open) {
				fillrect(vid_buf, 50, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, 40);
				if (b && !bq) {
					//Button Clicked
					queue_open = 1;
				}
			}
			//Fav Button
			if (mx > 100 && mx < 100+50 && my > YRES+MENUSIZE-68 && my < YRES+MENUSIZE-50 && svf_login && !queue_open) {
				fillrect(vid_buf, 100, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, 40);
				if (b && !bq) {
					//Button Clicked
					if(info->myfav){
						fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
						info_box(vid_buf, "Removing from favourites...");
						execute_unfav(vid_buf, save_id);
						info->myfav = 0;
					} else {
						fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
						info_box(vid_buf, "Adding to favourites...");
						execute_fav(vid_buf, save_id);
						info->myfav = 1;
					}
				}
			}
			//Report Button
			if (mx > 150 && mx < 150+50 && my > YRES+MENUSIZE-68 && my < YRES+MENUSIZE-50 && svf_login && info_ready && !queue_open) {
				fillrect(vid_buf, 150, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, 40);
				if (b && !bq) {
					//Button Clicked
					if (report_ui(vid_buf, save_id)) {
						retval = 0;
						break;
					}
				}
			}
			//Delete Button
			if (mx > 200 && mx < 200+50 && my > YRES+MENUSIZE-68 && my < YRES+MENUSIZE-50 && (authoritah || myown) && !queue_open) {
				fillrect(vid_buf, 200, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, 40);
				if (b && !bq) {
					//Button Clicked
					if (myown || !info->publish) {
						if (confirm_ui(vid_buf, "Are you sure you wish to delete this?", "You will not be able recover it.", "Delete")) {
							fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
							info_box(vid_buf, "Deleting...");
							if (execute_delete(vid_buf, save_id)) {
								retval = 0;
								break;
							}
						}
					} else {
						if (confirm_ui(vid_buf, "Are you sure?", "This save will be removed from the search index.", "Remove")) {
							fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
							info_box(vid_buf, "Removing...");
							if (execute_delete(vid_buf, save_id)) {
								retval = 0;
								break;
							}
						}
					}
				}
			}
			//Open in browser button
			if (mx > 250 && mx < 250+107 && my > YRES+MENUSIZE-68 && my < YRES+MENUSIZE-50  && !queue_open) {
				fillrect(vid_buf, 250, YRES+MENUSIZE-68, 107, 18, 255, 255, 255, 40);
				if (b && !bq) {
					//Button Clicked
					o_uri = (char*)malloc(7+strlen(SERVER)+41+strlen(save_id)*3);
					strcpy(o_uri, "http://" SERVER "/Browse/View.html?ID=");
					strcaturl(o_uri, save_id);
					open_link(o_uri);
					free(o_uri);
				}
			}
			//Submit Button
			if (mx > XRES+BARSIZE-100 && mx < XRES+BARSIZE-100+50 && my > YRES+MENUSIZE-68 && my < YRES+MENUSIZE-50 && svf_login && info_ready && !queue_open) {
				fillrect(vid_buf, XRES+BARSIZE-100, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, 40+active_4*80);
				if (b && !bq) {
					//Button Clicked
					fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
					info_box(vid_buf, "Submitting Comment...");
					if (!active_4 && !execute_submit(vid_buf, save_id, ed.str))
					{
						int i;
						ed.str[0] = 0;
						uri_4 = (char*)malloc(strlen(save_id)*3+strlen(STATICSERVER)+64);
						sprintf(uri_4,"http://%s/Browse/Comments.json?ID=%s&Start=%i&Count=20",SERVER,save_id,0);
						http_4 = http_async_req_start(http_4, uri_4, NULL, 0, 1);
						http_last_use_4 = time(NULL);
						free(uri_4);
						active_4 = 1;

						for (i=0; i < NUM_COMMENTS; i++)
						{
							if (info->comments[i].str) { info->comments[i].str[0] = 0; }
							if (info->commentauthors[i]) { free(info->commentauthors[i]); info->commentauthors[i] = NULL; }
							if (info->commentauthorsunformatted[i]) { free(info->commentauthorsunformatted[i]); info->commentauthorsunformatted[i] = NULL; }
							if (info->commentauthorIDs[i]) { free(info->commentauthorIDs[i]); info->commentauthorIDs[i] = NULL; }
							if (info->commenttimestamps[i]) { free(info->commenttimestamps[i]); info->commenttimestamps[i] = NULL; }
						}
						comment_page = 0;
						info->comment_count = 0;
						comment_scroll = 0;
						scroll_velocity = 0.0f;
					}
				}
			}
			if (scroll_velocity)
			{
				comment_scroll += (int)scroll_velocity;
				if (comment_scroll > 0)
					comment_scroll = 0;
				scroll_velocity *= .95f;
				if (abs(scroll_velocity) < .5f)
					scroll_velocity = 0.0f;
			}
			if (sdl_wheel && (!ed.focus || (sdl_key != '-' && sdl_key != '+')))
			{
				if (!disable_scrolling || sdl_wheel > 0)
				{
					comment_scroll += 6*sdl_wheel;
					scroll_velocity += sdl_wheel;
					if (comment_scroll > 0)
						comment_scroll = 0;
					redraw_comments = 1;
				}
			}
			if (sdl_key=='[' && !ed.focus)
			{
				comment_scroll += 10;
				if (comment_scroll > 0)
					comment_scroll = 0;
				redraw_comments = 1;
			}
			if (sdl_key==']' && !ed.focus && !disable_scrolling)
			{
				comment_scroll -= 10;
				redraw_comments = 1;
			}
			//If mouse was clicked outside of the window bounds.
			if (!(mx>50 && my>50 && mx<XRES+BARSIZE-50 && my<YRES+MENUSIZE-50) && !(mx >= ctb.x && mx <= ctb.x+ctb.width && my >= ctb.y && my <= ctb.y+ctb.height) && b && !bq && !queue_open) {
				retval = 0;
				break;
			}
		}

		//Download completion
		downloadTotal = saveTotal+infoTotal;
		downloadDone = saveDone+infoDone;
		if(downloadTotal>downloadDone)
		{
			clearrect(vid_buf, 51, (YRES/2)+37, (XRES)/2, 14);
			fillrect(vid_buf, 51, (YRES/2)+38, (int)((((float)XRES-2)/2.0f)*((float)downloadDone/(float)downloadTotal)), 12, 255, 200, 0, 255);
			if(((float)downloadDone/(float)downloadTotal)>0.5f)
				drawtext(vid_buf, 51+(((XRES/2)-textwidth("Downloading"))/2), (YRES/2)+40, "Downloading", 0, 0, 0, 255);
			else
				drawtext(vid_buf, 51+(((XRES/2)-textwidth("Downloading"))/2), (YRES/2)+40, "Downloading", 255, 255, 255, 255);
		}

		//User opened the save, wait until we've got all the data first...
		if (queue_open || instant_open) {
			if (info_ready && data_ready) {
				// Do Open!
				status = parse_save(data, data_size, 1, 0, 0, bmap, vx, vy, pv, fvx, fvy, signs, parts, pmap);
				if (!status) {
					if(svf_last)
						free(svf_last);
					svf_last = data;
					data = NULL; //so we don't free it when returning
					svf_lsize = data_size;

					svf_open = 1;
					svf_own = svf_login && !strcmp(info->author, svf_user);
					svf_publish = info->publish && svf_login && !strcmp(info->author, svf_user);

					strcpy(svf_id, save_id);
					strcpy(svf_name, info->name);
					strcpy(svf_description, info->description);
					strncpy(svf_author, info->author, 63);
					if (info->tags)
					{
						strncpy(svf_tags, info->tags, 255);
						svf_tags[255] = 0;
					} else {
						svf_tags[0] = 0;
					}
					svf_myvote = info->myvote;
					svf_filename[0] = 0;
					svf_fileopen = 0;
					retval = 1;
					ctrlzSnapshot();
					break;
				} else {
					queue_open = 0;

					svf_open = 0;
					svf_filename[0] = 0;
					svf_fileopen = 0;
					svf_modsave = 0;
					svf_publish = 0;
					svf_own = 0;
					svf_myvote = 0;
					svf_id[0] = 0;
					svf_name[0] = 0;
					svf_description[0] = 0;
					svf_author[0] = 0;
					svf_tags[0] = 0;
					if (svf_last)
						free(svf_last);
					svf_last = NULL;
					error_ui(vid_buf, 0, "An Error Occurred");
				}
			} else {
				fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 190);
				drawtext(vid_buf, 50+(XRES/4)-textwidth("Loading...")/2, 50+(YRES/4), "Loading...", 255, 255, 255, 128);
			}
		}
		if (!info_ready || !data_ready) {
			info_box(vid_buf, "Loading");
		}
#ifdef OGLR
		clearScreen(1.0f);
#endif
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
		memcpy(vid_buf, old_vid, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);
		if (info_ready && svf_login) {
			ui_edit_process(mx, my, b, bq, &ed);
		}
		if (dofocus)
		{
			ed.focus = 1;
			ed.cursor = ed.cursorstart = strlen(ed.str);
			dofocus = 0;
		}

		if (sdl_key==SDLK_ESCAPE) {
			retval = 0;
			break;
		}

		if (lasttime<TIMEOUT)
			lasttime++;
	}
	//Prevent those mouse clicks being passed down.
	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	finish:
	//Close open connections
	if (http)
	{
		http_hackyclosefreezefix(http);
		http_async_req_close(http);
	}
	if (http_2)
	{
		http_hackyclosefreezefix(http_2);
		http_async_req_close(http_2);
	}
	if (!instant_open)
	{
		if (http_3)
		{
			http_hackyclosefreezefix(http_3);
			http_async_req_close(http_3);
		}
		if (http_4)
		{
			http_hackyclosefreezefix(http_4);
			http_async_req_close(http_4);
		}
	}
	info_parse("", info);
	free(info);
	free(old_vid);
	if (data) free(data);
	if (thumb_data) free(thumb_data);
	return retval;
}

int info_parse(char *info_data, save_info *info)
{
	int i,j;
	char *p,*q,*r,*s,*vu,*vd,*pu,*sd;

	if (info->title) free(info->title);
	if (info->name) free(info->name);
	if (info->author) free(info->author);
	if (info->date) free(info->date);
	if (info->description) free(info->description);
	if (info->tags) free(info->tags);
	for (i=0;i<NUM_COMMENTS;i++)
	{
		if (info->commentauthors[i]) free(info->commentauthors[i]);
		if (info->commentauthorsunformatted[i]) free(info->commentauthorsunformatted[i]);
		if (info->commentauthorIDs[i]) free(info->commentauthorIDs[i]);
		if (info->commenttimestamps[i]) free(info->commenttimestamps[i]);
	}
	memset(info, 0, sizeof(save_info));
	for (i = 0; i < NUM_COMMENTS; i++)
	{
		ui_label_init(&info->comments[i], 61+(XRES/2), 0, XRES+BARSIZE-107-(XRES/2), 0);
	}

	if (!info_data || !*info_data)
		return 0;

	i = 0;
	j = 0;
	s = NULL;
	do_open = 0;
	while (1)
	{
		if (!*info_data)
			break;
		p = strchr(info_data, '\n');
		if (!p)
			p = info_data + strlen(info_data);
		else
			*(p++) = 0;

		if (!strncmp(info_data, "TITLE ", 6))
		{
			info->title = mystrdup(info_data+6);
			j++;
		}
		else if (!strncmp(info_data, "NAME ", 5))
		{
			info->name = mystrdup(info_data+5);
			j++;
		}
		else if (!strncmp(info_data, "AUTHOR ", 7))
		{
			info->author = mystrdup(info_data+7);
			j++;
		}
		else if (!strncmp(info_data, "DATE ", 5))
		{
			info->date = mystrdup(info_data+5);
			j++;
		}
		else if (!strncmp(info_data, "DESCRIPTION ", 12))
		{
			info->description = mystrdup(info_data+12);
			j++;
		}
		else if (!strncmp(info_data, "VOTEUP ", 7))
		{
			info->voteup = atoi(info_data+7);
			j++;
		}
		else if (!strncmp(info_data, "VOTEDOWN ", 9))
		{
			info->votedown = atoi(info_data+9);
			j++;
		}
		else if (!strncmp(info_data, "VOTE ", 5))
		{
			info->vote = atoi(info_data+5);
			j++;
		}
		else if (!strncmp(info_data, "MYVOTE ", 7))
		{
			info->myvote = atoi(info_data+7);
			j++;
		}
		else if (!strncmp(info_data, "DOWNLOADS ", 10))
		{
			info->downloadcount = atoi(info_data+10);
			j++;
		}
		else if (!strncmp(info_data, "MYFAV ", 6))
		{
			info->myfav = atoi(info_data+6);
			j++;
		}
		else if (!strncmp(info_data, "PUBLISH ", 8))
		{
			info->publish = atoi(info_data+8);
			j++;
		}
		else if (!strncmp(info_data, "TAGS ", 5))
		{
			info->tags = mystrdup(info_data+5);
			j++;
		}
		else if (!strncmp(info_data, "COMMENT ", 8))
		{
			if (info->comment_count>=NUM_COMMENTS) {
				info_data = p;
				continue;
			} else {
				q = strchr(info_data+8, ' ');
				*(q++) = 0;
				info->commentauthors[info->comment_count] = mystrdup(info_data+8);
				info->commentauthorsunformatted[info->comment_count] = mystrdup(info_data+8);
				strncpy(info->comments[info->comment_count].str,mystrdup(q), 1023);
				info->comment_count++;
			}
			j++;
		}
		info_data = p;
	}
	if (j>=8) {
		return 1;
	} else {
		return -1;
	}
}

int search_results(char *str, int votes)
{
	int i,j;
	char *p,*q,*r,*s,*vu,*vd,*pu,*sd;

	for (i=0; i<GRID_X*GRID_Y; i++)
	{
		if (search_ids[i])
		{
			free(search_ids[i]);
			search_ids[i] = NULL;
		}
		if (search_names[i])
		{
			free(search_names[i]);
			search_names[i] = NULL;
		}
		if (search_dates[i])
		{
			free(search_dates[i]);
			search_dates[i] = NULL;
		}
		if (search_owners[i])
		{
			free(search_owners[i]);
			search_owners[i] = NULL;
		}
		if (search_thumbs[i])
		{
			free(search_thumbs[i]);
			search_thumbs[i] = NULL;
			search_thsizes[i] = 0;
		}
	}
	for (j=0; j<TAG_MAX; j++)
		if (tag_names[j])
		{
			free(tag_names[j]);
			tag_names[j] = NULL;
		}
	server_motd[0] = 0;

	if (!str || !*str)
		return 0;

	i = 0;
	j = 0;
	s = NULL;
	do_open = 0;
	while (1)
	{
		if (!*str)
			break;
		p = strchr(str, '\n');
		if (!p)
			p = str + strlen(str);
		else
			*(p++) = 0;
		if (!strncmp(str, "OPEN ", 5))
		{
			do_open = 1;
			if (i>=GRID_X*GRID_Y)
				break;
			if (votes)
			{
				pu = strchr(str+5, ' ');
				if (!pu)
					return i;
				*(pu++) = 0;
				s = strchr(pu, ' ');
				if (!s)
					return i;
				*(s++) = 0;
				vu = strchr(s, ' ');
				if (!vu)
					return i;
				*(vu++) = 0;
				vd = strchr(vu, ' ');
				if (!vd)
					return i;
				*(vd++) = 0;
				q = strchr(vd, ' ');
			}
			else
			{
				pu = strchr(str+5, ' ');
				if (!pu)
					return i;
				*(pu++) = 0;
				vu = strchr(pu, ' ');
				if (!vu)
					return i;
				*(vu++) = 0;
				vd = strchr(vu, ' ');
				if (!vd)
					return i;
				*(vd++) = 0;
				q = strchr(vd, ' ');
			}
			if (!q)
				return i;
			*(q++) = 0;
			r = strchr(q, ' ');
			if (!r)
				return i;
			*(r++) = 0;
			search_ids[i] = mystrdup(str+5);

			search_publish[i] = atoi(pu);
			search_scoreup[i] = atoi(vu);
			search_scoredown[i] = atoi(vd);

			search_owners[i] = mystrdup(q);
			search_names[i] = mystrdup(r);

			if (s)
				search_votes[i] = atoi(s);
			thumb_cache_find(str+5, search_thumbs+i, search_thsizes+i);
			i++;
		}
		else if (!strncmp(str, "HISTORY ", 8))
		{
			char * id_d_temp = NULL;
			if (i>=GRID_X*GRID_Y)
				break;
			if (votes)
			{
				sd = strchr(str+8, ' ');
				if (!sd)
					return i;
				*(sd++) = 0;
				pu = strchr(sd, ' ');
				if (!pu)
					return i;
				*(pu++) = 0;
				s = strchr(pu, ' ');
				if (!s)
					return i;
				*(s++) = 0;
				vu = strchr(s, ' ');
				if (!vu)
					return i;
				*(vu++) = 0;
				vd = strchr(vu, ' ');
				if (!vd)
					return i;
				*(vd++) = 0;
				q = strchr(vd, ' ');
			}
			else
			{
				sd = strchr(str+8, ' ');
				if (!sd)
					return i;
				*(sd++) = 0;
				pu = strchr(sd, ' ');
				if (!pu)
					return i;
				*(pu++) = 0;
				vu = strchr(pu, ' ');
				if (!vu)
					return i;
				*(vu++) = 0;
				vd = strchr(vu, ' ');
				if (!vd)
					return i;
				*(vd++) = 0;
				q = strchr(vd, ' ');
			}
			if (!q)
				return i;
			*(q++) = 0;
			r = strchr(q, ' ');
			if (!r)
				return i;
			*(r++) = 0;
			search_ids[i] = mystrdup(str+8);

			search_dates[i] = mystrdup(sd);

			search_publish[i] = atoi(pu);
			search_scoreup[i] = atoi(vu);
			search_scoredown[i] = atoi(vd);

			search_owners[i] = mystrdup(q);
			search_names[i] = mystrdup(r);

			if (s)
				search_votes[i] = atoi(s);
				
			//Build thumb cache ID and find
			id_d_temp = (char*)malloc(strlen(search_ids[i])+strlen(search_dates[i])+2);
			strcpy(id_d_temp, search_ids[i]);
			strappend(id_d_temp, "_");
			strappend(id_d_temp, search_dates[i]);
			thumb_cache_find(id_d_temp, search_thumbs+i, search_thsizes+i);
			free(id_d_temp);
			
			i++;
		}
		else if (!strncmp(str, "MOTD ", 5))
		{
			strncpy(server_motd, str+5, 511);
		}
		else if (!strncmp(str, "TAG ", 4))
		{
			if (j >= TAG_MAX)
			{
				str = p;
				continue;
			}
			q = strchr(str+4, ' ');
			if (!q)
			{
				str = p;
				continue;
			}
			*(q++) = 0;
			tag_names[j] = mystrdup(str+4);
			tag_votes[j] = atoi(q);
			j++;
		}
		else
		{
			if (i>=GRID_X*GRID_Y)
				break;
			if (votes)
			{
				pu = strchr(str, ' ');
				if (!pu)
					return i;
				*(pu++) = 0;
				s = strchr(pu, ' ');
				if (!s)
					return i;
				*(s++) = 0;
				vu = strchr(s, ' ');
				if (!vu)
					return i;
				*(vu++) = 0;
				vd = strchr(vu, ' ');
				if (!vd)
					return i;
				*(vd++) = 0;
				q = strchr(vd, ' ');
			}
			else
			{
				pu = strchr(str, ' ');
				if (!pu)
					return i;
				*(pu++) = 0;
				vu = strchr(pu, ' ');
				if (!vu)
					return i;
				*(vu++) = 0;
				vd = strchr(vu, ' ');
				if (!vd)
					return i;
				*(vd++) = 0;
				q = strchr(vd, ' ');
			}
			if (!q)
				return i;
			*(q++) = 0;
			r = strchr(q, ' ');
			if (!r)
				return i;
			*(r++) = 0;
			search_ids[i] = mystrdup(str);

			search_publish[i] = atoi(pu);
			search_scoreup[i] = atoi(vu);
			search_scoredown[i] = atoi(vd);

			search_owners[i] = mystrdup(q);
			search_names[i] = mystrdup(r);

			if (s)
				search_votes[i] = atoi(s);
			thumb_cache_find(str, search_thumbs+i, search_thsizes+i);
			i++;
		}
		str = p;
	}
	if (*str)
		i++;
	return i;
}

int execute_tagop(pixel *vid_buf, char *op, char *tag)
{
	int status;
	char *result;

	char *names[] = {"ID", "Tag", NULL};
	char *parts[2];

	char *uri = (char*)malloc(strlen(SERVER)+strlen(op)+36);
	sprintf(uri, "http://" SERVER "/Tag.api?Op=%s", op);

	parts[0] = svf_id;
	parts[1] = tag;

	result = http_multipart_post(
	             uri,
	             names, parts, NULL,
	             svf_user_id, /*svf_pass*/NULL, svf_session_id,
	             &status, NULL);

	free(uri);

	if (status!=200)
	{
		error_ui(vid_buf, status, http_ret_text(status));
		if (result)
			free(result);
		return 1;
	}
	if (result && strncmp(result, "OK", 2))
	{
		error_ui(vid_buf, 0, result);
		free(result);
		return 1;
	}

	if (result && result[2])
	{
		strncpy(svf_tags, result+3, 255);
		svf_id[15] = 0;
	}

	if (result)
		free(result);
	else
		error_ui(vid_buf, 0, "Could not add tag");

	return 0;
}

int execute_save(pixel *vid_buf)
{
	int status, oldsave_as = save_as;
	char *result;

	char *names[] = {"Name","Description", "Data:save.bin", "Thumb:thumb.bin", "Publish", "ID", NULL};
	char *uploadparts[6];
	int plens[6];

	/*if (save_as == 3 && !check_save(2,0,0,XRES,YRES,0))
	{
		if (confirm_ui(vid_buf, "Save as official version?", "This save doesn't use any mod elements, and could be opened by everyone and have a correct thumbnail if it was saved as the official version. Click cancel to save like this anyway, incase you used some mod features", "OK"))
		{
			oldsave_as = save_as;
			save_as = 5;
		}
	}*/
	if (save_as != 4 && !svf_modsave)
	{
		oldsave_as = save_as;
		save_as = 5;
	}
	else if (svf_modsave)
	{
		oldsave_as = save_as;
		save_as = 3;
	}
	if (svf_publish == 1 && save_as != 5)
	{
		error_ui(vid_buf, 0, "You must save this as the non beta version");
		return 1;
	}
	if (svf_publish == 1 && check_save(2,0,0,XRES,YRES,1))
		return 1;

	uploadparts[0] = svf_name;
	plens[0] = strlen(svf_name);
	uploadparts[1] = svf_description;
	plens[1] = strlen(svf_description);
	uploadparts[2] = (char*)build_save(plens+2, 0, 0, XRES, YRES, bmap, vx, vy, pv, fvx, fvy, signs, parts, (save_as == 3 && (sdl_mod & KMOD_SHIFT)));
	if (!uploadparts[2])
	{
		error_ui(vid_buf, 0, "Error creating save");
		return 1;
	}
	uploadparts[3] = (char*)build_thumb(plens+3, 1);
	uploadparts[4] = (char*)((svf_publish==1)?"Public":"Private");
	plens[4] = strlen((svf_publish==1)?"Public":"Private");

	if (svf_id[0])
	{
		uploadparts[5] = svf_id;
		plens[5] = strlen(svf_id);
	}
	else
		names[5] = NULL;

	result = http_multipart_post(
	             "http://" SERVER "/Save.api",
	             names, uploadparts, plens,
	             svf_user_id, /*svf_pass*/NULL, svf_session_id,
	             &status, NULL);

	if (svf_last)
		free(svf_last);
	svf_last = uploadparts[2];
	svf_lsize = plens[2];

	if (uploadparts[3])
		free(uploadparts[3]);

	save_as = oldsave_as;
	if (status!=200)
	{
		error_ui(vid_buf, status, http_ret_text(status));
		if (result)
			free(result);
		return 1;
	}
	if (!result || strncmp(result, "OK", 2))
	{
		if (!result)
			result = mystrdup("Could not save - no reply from server");
		error_ui(vid_buf, 0, result);
		free(result);
		return 1;
	}

	if (result && result[2])
	{
		strncpy(svf_id, result+3, 15);
		svf_id[15] = 0;
	}

	if (!svf_id[0])
	{
		error_ui(vid_buf, 0, "No ID supplied by server");
		free(result);
		return 1;
	}

	thumb_cache_inval(svf_id);

	svf_own = 1;
	if (result)
		free(result);
	return 0;
}

int execute_delete(pixel *vid_buf, char *id)
{
	int status;
	char *result;

	char *names[] = {"ID", NULL};
	char *parts[1];

	parts[0] = id;

	result = http_multipart_post(
	             "http://" SERVER "/Delete.api",
	             names, parts, NULL,
	             svf_user_id, /*svf_pass*/NULL, svf_session_id,
	             &status, NULL);

	if (status!=200)
	{
		error_ui(vid_buf, status, http_ret_text(status));
		if (result)
			free(result);
		return 0;
	}
	if (result && strncmp(result, "INFO: ", 6)==0)
	{
		info_ui(vid_buf, "Info", result+6);
		free(result);
		return 0;
	}
	if (result && strncmp(result, "OK", 2))
	{
		error_ui(vid_buf, 0, result);
		free(result);
		return 0;
	}

	if (result)
		free(result);
	return 1;
}

int execute_submit(pixel *vid_buf, char *id, char *message)
{
	int status;
	char *result;

	if (1)//strlen(svf_session_key))
	{
		char * postNames[] = { "Comment", NULL };
		char * postDatas[] = { message };
		int postLengths[] = { strlen(message) };
		int dataLength;
		char *url = (char*)malloc(sizeof(message)+sizeof(id) + 128);
		//sprintf(url, "http://%s/Browse/Comments.json?ID=%s&Key=%s", SERVER, id, svf_session_key);
		sprintf(url, "http://%s/Browse/Comments.json?ID=%s", SERVER, id);
		result = http_multipart_post(url, postNames, postDatas, postLengths, svf_user_id, NULL, svf_session_id, &status, &dataLength);

		if (status!=200)
		{
			error_ui(vid_buf, status, http_ret_text(status));
			if (result)
				free(result);
			return 1;
		}
		else
		{
			cJSON *root, *tmpobj;
			if (root = cJSON_Parse((const char*)result))
			{
				tmpobj = cJSON_GetObjectItem(root, "Status");
				if (tmpobj && tmpobj->type == cJSON_Number && tmpobj->valueint != 1)
				{
					tmpobj = cJSON_GetObjectItem(root, "Error");
					if (tmpobj && tmpobj->type == cJSON_String)
					{
						error_ui(vid_buf, 0, tmpobj->valuestring);
					}
					else
						error_ui(vid_buf, 0, "Could not read response");
					return 1;
				}
			}
		}
	}
	else
	{
		char *names[] = {"ID", "Message", NULL};
		char *parts[2];

		parts[0] = id;
		parts[1] = message;

		result = http_multipart_post(
					 "http://" SERVER "/Comment.api",
					 names, parts, NULL,
					 svf_user_id, /*svf_pass*/NULL, svf_session_id,
					 &status, NULL);

		if (status!=200)
		{
			error_ui(vid_buf, status, http_ret_text(status));
			if (result)
				free(result);
			return 1;
		}
		if (result && strncmp(result, "OK", 2))
		{
			error_ui(vid_buf, 0, result);
			free(result);
			return 1;
		}
	}

	if (result)
		free(result);
	return 0;
}

int execute_report(pixel *vid_buf, char *id, char *reason)
{
	int status;
	char *result;

	char *names[] = {"ID", "Reason", NULL};
	char *parts[2];

	parts[0] = id;
	parts[1] = reason;

	result = http_multipart_post(
	             "http://" SERVER "/Report.api",
	             names, parts, NULL,
	             svf_user_id, /*svf_pass*/NULL, svf_session_id,
	             &status, NULL);

	if (status!=200)
	{
		error_ui(vid_buf, status, http_ret_text(status));
		if (result)
			free(result);
		return 0;
	}
	if (result && strncmp(result, "OK", 2))
	{
		error_ui(vid_buf, 0, result);
		free(result);
		return 0;
	}

	if (result)
		free(result);
	return 1;
}

void execute_fav(pixel *vid_buf, char *id)
{
	int status;
	char *result;

	char *names[] = {"ID", NULL};
	char *parts[1];

	parts[0] = id;

	result = http_multipart_post(
	             "http://" SERVER "/Favourite.api",
	             names, parts, NULL,
	             svf_user_id, /*svf_pass*/NULL, svf_session_id,
	             &status, NULL);

	if (status!=200)
	{
		error_ui(vid_buf, status, http_ret_text(status));
		if (result)
			free(result);
		return;
	}
	if (result && strncmp(result, "OK", 2))
	{
		error_ui(vid_buf, 0, result);
		free(result);
		return;
	}

	if (result)
		free(result);
}

void execute_unfav(pixel *vid_buf, char *id)
{
	int status;
	char *result;

	char *names[] = {"ID", NULL};
	char *parts[1];

	parts[0] = id;

	result = http_multipart_post(
	             "http://" SERVER "/Favourite.api?Action=Remove",
	             names, parts, NULL,
	             svf_user_id, /*svf_pass*/NULL, svf_session_id,
	             &status, NULL);

	if (status!=200)
	{
		error_ui(vid_buf, status, http_ret_text(status));
		if (result)
			free(result);
		return;
	}
	if (result && strncmp(result, "OK", 2))
	{
		error_ui(vid_buf, 0, result);
		free(result);
		return;
	}

	if (result)
		free(result);
}

int execute_vote(pixel *vid_buf, char *id, char *action)
{
	int status;
	char *result;

	char *names[] = {"ID", "Action", NULL};
	char *parts[2];

	parts[0] = id;
	parts[1] = action;

	result = http_multipart_post(
	             "http://" SERVER "/Vote.api",
	             names, parts, NULL,
	             svf_user_id, /*svf_pass*/NULL, svf_session_id,
	             &status, NULL);

	if (status!=200)
	{
		error_ui(vid_buf, status, http_ret_text(status));
		if (result)
			free(result);
		return 0;
	}
	if (result && strncmp(result, "OK", 2))
	{
		error_ui(vid_buf, 0, result);
		free(result);
		return 0;
	}

	if (result)
		free(result);
	return 1;
}
void open_link(char *uri) {
#ifdef WIN32
	ShellExecute(0, "OPEN", uri, NULL, NULL, 0);
#elif MACOSX
	char *cmd = (char*)malloc(7+strlen(uri));
	strcpy(cmd, "open ");
	strappend(cmd, uri);
	system(cmd);
#elif LIN32
	char *cmd = (char*)malloc(11+strlen(uri));
	strcpy(cmd, "xdg-open ");
	strappend(cmd, uri);
	system(cmd);
#elif LIN64
	char *cmd = (char*)malloc(11+strlen(uri));
	strcpy(cmd, "xdg-open ");
	strappend(cmd, uri);
	system(cmd);
#else
	printf("Cannot open browser\n");
#endif
}

struct command_match {
	const char *command;
	const char *min_match;
};

const static struct command_match matches [] = {
	{"dofile(\"","dof"},
	{"autorun.lua\")","au"},
	{".lua\")",".lu"},
	{".txt\")",".tx"},
	{"tpt.drawtext(", "tpt.dr"},
	{"tpt.set_pause(", "tpt.set_pa"},
	{"tpt.toggle_pause()", "tpt.to"},
	{"tpt.set_console(", "tpt.set_c"},
	{"tpt.set_pressure(", "tpt.set_pre"},
	{"tpt.set_aheat(", "tpt.set_ah"},
	{"tpt.set_velocity(", "tpt.set_v"},
	{"tpt.set_gravity(", "tpt.set_g"},
	{"tpt.reset_gravity_field()", "tpt.reset_g"},
	{"tpt.reset_velocity()", "tpt.reset_v"},
	{"tpt.reset_spark()", "tpt.reset_s"},
	{"tpt.set_wallmap(", "tpt.set_w"},
	{"tpt.get_wallmap(", "tpt.get_w"},
	{"tpt.set_elecmap(", "tpt.set_e"},
	{"tpt.get_elecmap(", "tpt.get_e"},
	{"tpt.drawpixel(", "tpt.drawp"},
	{"tpt.drawrect(", "tpt.drawr"},
	{"tpt.drawcircle(", "tpt.drawc"},
	{"tpt.fillcircle(", "tpt.fillc"},
	{"tpt.drawline(", "tpt.drawl"},
	{"tpt.get_name()", "tpt.get_n"},
	{"tpt.set_shortcuts(", "tpt.set_s"},
	{"tpt.delete(", "tpt.de"},
	{"tpt.register_mouseevent(", "tpt.register_m"},
	{"tpt.unregister_mouseevent(", "tpt.unregister_m"},
	{"tpt.register_keyevent(", "tpt.register_k"},
	{"tpt.unregister_keyevent(", "tpt.unregister_k"},
	{"tpt.register_", "tpt.reg"},
	{"tpt.unregister_", "tpt.un"},
	{"tpt.get_numOfParts()", "tpt.get_nu"},
	{"tpt.start_getPartIndex()", "tpt.st"},
	{"tpt.next_getPartIndex()", "tpt.ne"},
	{"tpt.getPartIndex()", "tpt.getP"},
	{"tpt.active_menu(", "tpt.ac"},
	{"tpt.display_mode(", "tpt.di"},
	{"tpt.throw_error(\"", "tpt.th"},
	{"tpt.heat(", "tpt.he"},
	{"tpt.setfire(", "tpt.setfi"},
	{"tpt.setdebug(", "tpt.setd"},
	{"tpt.setfpscap(", "tpt.setf"},
	{"tpt.getscript(\"", "tpt.gets"},
	{"tpt.setwindowsize(", "tpt.setw"},
	{"tpt.watertest(", "tpt.wa"},
	{"tpt.screenshot(", "tpt.sc"},
	{"tpt.element_func(", "tpt.element_"},
	{"tpt.element(\"", "tpt.ele"},
	{"tpt.graphics_func(", "tpt.gr"},
	{"tpt.sound(", "tpt.so"},
	{"tpt.bubble(", "tpt.bu"},
	{"tpt.reset_pressure()", "tpt.reset_p"},
	{"tpt.reset_temp()", "tpt.reset_t"},
	{"tpt.get_pressure(", "tpt.get_pre"},
	{"tpt.get_aheat(", "tpt.get_ah"},
	{"tpt.get_velocity(", "tpt.get_v"},
	{"tpt.get_gravity(", "tpt.get_g"},
	{"tpt.maxframes(", "tpt.ma"},
	{"tpt.clear_sim()", "tpt.cl"},
	{"tpt.reset_elements()", "tpt.reset_e"},
	{"tpt.indestructible(", "tpt.ind"},
	{"tpt.moving_solid(","tpt.mo"},
	{"tpt.save_stamp(","tpt.sav"},
	{"tpt.set_selected(","tpt.set_se"},
	{"tpt.set_decocol(","tpt.set_d"},
	{"tpt.load_stamp(","tpt.load_"},
	{"tpt.load(", "tpt.loa"},
	{"tpt.reset_", "tpt.res"},
	{"tpt.create(", "tpt.c"},
	{"tpt.log(", "tpt.l"},
	{"tpt.set_property(\"", "tpt.s"},
	{"tpt.get_property(\"", "tpt.g"},
	{"tpt.fillrect(", "tpt.f"},
	{"tpt.textwidth(", "tpt.t"},
	{"tpt.register_step(", "tpt.r"},
	{"tpt.unregister_step(", "tpt.u"},
	{"tpt.input(\"", "tpt.i"},
	{"tpt.message_box(\"", "tpt.m"},
	{"tpt.hud(", "tpt.h"},
	{"tpt.newtonian_gravity(", "tpt.n"},
	{"tpt.ambient_heat(", "tpt.a"},
	{"tpt.decorations_enable(", "tpt.d"},
	{"tpt.outside_airtemp(", "tpt.o"},

	{"presHighType", "presHighT"},
	{"presLowValue", "presLowV"},
	{"tempHighType", "tempHighT"},
	{"tempLowValue", "tempLowV"},
	{"presHighValue", "presHigh"},
	{"tempHighValue", "tempHigh"},
	{"presLowType", "presLow"},
	{"tempLowType", "tempLow"},
	{"presLow", "presL"},
	{"tempHigh", "tempH"},
	{"tempLow", "tempL"},
	{"presHigh", "pres"},
	{"pres", "pr"},
	{"temp", "tem"},

	{"name", "nam"},
	{"color", "colo"},
	{"colour", "colou"},
	{"advection", "adv"},
	{"airdrag", "aird"},
	{"airloss", "airl"},
	{"loss", "los"},
	{"collision", "coll"},
	{"gravity", "grav"},
	{"diffusion", "diff"},
	{"hotair", "hota"},
	{"falldown", "fall"},
	{"flammable", "flam"},
	{"explosive", "explo"},
	{"meltable", "melta"},
	{"hardness", "hard"},
	{"enabled", "ena"},
	{"weight", "wei"},
	{"menusection", "menus"},
	{"heat", "hea"},
	{"hconduct", "hc"},
	{"state", "sta"},
	{"properties", "prop"},
	{"description", "desc"},
	{"menu", "men"},
	{"dcolor", "dco"},
	{"dcolour", "dcolou"},
	{"ctype", "cty"},
	{"type", "typ"},
};

// limits console to only have 20 (limit) previous commands stored
void console_limit_history(int limit, command_history *commandList)
{
	if (commandList==NULL)
		return;
	console_limit_history(limit-1, commandList->prev_command);
	if (limit <= 0)
	{
		free(commandList);
		commandList = NULL;
	}
	else if (limit == 1)
		commandList->prev_command = NULL;
}

// draws and processes all the history, which are in ui_labels (which aren't very nice looking, but get the job done).
// returns if one of them is focused, to fix copying (since the textbox is usually always focused and would overwrite the copy after)
int console_draw_history(command_history *commandList, command_history *commandresultList, int limit, int divideX, int mx, int my, int b, int bq)
{
	int cc, focused = 0;
	for (cc = 0; cc < limit; cc++)
	{
		if (commandList == NULL)
			break;

		cc += (commandList->command.h/14);
		if (cc >= limit)
			break;

		ui_label_draw(vid_buf, &commandList->command);
		ui_label_process(mx, my, b, bq, &commandList->command);
		ui_label_draw(vid_buf, &commandresultList->command);
		ui_label_process(mx, my, b, bq, &commandresultList->command);
		if (commandList->command.focus || commandresultList->command.focus)
			focused = 1;

		if (commandList->prev_command == NULL)
			break;
		else
		{
			commandList = commandList->prev_command;
			commandresultList = commandresultList->prev_command;
		}
	}
	return focused;
}

// reset the locations of all the history labels when the divider is dragged or a new command added
void console_set_history_X(command_history *commandList, command_history *commandresultList, int divideX)
{
	int cc;
	for (cc = 0; ; cc++)
	{
		int commandHeight, resultHeight;
		if (commandList == NULL)
			break;

		commandHeight = drawtextwrap(vid_buf, 15, 175-(cc*14), divideX-30, 0, commandList->command.str, 0, 0, 0, 0);
		resultHeight = drawtextwrap(vid_buf, divideX+15, 175-(cc*14), XRES-divideX-30, 0, commandresultList->command.str, 0, 0, 0, 0);
		if (resultHeight > commandHeight)
			commandHeight = resultHeight;
		cc += (commandHeight/14);

		commandList->command.y = 175-(cc*14);
		commandList->command.w = divideX-30+14;
		commandList->command.h = commandHeight;
		commandresultList->command.x = divideX+15;
		commandresultList->command.y = 175-(cc*14);
		commandresultList->command.w = XRES-divideX-30+14;
		commandresultList->command.h = commandHeight;

		if (commandList->prev_command == NULL)
			break;
		else
		{
			commandList = commandList->prev_command;
			commandresultList = commandresultList->prev_command;
		}
	}
}

command_history *last_command = NULL;
command_history *last_command_result = NULL;
int divideX = XRES/2-50;
int console_ui(pixel *vid_buf)
{
	int i, mx, my, b = 0, bq, selectedCommand = -1, commandHeight = 12;
	char *match = 0, laststr[1024] = "", draggingDivindingLine = 0, focusTextbox = 1;
	pixel *old_buf = (pixel*)calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	command_history *currentcommand = last_command;
	command_history *currentcommand_result = last_command_result;
	ui_edit ed;
	ui_edit_init(&ed, 15, 207, divideX-15, 14);
	ed.multiline = 1;
	ed.limit = 1023;
	ed.resizable = 1;
	ed.alwaysFocus = 1;

	if (!old_buf)
		return 0;
	memcpy(old_buf,vid_buf,(XRES+BARSIZE)*(YRES+MENUSIZE)*PIXELSIZE);

	console_limit_history(20, currentcommand);
	console_limit_history(20, currentcommand_result);
	console_set_history_X(currentcommand, currentcommand_result, divideX);

	for (i = 0; i < 80; i++) //make background at top slightly darker
		fillrect(old_buf, -1, -1+i, XRES+BARSIZE, 2, 0, 0, 0, 160-(i*2));

	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);
		//everything for dragging the line in the center
		if (mx > divideX - 10 && mx < divideX + 10 && my < 202 && !bq)
		{
			if (b)
				draggingDivindingLine = 2;
			else if (!draggingDivindingLine)
				draggingDivindingLine = 1;
		}
		else if (!b)
			draggingDivindingLine = 0;
		if (draggingDivindingLine == 2)
		{
			int newLine = mx;
			if (newLine < 100)
				newLine = 100;
			if (newLine > XRES-100)
				newLine = XRES-100;
			divideX = newLine;
			ed.w = divideX - 15;
			console_set_history_X(currentcommand, currentcommand_result, divideX);
			if (!b)
				draggingDivindingLine = 1;
		}

		//draw most of the things to the screen
		memcpy(vid_buf,old_buf,(XRES+BARSIZE)*(YRES+MENUSIZE)*PIXELSIZE);
		fillrect(vid_buf, -1, -1, XRES+BARSIZE, 208+commandHeight, 0, 0, 0, 190);
		blend_line(vid_buf, 0, 207+commandHeight, XRES+BARSIZE-1, 207+commandHeight, 228, 228, 228, 255);
		blend_line(vid_buf, divideX, 0, divideX, 207+commandHeight, 255, 255, 255, 155+50*draggingDivindingLine);
#if defined(LUACONSOLE)
		drawtext(vid_buf, 15, 15, "Welcome to The Powder Toy console v.5 (by cracker64/jacob1, Lua enabled)", 255, 255, 255, 255);
#else
		drawtext(vid_buf, 15, 15, "Welcome to The Powder Toy console v.3 (by cracker64)", 255, 255, 255, 255);
#endif

		//draw the visible console history
		currentcommand = last_command;
		currentcommand_result = last_command_result;
		focusTextbox = !console_draw_history(currentcommand, currentcommand_result, 11, divideX, mx, my, b, bq);

		//find matches, for tab autocomplete
		if (strcmp(laststr,ed.str))
		{
			char *str = ed.str;
			for (i = 0; i < sizeof(matches)/sizeof(*matches); i++)
			{
				match = 0;
				while (strstr(str,matches[i].min_match)) //find last match
				{
					match = strstr(str,matches[i].min_match);
					str = match + 1;
				}
				if (match && !strstr(str-1,matches[i].command) && strstr(matches[i].command,match) && strlen(ed.str)-strlen(match)+strlen(matches[i].command) < 256) //if match found
				{
					break;
				}
				else
					match = 0;
			}
		}
		if (match)
			drawtext(vid_buf,ed.x+textwidth(ed.str)-textwidth(match),ed.y,matches[i].command,255,255,255,127);

		drawtext(vid_buf, 5, 207, ">", 255, 255, 255, 240);

		strncpy(laststr, ed.str, 256);
		commandHeight = ui_edit_draw(vid_buf, &ed);
		if (focusTextbox)
			ed.alwaysFocus = 1;
		else
			ed.alwaysFocus = 0;
		ui_edit_process(mx, my, b, bq, &ed);
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
#ifdef OGLR
		clearScreenNP(1.0f);
		draw_parts_fbo();
#endif
		if (sdl_key==SDLK_TAB && match)
		{
			strncpy(match,matches[i].command,strlen(matches[i].command));
			match[strlen(matches[i].command)] = '\0';
			ed.cursor = ed.cursorstart = strlen(ed.str);
		}
		if (sdl_key==SDLK_RETURN) //execute the command, create a new history item
		{
			char *command = mystrdup(ed.str);
			char *result = NULL;
#ifdef LUACONSOLE
			if (process_command_lua(vid_buf, command, &result) == -1)
#else
			if (process_command_old(vid_buf, command, &result) == -1)
#endif
			{
				free(old_buf);
				return -1;
			}

			currentcommand = (command_history*)malloc(sizeof(command_history));
			memset(currentcommand, 0, sizeof(command_history));
			currentcommand->prev_command = last_command;
			ui_label_init(&currentcommand->command, 15, 0, 0, 0);
			if (command)
			{
				strncpy(currentcommand->command.str, command, 1023);
				free(command);
			}
			else
				strcpy(currentcommand->command.str, "");
			last_command = currentcommand;

			currentcommand_result = (command_history*)malloc(sizeof(command_history));
			memset(currentcommand_result, 0, sizeof(command_history));
			currentcommand_result->prev_command = last_command_result;
			ui_label_init(&currentcommand_result->command, 0, 0, 0, 0);
			if (result)
			{
				strncpy(currentcommand_result->command.str, result, 1023);
				free(result);
			}
			else
				strcpy(currentcommand_result->command.str, "");
			last_command_result = currentcommand_result;

			console_limit_history(20, currentcommand);
			console_limit_history(20, currentcommand_result);
			console_set_history_X(currentcommand, currentcommand_result, divideX); // initialize the ui_label locations here, so they can all be changed

			strcpy(ed.str, "");
			ed.cursor = ed.cursorstart = 0;
			selectedCommand = -1;
		}
		if (sdl_key==SDLK_ESCAPE || (sdl_key==SDLK_BACKQUOTE && !(sdl_mod & (KMOD_SHIFT))) || !console_mode) // exit the console
		{
			console_mode = 0;
			free(old_buf);
			return 1;
		}
		if (sdl_key==SDLK_UP || sdl_key==SDLK_DOWN) //up / down to scroll through history
		{
			selectedCommand += sdl_key==SDLK_UP?1:-1;
			if (selectedCommand<-1)
				selectedCommand = -1;
			if (selectedCommand==-1)
			{
				strcpy(ed.str, "");
				ed.cursor = ed.cursorstart = strlen(ed.str);
			}
			else
			{
				if (last_command != NULL)
				{
					int commandLoop;
					currentcommand = last_command;
					for (commandLoop = 0; commandLoop < selectedCommand; commandLoop++) {
						if (currentcommand->prev_command==NULL)
							selectedCommand = commandLoop;
						else
							currentcommand = currentcommand->prev_command;
					}
					strncpy(ed.str, currentcommand->command.str, 1023);
					ed.cursor = ed.cursorstart = strlen(ed.str);
				}
				else
				{
					strcpy(ed.str, "tpt.load(644543)");
					ed.cursor = ed.cursorstart = strlen(ed.str);
				}
			}
		}
	}
	console_mode = 0;
	free(old_buf);
	return 1;
}

ui_edit box_R;
ui_edit box_G;
ui_edit box_B;
ui_edit box_A;

void init_color_boxes()
{
	ui_edit_init(&box_R, 5, 264, 30, 14);
	strcpy(box_R.str, "255");
	box_R.focus = 0;

	ui_edit_init(&box_G, 40, 264, 30, 14);
	strcpy(box_G.str, "255");
	box_G.focus = 0;

	ui_edit_init(&box_B, 75, 264, 30, 14);
	strcpy(box_B.str, "255");
	box_B.focus = 0;

	ui_edit_init(&box_A, 110, 264, 30, 14);
	strcpy(box_A.str, "255");
	box_A.focus = 0;
}

int currR = 255, currG = 0, currB = 0, currA = 255;
int currH = 0, currS = 255, currV = 255;
int on_left = 1, decobox_hidden = 0;

int deco_disablestuff;
void decoration_editor(pixel *vid_buf, int b, int bq, int mx, int my)
{
	int i, t, hh, ss, vv, can_select_color = 1;
	int cr = 255, cg = 0, cb = 0, ca = 255;
	int th = currH, ts = currS, tv = currV;
	int grid_offset_x;
	int window_offset_x;
	int onleft_button_offset_x;
	char frametext[64];

	if (!deco_disablestuff && b) //If mouse is down, but a color isn't already being picked
		can_select_color = 0;
	if (!bq)
		deco_disablestuff = 0;
	currR = PIXR(decocolor), currG = PIXG(decocolor), currB = PIXB(decocolor), currA = decocolor>>24;

	/*for (i = 0; i <= parts_lastActiveIndex; i++)
		if (parts[i].type == PT_ANIM)
		{
			parts[i].tmp2 = framenum;
		}*/

	
	ui_edit_process(mx, my, b, bq, &box_R);
	ui_edit_process(mx, my, b, bq, &box_G);
	ui_edit_process(mx, my, b, bq, &box_B);
	ui_edit_process(mx, my, b, bq, &box_A);

	if(on_left==1)
	{
		grid_offset_x = 5;
		window_offset_x = 2;
		onleft_button_offset_x = 259;
		box_R.x = 5;
		box_G.x = 40;
		box_B.x = 75;
		box_A.x = 110;
	}
	else
	{
		grid_offset_x = XRES - 274;
		window_offset_x = XRES - 279;
		onleft_button_offset_x = 4;
		box_R.x = XRES - 254 + 5;
		box_G.x = XRES - 254 + 40;
		box_B.x = XRES - 254 + 75;
		box_A.x = XRES - 254 + 110;
	}
	
	//drawrect(vid_buf, -1, -1, XRES+1, YRES+1, 220, 220, 220, 255);
	//drawrect(vid_buf, -1, -1, XRES+2, YRES+2, 70, 70, 70, 255);
	//drawtext(vid_buf, 2, 388, "Welcome to the decoration editor v.3 (by cracker64) \n\nClicking the current color on the window will move it to the other side. Right click is eraser. ", 255, 255, 255, 255);
	//drawtext(vid_buf, 2, 388, "Welcome to the decoration editor v.4 (by cracker64/jacob1)", 255, 255, 255, 255);
	//sprintf(frametext,"Frame %i/%i",framenum+1,maxframes);
	//drawtext(vid_buf, 2, 399, frametext, 255, 255, 255, 255);

	if(!decobox_hidden)
	{
		char hex[32] = "";
		fillrect(vid_buf, window_offset_x, 2, 2+255+4+10+5, 2+255+20, 0, 0, 0, currA);
		drawrect(vid_buf, window_offset_x, 2, 2+255+4+10+5, 2+255+20, 255, 255, 255, 255);//window around whole thing

		drawrect(vid_buf, window_offset_x + onleft_button_offset_x +1, 2 +255+6, 12, 12, 255, 255, 255, 255);
		drawrect(vid_buf, window_offset_x + 230, 2 +255+6, 26, 12, 255, 255, 255, 255);
		drawtext(vid_buf, window_offset_x + 232, 2 +255+9, "Clear", 255, 255, 255, 255);
		ui_edit_draw(vid_buf, &box_R);
		ui_edit_draw(vid_buf, &box_G);
		ui_edit_draw(vid_buf, &box_B);
		ui_edit_draw(vid_buf, &box_A);
		sprintf(hex,"0x%.8X",(currA<<24)+(currR<<16)+(currG<<8)+currB);
		drawtext(vid_buf,on_left?145:504,264,hex,currR,currG,currB,currA);

		//draw color square
		for(ss=0; ss<=255; ss++)
		{
			int lasth = -1, currh = 0;
			for(hh=0;hh<=359;hh++)
			{
				currh = clamp_flt((float)hh, 0, 359);
				if (currh == lasth)
					continue;
				lasth = currh;
				t = vid_buf[(ss+5)*(XRES+BARSIZE)+(currh+grid_offset_x)];
				cr = 0;
				cg = 0;
				cb = 0;
				HSV_to_RGB(hh,255-ss,currV,&cr,&cg,&cb);
				cr = ((currA*cr + (255-currA)*PIXR(t))>>8);
				cg = ((currA*cg + (255-currA)*PIXG(t))>>8);
				cb = ((currA*cb + (255-currA)*PIXB(t))>>8);
				vid_buf[(ss+5)*(XRES+BARSIZE)+(currh+grid_offset_x)] = PIXRGB(cr, cg, cb);
			}
		}
		//draw brightness bar
		for(vv=0; vv<=255; vv++)
			for( i=0; i<10; i++)
			{
				t = vid_buf[(vv+5)*(XRES+BARSIZE)+(i+grid_offset_x+255+4)];
				cr = 0;
				cg = 0;
				cb = 0;
				HSV_to_RGB(currH,currS,vv,&cr,&cg,&cb);
				cr = ((currA*cr + (255-currA)*PIXR(t))>>8);
				cg = ((currA*cg + (255-currA)*PIXG(t))>>8);
				cb = ((currA*cb + (255-currA)*PIXB(t))>>8);
				vid_buf[(vv+5)*(XRES+BARSIZE)+(i+grid_offset_x+255+4)] = PIXRGB(cr, cg, cb);
			}
		addpixel(vid_buf,grid_offset_x + clamp_flt((float)currH, 0, 359),5-1,255,255,255,255);
		addpixel(vid_buf,grid_offset_x -1,5+(255-currS),255,255,255,255);

		addpixel(vid_buf,grid_offset_x + clamp_flt((float)th, 0, 359),5-1,100,100,100,255);
		addpixel(vid_buf,grid_offset_x -1,5+(255-ts),100,100,100,255);

		addpixel(vid_buf,grid_offset_x + 255 +3,5+tv,100,100,100,255);
		addpixel(vid_buf,grid_offset_x + 255 +3,5 +currV,255,255,255,255);

		fillrect(vid_buf, window_offset_x + onleft_button_offset_x +1, 2 +255+6, 12, 12, currR, currG, currB, currA);
	}

	if(!box_R.focus)//prevent text update if it is being edited
		sprintf(box_R.str,"%d",currR);
	else
	{
		deco_disablestuff = 1;
		if(sdl_key == SDLK_RETURN)
		{
			cr = atoi(box_R.str);
			if (cr > 255) cr = 255;
			if (cr < 0) cr = 0;
			currR = cr;
			RGB_to_HSV(currR,currG,currB,&currH,&currS,&currV);
			box_R.focus = 0;
		}
	}
	if(!box_G.focus)
		sprintf(box_G.str,"%d",currG);
	else
	{
		deco_disablestuff = 1;
		if(sdl_key == SDLK_RETURN)
		{
			cg = atoi(box_G.str);
			if (cg > 255) cg = 255;
			if (cg < 0) cg = 0;
			currG = cg;
			RGB_to_HSV(currR,currG,currB,&currH,&currS,&currV);
			box_G.focus = 0;
		}
	}
	if(!box_B.focus)
		sprintf(box_B.str,"%d",currB);
	else
	{
		deco_disablestuff = 1;
		if(sdl_key == SDLK_RETURN)
		{
			cb = atoi(box_B.str);
			if (cb > 255) cb = 255;
			if (cb < 0) cb = 0;
			currB = cb;
			RGB_to_HSV(currR,currG,currB,&currH,&currS,&currV);
			box_B.focus = 0;
		}
	}
	if(!box_A.focus)
		sprintf(box_A.str,"%d",currA);
	else
	{
		deco_disablestuff = 1;
		if(sdl_key == SDLK_RETURN)
		{
			ca = atoi(box_A.str);
			if (ca > 255) ca = 255;
			if (ca < 0) ca = 0;
			currA = ca;
			//RGB_to_HSV(currR,currG,currB,&currH,&currS,&currV);
			box_A.focus = 0;
		}
	}

	//fillrect(vid_buf, 250, YRES+4, 40, 15, currR, currG, currB, currA);

	drawrect(vid_buf, 295, YRES+5, 25, 12, 255, 255, 255, 255);
	if(decobox_hidden)
		drawtext(vid_buf, 297, YRES+5 +3, "Show", 255, 255, 255, 255);
	else
		drawtext(vid_buf, 297, YRES+5 +3, "Hide", 255, 255, 255, 255);

	if(can_select_color && !decobox_hidden && mx >= window_offset_x && my >= 2 && mx <= window_offset_x+255+4+10+5 && my <= 2+255+20)//in the main window
	{
		//inside brightness bar
		if(mx >= grid_offset_x +255+4 && my >= 5 && mx <= grid_offset_x+255+4+10 && my <= 5+255)
		{
			tv =  my - 5;
			if (b)
			{
				currV = my - 5;
				HSV_to_RGB(currH,currS,tv,&currR,&currG,&currB);
				deco_disablestuff = 1;
				if (activeTools[0]->GetIdentifier() != "DEFAULT_WL_26")
					activeTools[0] = GetToolFromIdentifier("DEFAULT_WL_26");
			}
			HSV_to_RGB(currH,currS,tv,&cr,&cg,&cb);
			//clearrect(vid_buf, window_offset_x + onleft_button_offset_x +1, window_offset_y +255+6,12,12);
			fillrect(vid_buf, window_offset_x + onleft_button_offset_x +1, 2 +255+6, 12, 12, cr, cg, cb, ca);
			if(!box_R.focus)
				sprintf(box_R.str,"%d",cr);
			if(!box_G.focus)
				sprintf(box_G.str,"%d",cg);
			if(!box_B.focus)
				sprintf(box_B.str,"%d",cb);
			if(!box_A.focus)
				sprintf(box_A.str,"%d",ca);
		}
		//inside color grid
		if(mx >= grid_offset_x && my >= 5 && mx <= grid_offset_x+255 && my <= 5+255)
		{
			th = mx - grid_offset_x;
			th = (int)( th*359/255 );
			ts = 255 - (my - 5);
			if(b)
			{
				currH = th;
				currS = ts;
				HSV_to_RGB(th,ts,currV,&currR,&currG,&currB);
				deco_disablestuff = 1;
				if (activeTools[0]->GetIdentifier() != "DEFAULT_WL_26")
					activeTools[0] = GetToolFromIdentifier("DEFAULT_WL_26");
			}
			HSV_to_RGB(th,ts,currV,&cr,&cg,&cb);
			//clearrect(vid_buf, window_offset_x + onleft_button_offset_x +1, window_offset_y +255+6,12,12);
			fillrect(vid_buf, window_offset_x + onleft_button_offset_x +1, 2 +255+6, 12, 12, cr, cg, cb, ca);
			//sprintf(box_R.def,"%d",cr);
			if(!box_R.focus)
				sprintf(box_R.str,"%d",cr);
			if(!box_G.focus)
				sprintf(box_G.str,"%d",cg);
			if(!box_B.focus)
				sprintf(box_B.str,"%d",cb);
			if(!box_A.focus)
				sprintf(box_A.str,"%d",ca);
		}
		//switch side button
		if(b && !bq && mx >= window_offset_x + onleft_button_offset_x +1 && my >= 2 +255+6 && mx <= window_offset_x + onleft_button_offset_x +13 && my <= 2 +255+5 +13)
		{
			on_left = !on_left;
			deco_disablestuff = 1;
		}
		//clear button
		if(b && !bq && mx >= window_offset_x + 230 && my >= 2 +255+6 && mx <= window_offset_x + 230 +26 && my <= 2 +255+5 +13)
			if (confirm_ui(vid_buf, "Reset Decoration Layer", "Do you really want to erase everything?", "Erase") )
			{
				int i;
				for (i=0;i<NPART;i++)
					parts[i].dcolour = 0;
			}
			deco_disablestuff = 1;
	}
	else if (mx > XRES || my > YRES)//mouse outside normal drawing area
	{
		//hide/show button
		if (b && !bq && mx >= 295 && mx <= 295+25 && my >= YRES+5 && my<= YRES+5+12)
		{
			decobox_hidden = !decobox_hidden;
			zoom_en = 0;
		}
	}
	if (sdl_key==SDLK_RIGHT && framenum < maxframes-1)
	{
		int i;
		framenum++;
		if (framenum > numframes)
			numframes = framenum;
		for (i = 0; i <= parts_lastActiveIndex; i++)
			if (parts[i].type == PT_ANIM)
			{
				parts[i].tmp2 = framenum;
				if (parts[i].ctype < numframes)
					parts[i].ctype = numframes;
				if (sdl_mod & (KMOD_CTRL))
					parts[i].animations[numframes] = parts[i].animations[numframes-1];
			}
	}
	if (sdl_key==SDLK_LEFT && framenum > 0)
	{
		int i;
		framenum--;
		for (i = 0; i <= parts_lastActiveIndex; i++)
			if (parts[i].type == PT_ANIM)
				parts[i].tmp2 = framenum;
	}
	if (sdl_key==SDLK_DELETE && numframes > 0)
	{
		int i;
		numframes--;
		for (i = 0; i <= parts_lastActiveIndex; i++)
			if (parts[i].type == PT_ANIM)
			{
				int j;
				if (parts[i].ctype != numframes)
				{
					for (j = framenum; j <= parts[i].ctype; j++)
						parts[i].animations[j] = parts[i].animations[j+1];
					parts[i].animations[parts[i].ctype] = 0;
				}
				parts[i].ctype--;
				if (framenum == numframes+1)
					parts[i].tmp2 = framenum-1;
			}
		if (framenum >= numframes)
			framenum--;
	}
	if (sdl_zoom_trig)
		decobox_hidden = 1;
	/*if(sdl_key=='b' || sdl_key==SDLK_ESCAPE)
	{
		/*for (i = 0; i <= parts_lastActiveIndex; i++)
			if (parts[i].type == PT_ANIM)
			{
				parts[i].tmp2 = 0;
			}* /
		decocolor = (currA<<24)|PIXRGB(currR,currG,currB);
	}*/
	/*for (i = 0; i <= parts_lastActiveIndex; i++)
		if (parts[i].type == PT_ANIM)
		{
			parts[i].tmp2 = 0;
		}*/
	decocolor = (currA<<24)|PIXRGB(currR,currG,currB);
}
struct savelist_e;
typedef struct savelist_e savelist_e;
struct savelist_e {
	char *filename;
	char *name;
	pixel *image;
	savelist_e *next;
	savelist_e *prev;
};
savelist_e *get_local_saves(char *folder, char *search, int *results_ret)
{
	int index = 0, results = 0;
	savelist_e *new_savelist = NULL;
	savelist_e *current_item = NULL, *new_item = NULL;
	char *fname;
#if defined(WIN32) && !defined(__GNUC__)
	struct _finddata_t current_file;
	intptr_t findfile_handle;
	char *filematch = (char*)malloc(strlen(folder)+4);
	sprintf(filematch, "%s%s", folder, "*.*");
	findfile_handle = _findfirst(filematch, &current_file);
	free(filematch);
	if (findfile_handle == -1L)
	{
		*results_ret = 0;
		return NULL;
	}
	do
	{
		fname = current_file.name;
#else
	struct dirent *derp;
	DIR *directory = opendir(folder);
	if(!directory)
	{
		printf("Unable to open directory\n");
		*results_ret = 0;
		return NULL;
	}
	while(derp = readdir(directory))
	{
		fname = derp->d_name;
#endif
		if(strlen(fname)>4)
		{
			char *ext = fname+(strlen(fname)-4);
			if((!strncmp(ext, ".cps", 4) || !strncmp(ext, ".stm", 4)) && (search==NULL || strstr(fname, search)))
			{
				new_item = (savelist_e*)malloc(sizeof(savelist_e));
				new_item->filename = (char*)malloc(strlen(folder)+strlen(fname)+1);
				sprintf(new_item->filename, "%s%s", folder, fname);
				new_item->name = mystrdup(fname);
				new_item->image = NULL;
				new_item->next = NULL;
				if(new_savelist==NULL){
					new_savelist = new_item;
					new_item->prev = NULL;
				} else {
					current_item->next = new_item;
					new_item->prev = current_item;
				}
				current_item = new_item;
				results++;
			}
		}
	}
#if defined(WIN32) && !defined(__GNUC__)
	while (_findnext(findfile_handle, &current_file) == 0);
	_findclose(findfile_handle);
#else
	closedir(directory);
#endif
	*results_ret = results;
	return new_savelist;
}

void free_saveslist(savelist_e *saves)
{
	if(!saves)
		return;
	if(saves->next!=NULL)
		free_saveslist(saves->next);
	if(saves->filename!=NULL)
		free(saves->filename);
	if(saves->name!=NULL)
		free(saves->name);
	if(saves->image!=NULL)
		free(saves->image);
}

int save_filename_ui(pixel *vid_buf)
{
	int xsize = 16+(XRES/3);
	int ysize = 64+(YRES/3);
	float ca = 0;
	int x0=(XRES+BARSIZE-xsize)/2,y0=(YRES+MENUSIZE-ysize)/2,b=1,bq,mx,my;
	int idtxtwidth, nd=0, imgw, imgh, save_size;
	void *save_data;
	char *savefname = NULL;
	char *filename = NULL;
	pixel *old_vid=(pixel *)calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	pixel *save_tmp;
	pixel *save_data_image;
	pixel *save = NULL;//calloc((XRES/3)*(YRES/3), PIXELSIZE);
	ui_edit ed;
	int official_save = check_save(2,0,0,XRES,YRES,0), oldsave_as = save_as;
	if (official_save)
		save_as = 3;

	save_data = build_save(&save_size, 0, 0, XRES, YRES, bmap, vx, vy, pv, fvx, fvy, signs, parts, (save_as == 3 && (sdl_mod & KMOD_SHIFT)));
	if (!save_data)
	{
		error_ui(vid_buf, 0, "Unable to create save file");
		free(old_vid);
		return 1;
	}

	save_data_image = prerender_save(save_data, save_size, &imgw, &imgh);
	if(save_data_image!=NULL)
	{
		save = resample_img(save_data_image, imgw, imgh, XRES/3, YRES/3);
	}

	ed.x = x0+11;
	ed.y = y0+25;
	ed.w = xsize-4-16;
	ed.nx = 1;
	ed.def = "[filename]";
	ed.focus = 1;
	ed.hide = 0;
	ed.cursor = ed.cursorstart = 0;
	ed.multiline = 0;
	ed.limit = 255;
	ed.str[0] = 0;
	
	if(svf_fileopen){
		char * dotloc = NULL;
		strncpy(ed.str, svf_filename, 255);
		if(dotloc = strstr(ed.str, "."))
		{
			dotloc[0] = 0;
		}
		ed.cursor = ed.cursorstart = strlen(ed.str);
	}

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
	draw_rgba_image(vid_buf, (unsigned char*)save_to_disk_image, 0, 0, 0.7f);
	
	memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);

	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		clearrect(vid_buf, x0-2, y0-2, xsize+4, ysize+4);
		drawrect(vid_buf, x0, y0, xsize, ysize, 192, 192, 192, 255);
		drawtext(vid_buf, x0+8, y0+8, "Filename:", 255, 255, 255, 255);
		drawrect(vid_buf, x0+8, y0+20, xsize-16, 16, 255, 255, 255, 180);
		if(save!=NULL)
		{
			draw_image(vid_buf, save, x0+8, y0+40, XRES/3, YRES/3, 255);
		}
		drawrect(vid_buf, x0+8, y0+40, XRES/3, YRES/3, 192, 192, 192, 255);
		
		drawrect(vid_buf, x0, y0+ysize-16, xsize, 16, 192, 192, 192, 255);
		fillrect(vid_buf, x0, y0+ysize-16, xsize, 16, 170, 170, 192, (int)ca);
		drawtext(vid_buf, x0+8, y0+ysize-12, "Save", 255, 255, 255, 255);

		ui_edit_draw(vid_buf, &ed);
		if (strlen(ed.str) || ed.focus)
			drawtext(vid_buf, x0+12+textwidth(ed.str), y0+25, ".cps", 240, 240, 255, 180);
#ifdef OGLR
		clearScreen(1.0f);
#endif
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		memcpy(vid_buf, old_vid, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);

		ui_edit_process(mx, my, b, bq, &ed);
		
		if(mx > x0 && mx < x0+xsize && my > y0+ysize-16 && my < y0+ysize)
		{
			clean_text(ed.str,256);
			if(b && !bq)
			{
				FILE *f = NULL;
				savefname = (char*)malloc(strlen(ed.str)+5);
				filename = (char*)malloc(strlen(LOCAL_SAVE_DIR)+strlen(PATH_SEP)+strlen(ed.str)+5);
				sprintf(filename, "%s%s%s.cps", LOCAL_SAVE_DIR, PATH_SEP, ed.str);
				sprintf(savefname, "%s.cps", ed.str);
			
#ifdef WIN32
				_mkdir(LOCAL_SAVE_DIR);
#else
				mkdir(LOCAL_SAVE_DIR, 0755);
#endif
				f = fopen(filename, "r");
				if(!f || confirm_ui(vid_buf, "A save with the name already exists.", filename, "Overwrite"))
				{
					if(f)
					{
						fclose(f);
						f = NULL;
					}
					f = fopen(filename, "wb");
					if (f)
					{
						fwrite(save_data, save_size, 1, f);
						fclose(f);
						if(svf_fileopen)
						{
							strncpy(svf_filename, savefname, 255);
							svf_fileopen = 1;
							
							//Allow reloading
							if(svf_last)
								free(svf_last);
							svf_last = malloc(save_size);
							memcpy(svf_last, save_data, save_size);
							svf_lsize = save_size;
						}
						fclose(f);
						break;
					} else {
						error_ui(vid_buf, 0, "Unable to write to save file.");
					}
				}
			}
		}

		if (sdl_key==SDLK_ESCAPE)
		{
			if (!ed.focus)
				break;
			ed.focus = 0;
		}
	}
		
savefin:
	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
	free(save_data_image);
	free(save_data);
	free(old_vid);
	free(save);
	if(filename) free(filename);
	if(savefname) free(savefname);
	save_as = oldsave_as;
	return 0;
}

void catalogue_ui(pixel * vid_buf)
{
	int xsize = 8+(XRES/CATALOGUE_S+8)*CATALOGUE_X;
	int ysize = 48+(YRES/CATALOGUE_S+20)*CATALOGUE_Y;
	int x0=(XRES+BARSIZE-xsize)/2,y0=(YRES+MENUSIZE-ysize)/2,b=1,bq,mx,my;
	int rescount, imageoncycle = 0, currentstart = 0, currentoffset = 0, thidden = 0, cactive = 0;
	int listy = 0, listxc;
	int listx = 0, listyc;
	pixel * vid_buf2;
	float scrollvel, offsetf = 0.0f;
	char savetext[128] = "";
	char * last = mystrdup("");
	savelist_e *saves, *cssave, *csave;
	ui_edit ed;
	
	vid_buf2 = (pixel*)calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	if (!vid_buf2)
		return;
	
	ed.w = xsize-16-4;
	ed.x = x0+11;
	ed.y = y0+29;
	ed.multiline = 0;
	ed.limit = 255;
	ed.def = "[search]";
	ed.focus = 0;
	ed.hide = 0;
	ed.cursor = ed.cursorstart = 0;
	ed.nx = 0;
	strcpy(ed.str, "");

	saves = get_local_saves(LOCAL_SAVE_DIR PATH_SEP, NULL, &rescount);
	cssave = csave = saves;
	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
	
	fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);
		sprintf(savetext, "Found %d save%s", rescount, rescount==1?"":"s");
		clearrect(vid_buf, x0-2, y0-2, xsize+4, ysize+4);
		clearrect(vid_buf2, x0-2, y0-2, xsize+4, ysize+4);
		drawrect(vid_buf, x0, y0, xsize, ysize, 192, 192, 192, 255);
		drawtext(vid_buf, x0+8, y0+8, "Saves", 255, 216, 32, 255);
		drawtext(vid_buf, x0+xsize-8-textwidth(savetext), y0+8, savetext, 255, 216, 32, 255);
		drawrect(vid_buf, x0+8, y0+24, xsize-16, 16, 255, 255, 255, 180);
		if(strcmp(ed.str, last)){
			free(last);
			last = mystrdup(ed.str);
			currentstart = 0;
			if(saves!=NULL) free_saveslist(saves);
			saves = get_local_saves(LOCAL_SAVE_DIR PATH_SEP, last, &rescount);
			cssave = saves;
			scrollvel = 0.0f;
			offsetf = 0.0f;
			thidden = 0;
		}
		//Scrolling
		if (sdl_wheel!=0)
		{
			scrollvel -= (float)sdl_wheel;
			if(scrollvel > 5.0f) scrollvel = 5.0f;
			if(scrollvel < -5.0f) scrollvel = -5.0f;
			sdl_wheel = 0;
		}
		offsetf += scrollvel;
		scrollvel*=0.99f;
		if(offsetf >= (YRES/CATALOGUE_S+20) && rescount)
		{
			if(rescount - thidden > CATALOGUE_X*(CATALOGUE_Y+1))
			{
				int i = 0;
				for(i = 0; i < CATALOGUE_X; i++){
					if(cssave->next==NULL)
						break;
					cssave = cssave->next;
				}
				offsetf -= (YRES/CATALOGUE_S+20);
				thidden += CATALOGUE_X;
			} else {
				offsetf = (YRES/CATALOGUE_S+20);
			}
		} 
		if(offsetf > 0.0f && rescount <= CATALOGUE_X*CATALOGUE_Y && rescount)
		{
			offsetf = 0.0f;
		}
		if(offsetf < 0.0f && rescount)
		{
			if(thidden >= CATALOGUE_X)
			{
				int i = 0;
				for(i = 0; i < CATALOGUE_X; i++){
					if(cssave->prev==NULL)
						break;
					cssave = cssave->prev;
				}
				offsetf += (YRES/CATALOGUE_S+20);
				thidden -= CATALOGUE_X;
			} else {
				offsetf = 0.0f;
			}
		}
		currentoffset = (int)offsetf;
		csave = cssave;
		//Diplay
		if(csave!=NULL && rescount)
		{
			listx = 0;
			listy = 0;
			while(csave!=NULL)
			{
				listxc = x0+8+listx*(XRES/CATALOGUE_S+8);
				listyc = y0+48-currentoffset+listy*(YRES/CATALOGUE_S+20);
				if(listyc > y0+ysize) //Stop when we get to the bottom of the viewable
					break;
				cactive = 0;
				if(my > listyc && my < listyc+YRES/CATALOGUE_S+2 && mx > listxc && mx < listxc+XRES/CATALOGUE_S && my > y0+48 && my < y0+ysize)
				{
					if(b)
					{
						int status, size;
						void *data;
						data = file_load(csave->filename, &size);
						if(data){
							status = parse_save(data, size, 1, 0, 0, bmap, vx, vy, pv, fvx, fvy, signs, parts, pmap);
							if(!status)
							{
								//svf_filename[0] = 0;
								strncpy(svf_filename, csave->name, 255);
								svf_fileopen = 1;
								svf_open = 0;
								svf_publish = 0;
								svf_modsave = 0;
								svf_own = 0;
								svf_myvote = 0;
								svf_id[0] = 0;
								svf_name[0] = 0;
								svf_description[0] = 0;
								svf_author[0] = 0;
								svf_tags[0] = 0;
								if (svf_last)
									free(svf_last);
								svf_last = data;
								data = NULL;
								svf_lsize = size;
								ctrlzSnapshot();
								goto openfin;
							} else {
								error_ui(vid_buf, 0, "Save data corrupt");
								free(data);
							}
						} else {
							error_ui(vid_buf, 0, "Unable to read save file");
						}
					}
					cactive = 1;
				}
				//Generate an image
				if(csave->image==NULL && !imageoncycle){ //imageoncycle: Don't read/parse more than one image per cycle, makes it more resposive for slower computers
					int imgwidth, imgheight, size;
					pixel *tmpimage = NULL;
					void *data = NULL;
					data = file_load(csave->filename, &size);
					if(data!=NULL){
						tmpimage = prerender_save(data, size, &imgwidth, &imgheight);
						if(tmpimage!=NULL)
						{
							csave->image = resample_img(tmpimage, imgwidth, imgheight, XRES/CATALOGUE_S, YRES/CATALOGUE_S);
							free(tmpimage);
						} else {
							//Blank image, TODO: this should default to something else
							csave->image = (pixel*)calloc((XRES/CATALOGUE_S)*(YRES/CATALOGUE_S), PIXELSIZE);
						}
						free(data);
					} else {
						//Blank image, TODO: this should default to something else
						csave->image = (pixel*)calloc((XRES/CATALOGUE_S)*(YRES/CATALOGUE_S), PIXELSIZE);
					}
					imageoncycle = 1;
				}
				if(csave->image!=NULL)
					draw_image(vid_buf2, csave->image, listxc, listyc, XRES/CATALOGUE_S, YRES/CATALOGUE_S, 255);
				drawrect(vid_buf2, listxc, listyc, XRES/CATALOGUE_S, YRES/CATALOGUE_S, 255, 255, 255, 190);
				if(cactive)
					drawtext(vid_buf2, listxc+((XRES/CATALOGUE_S)/2-textwidth(csave->name)/2), listyc+YRES/CATALOGUE_S+3, csave->name, 255, 255, 255, 255);
				else
					drawtext(vid_buf2, listxc+((XRES/CATALOGUE_S)/2-textwidth(csave->name)/2), listyc+YRES/CATALOGUE_S+3, csave->name, 240, 240, 255, 180);
				if (mx>=listxc+XRES/GRID_S-4 && mx<=listxc+XRES/GRID_S+6 && my>=listyc-6 && my<=listyc+4)
				{
					if (b && !bq && confirm_ui(vid_buf, "Do you want to delete?", csave->name, "Delete"))
					{
						remove(csave->filename);
						currentstart = 0;
						if(saves!=NULL) free_saveslist(saves);
						saves = get_local_saves(LOCAL_SAVE_DIR PATH_SEP, last, &rescount);
						cssave = saves;
						scrollvel = 0.0f;
						offsetf = 0.0f;
						thidden = 0;
						if (rescount == 0)
							rmdir(LOCAL_SAVE_DIR PATH_SEP);
						break;
					}
					drawtext(vid_buf2, listxc+XRES/GRID_S-4, listyc-6, "\x86", 255, 48, 32, 255);
				}
				else
					drawtext(vid_buf2, listxc+XRES/GRID_S-4, listyc-6, "\x86", 160, 48, 32, 255);
				drawtext(vid_buf2, listxc+XRES/GRID_S-4, listyc-6, "\x85", 255, 255, 255, 255);
				csave = csave->next;
				if(++listx==CATALOGUE_X){
					listx = 0;
					listy++;
				}
			}
			imageoncycle = 0;
		} else {
			drawtext(vid_buf2, x0+(xsize/2)-(textwidth("No saves found")/2), y0+(ysize/2)+20, "No saves found", 255, 255, 255, 180);
		}
		ui_edit_draw(vid_buf, &ed);
		ui_edit_process(mx, my, b, bq, &ed);
		//Draw the scrollable area onto the main buffer
		{
			pixel *srctemp = vid_buf2, *desttemp = vid_buf;
			int j = 0;
			for (j = y0+42; j < y0+ysize; j++)
			{
				memcpy(desttemp+j*(XRES+BARSIZE)+x0+1, srctemp+j*(XRES+BARSIZE)+x0+1, (xsize-1)*PIXELSIZE);
				//desttemp+=(XRES+BARSIZE);//*PIXELSIZE;
				//srctemp+=(XRES+BARSIZE);//*PIXELSIZE;
			}
		}
#ifdef OGLR
		clearScreen(1.0f);
#endif
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
			break;
	}
openfin:	
	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	if(saves)
		free_saveslist(saves);
	return;
}

void drawIcon(pixel * vid_buf, int x, int y, int cmode)
{
	switch (cmode)
	{
	case 0x98:
		drawtext(vid_buf, x, y, "\x98", 128, 160, 255, 255);
		break;
	case 0x99:
		drawtext(vid_buf, x, y, "\x99", 255, 212, 32, 255);
		break;
	case 0x9A:
		drawtext(vid_buf, x, y, "\x9A", 212, 212, 212, 255);
		break;
	case 0x9B:
		drawtext(vid_buf, x+1, y, "\x9B", 255, 0, 0, 255);
		drawtext(vid_buf, x+1, y, "\x9C", 255, 255, 64, 255);
		break;
	case 0xBF:
		drawtext(vid_buf, x, y, "\xBF", 55, 255, 55, 255);
		break;
	case 0xBE:
		drawtext(vid_buf, x+2, y, "\xBE", 255, 0, 0, 255);
		drawtext(vid_buf, x+2, y, "\xBD", 255, 255, 255, 255);
		break;
	case 0xC4:
		drawtext(vid_buf, x, y, "\xC4", 100, 150, 255, 255);
		break;
	case 0xD3:
		drawtext(vid_buf, x, y, "\xD3", 255, 50, 255, 255);
		break;
	case 0xE0:
		drawtext(vid_buf, x, y, "\xE0", 255, 255, 255, 255);
		break;
	case 0xE1:
		drawtext(vid_buf, x, y, "\xE1", 255, 255, 160, 255);
		break;
	case 0xDF:
		drawtext(vid_buf, x, y, "\xDF", 200, 255, 255, 255);
		break;
	case 0xDE:
		drawtext(vid_buf, x, y, "\xDE", 255, 255, 255, 255);
		break;
	case 0xDB:
		drawtext(vid_buf, x, y, "\xDB", 255, 255, 200, 255);
		break;
	case 0xD4:
		drawtext(vid_buf, x, y, "\xD4", 255, 55, 55, 255);
		drawtext(vid_buf, x, y, "\xD5", 55, 255, 55, 255);
		break;
	}
}

void render_ui(pixel * vid_buf, int xcoord, int ycoord, int orientation)
{
	pixel * o_vid_buf;
	pixel *part_vbuf; //Extra video buffer
	pixel *part_vbuf_store;
	int i, j, count, changed, temp;
	int xsize;
	int ysize;
	int yoffset;
	int xoffset;
	int xcoffset;
	int b, bq, mx, my;
	ui_checkbox *render_cb;
	ui_checkbox *display_cb;
	ui_checkbox *colour_cb;

	int render_optioncount = 7;
	int render_options[] = {RENDER_EFFE, RENDER_GLOW, RENDER_FIRE, RENDER_BLUR, RENDER_BASC, RENDER_BLOB, RENDER_NONE};
	int render_optionicons[] = {0xE1, 0xDF, 0x9B, 0xC4, 0xDB, 0xBF, 0xDB};
	char * render_desc[] = {"Effects", "Glow", "Fire", "Blur", "Basic", "Blob", "None"};

#ifdef OGLR
	int display_optioncount = 7;
	int display_options[] = {DISPLAY_AIRC, DISPLAY_AIRP, DISPLAY_AIRV, DISPLAY_AIRH, DISPLAY_WARP, DISPLAY_PERS, DISPLAY_EFFE};
	int display_optionicons[] = {0xD4, 0x99, 0x98, 0xBE, 0xDE, 0x9A, 0xE1};
	char * display_desc[] = {"Air: Cracker", "Air: Pressure", "Air: Velocity", "Air: Heat", "Warp effect", "Persistent", "Effects"};
#else
	int display_optioncount = 6;
	int display_options[] = {DISPLAY_AIRC, DISPLAY_AIRP, DISPLAY_AIRV, DISPLAY_AIRH, DISPLAY_WARP, DISPLAY_PERS};
	int display_optionicons[] = {0xD4, 0x99, 0x98, 0xBE, 0xDE, 0x9A};
	char * display_desc[] = {"Air: Cracker", "Air: Pressure", "Air: Velocity", "Air: Heat", "Warp effect", "Persistent"};
#endif

	int colour_optioncount = 4;
	int colour_options[] = {COLOUR_BASC, COLOUR_LIFE, COLOUR_HEAT, COLOUR_GRAD};
	int colour_optionicons[] = {0xDB, 0xE0, 0xBE, 0xD3};
	char * colour_desc[] = {"Basic", "Life", "Heat", "Heat Gradient"};

	yoffset = 16;
	xoffset = 0;
	
	xcoffset = 35;
	
	xsize = xcoffset*3;
	ysize = render_optioncount * yoffset + 6;
	
	ycoord -= ysize;
	xcoord -= xsize;
	
	colour_cb = (ui_checkbox*)calloc(colour_optioncount, sizeof(ui_checkbox));
	for(i = 0; i < colour_optioncount; i++)
	{
		colour_cb[i].x = (xcoffset * 0) + xcoord + (i * xoffset) + 5;
		colour_cb[i].y = ycoord + (i * yoffset) + 5;
		colour_cb[i].focus = 0;
		colour_cb[i].checked = 0;
		j = 0;
		if(colour_mode == colour_options[i])
		{
			colour_cb[i].checked = 1;
		}
	}
	
	render_cb = (ui_checkbox*)calloc(render_optioncount, sizeof(ui_checkbox));
	for(i = 0; i < render_optioncount; i++)
	{
		render_cb[i].x = (xcoffset * 1) + xcoord + (i * xoffset) + 5;
		render_cb[i].y = ycoord + (i * yoffset) + 5;
		render_cb[i].focus = 0;
		render_cb[i].checked = 0;
		j = 0;
		while(render_modes[j])
		{
			if(render_modes[j] == render_options[i])
			{
				render_cb[i].checked = 1;
				break;
			}
			j++;
		}
	}
	
	display_cb = (ui_checkbox*)calloc(display_optioncount, sizeof(ui_checkbox));
	for(i = 0; i < display_optioncount; i++)
	{
		display_cb[i].x = (xcoffset * 2) + xcoord + (i * xoffset) + 5;
		display_cb[i].y = ycoord + (i * yoffset) + 5;
		display_cb[i].focus = 0;
		display_cb[i].checked = 0;
		j = 0;
		while(display_modes[j])
		{
			if(display_modes[j] == display_options[i])
			{
				display_cb[i].checked = 1;
				break;
			}
			j++;
		}
	}
	
	o_vid_buf = (pixel*)calloc((YRES+MENUSIZE) * (XRES+BARSIZE), PIXELSIZE);
	//memcpy(o_vid_buf, vid_buf, ((YRES+MENUSIZE) * (XRES+BARSIZE)) * PIXELSIZE);
	
	part_vbuf = (pixel*)calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE); //Extra video buffer
	part_vbuf_store = part_vbuf;
	
	if (!o_vid_buf || !part_vbuf || !display_cb || !colour_cb || !render_cb)
	{
		if(o_vid_buf)
			free(o_vid_buf);
		if(part_vbuf)
			free(part_vbuf);
		if (display_cb)
			free(display_cb);
		if (colour_cb)
			free(colour_cb);
		if (render_cb)
			free(render_cb);
		return;
	}
	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
	
	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);
		
		memcpy(vid_buf, o_vid_buf, ((YRES+MENUSIZE) * (XRES+BARSIZE)) * PIXELSIZE);
#ifdef OGLR
		part_vbuf = vid_buf;
#else
		if(ngrav_enable && (display_mode & DISPLAY_WARP))
		{
			part_vbuf = part_vbuf_store;
			memset(vid_buf, 0, (XRES+BARSIZE)*YRES*PIXELSIZE);
		} else {
			part_vbuf = vid_buf;
		}
#endif
		render_before(part_vbuf);
		render_after(part_vbuf, vid_buf);
		if (old_menu)
		{
			for (i=0; i<SC_TOTAL; i++)
			{
				if (i < SC_FAV)
					drawchar(vid_buf, XRES+1, /*(12*i)+2*/((YRES/GetNumMenus())*i)+((YRES/GetNumMenus())/2)+5, menuSections[i]->icon, 255, 255, 255, 255);
			}
		}
		else
		{
			quickoptions_menu(vid_buf, b, bq, mx, my);
			DrawMenus(vid_buf, active_menu, my);
		}
		draw_svf_ui(vid_buf, sdl_mod & (KMOD_LCTRL|KMOD_RCTRL));
		
		clearrect(vid_buf, xcoord-2, ycoord-2, xsize+4, ysize+4);
		drawrect(vid_buf, xcoord, ycoord, xsize, ysize, 192, 192, 192, 255);
		
		changed = 0;
		for(i = 0; i < render_optioncount; i++)
		{
			temp = render_cb[i].checked;
			drawIcon(vid_buf, render_cb[i].x + 16, render_cb[i].y+2, render_optionicons[i]);
			ui_checkbox_draw(vid_buf, &(render_cb[i]));
			ui_checkbox_process(mx, my, b, bq, &(render_cb[i]));
			if(render_cb[i].focus)
				drawtext(vid_buf, xcoord - textwidth(render_desc[i]) - 10, render_cb[i].y+2, render_desc[i], 255, 255, 255, 255);
			if(temp != render_cb[i].checked)
				changed = 1;
		}
		if(changed)
		{
			//Compile render options
			count = 1;
			for(i = 0; i < render_optioncount; i++)
			{
				if(render_cb[i].checked)
					count++;
			}
			free(render_modes);
			render_mode = 0;
			render_modes = (unsigned int*)calloc(count, sizeof(unsigned int));
			count = 0;
			for(i = 0; i < render_optioncount; i++)
			{
				if(render_cb[i].checked)
				{
					render_modes[count] = render_options[i];
					render_mode |= render_options[i];
					count++;
				}
			}
		}
		
		changed = 0;
		for(i = 0; i < display_optioncount; i++)
		{
			temp = display_cb[i].checked;
			drawIcon(vid_buf, display_cb[i].x + 16, display_cb[i].y+2, display_optionicons[i]);

			if(display_options[i] & DISPLAY_AIR)
			{
				ui_radio_draw(vid_buf, &(display_cb[i]));
				ui_radio_process(mx, my, b, bq, &(display_cb[i]));
			}
			else
			{
				ui_checkbox_draw(vid_buf, &(display_cb[i]));
				ui_checkbox_process(mx, my, b, bq, &(display_cb[i]));
			}
			if(display_cb[i].checked && (display_options[i] & DISPLAY_AIR))	//One air type only
			{
				for(j = 0; j < display_optioncount; j++)
				{
					if((display_options[j] & DISPLAY_AIR) && j!=i)
					{
						display_cb[j].checked = 0;
					}
				}
			}
			if(display_cb[i].focus)
				drawtext(vid_buf, xcoord - textwidth(display_desc[i]) - 10, display_cb[i].y+2, display_desc[i], 255, 255, 255, 255);
			if(temp != display_cb[i].checked)
				changed = 1;
		}
		if(changed)
		{
			//Compile display options
			count = 1;
			for(i = 0; i < display_optioncount; i++)
			{
				if(display_cb[i].checked)
					count++;
			}
			free(display_modes);
			display_mode = 0;
			display_modes = (unsigned int*)calloc(count, sizeof(unsigned int));
			count = 0;
			for(i = 0; i < display_optioncount; i++)
			{
				if(display_cb[i].checked)
				{
					display_modes[count] = display_options[i];
					display_mode |= display_options[i];
					count++;
				}
			}
		}
		
		changed = 0;
		for(i = 0; i < colour_optioncount; i++)
		{
			temp = colour_cb[i].checked;
			drawIcon(vid_buf, colour_cb[i].x + 16, colour_cb[i].y+2, colour_optionicons[i]);
			ui_radio_draw(vid_buf, &(colour_cb[i]));
			ui_radio_process(mx, my, b, bq, &(colour_cb[i]));
			if(colour_cb[i].checked)	//One colour only
			{
				for(j = 0; j < colour_optioncount; j++)
				{
					if(j!=i)
					{
						colour_cb[j].checked = 0;
					}
				}
			}
			if(colour_cb[i].focus)
				drawtext(vid_buf, xcoord - textwidth(colour_desc[i]) - 10, colour_cb[i].y+2, colour_desc[i], 255, 255, 255, 255);
			if(temp != colour_cb[i].checked)
				changed = 1;
		}
		if(changed)
		{
			//Compile colour options
			colour_mode = 0;
			for(i = 0; i < colour_optioncount; i++)
			{
				if(colour_cb[i].checked)
				{
					colour_mode |= colour_options[i];
				}
			}
		}
		
#ifdef OGLR
		clearScreenNP(1.0f);
		draw_parts_fbo();
#endif
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
		
		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
			break;
		if (b && !bq && (mx < xcoord || mx > xcoord+xsize || my < ycoord || my > ycoord+ysize))
			break;
	}
	
	free(colour_cb);
	
	free(render_cb);
	
	free(display_cb);
	
	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
	
	free(part_vbuf_store);
	
	free(o_vid_buf);
}

void simulation_ui(pixel * vid_buf)
{
	int xsize = 300;
	int ysize = 316;
	int x0=(XRES-xsize)/2,y0=(YRES-MENUSIZE-ysize)/2,b=1,bq,mx,my;
	int new_scale, new_kiosk, oldedgeMode = edgeMode;
	ui_checkbox cb, cb2, cb3, cb4, cb5, cb6, cb7;
	char * airModeList[] = {"On", "Pressure Off", "Velocity Off", "Off", "No Update"};
	int airModeListCount = 5;
	char * gravityModeList[] = {"Vertical", "Off", "Radial"};
	int gravityModeListCount = 3;
	char * edgeModeList[] = {"Void", "Solid", "Loop"};//, "Empty"};
	int edgeModeListCount = 3;
	char * updateList[] = {"No Update Check", "ip: 178.219.36.155", "website: mniip.com"};//, "Empty"};
	int updateListCount = 3;
	ui_list list;
	ui_list list2;
	ui_list list3;
	ui_list listUpdate;

	cb.x = x0+xsize-16;		//Heat simulation
	cb.y = y0+23;
	cb.focus = 0;
	cb.checked = !legacy_enable;

	cb2.x = x0+xsize-16;	//Newt. Gravity
	cb2.y = y0+79;
	cb2.focus = 0;
	cb2.checked = ngrav_enable;
	
	cb3.x = x0+xsize-16;	//Large window
	cb3.y = y0+255;
	cb3.focus = 0;
	cb3.checked = (sdl_scale==2)?1:0;
	
	cb4.x = x0+xsize-16;	//Fullscreen
	cb4.y = y0+269;
	cb4.focus = 0;
	cb4.checked = (kiosk_enable==1)?1:0;
	
	cb5.x = x0+xsize-16;	//Ambient heat
	cb5.y = y0+51;
	cb5.focus = 0;
	cb5.checked = aheat_enable;

	cb6.x = x0+xsize-16;	//Water equalisation
	cb6.y = y0+107;
	cb6.focus = 0;
	cb6.checked = water_equal_test;

	cb7.x = x0+xsize-16;	//Fast Quit
	cb7.y = y0+283;
	cb7.focus = 0;
	cb7.checked = fastquit;
	
	list.x = x0+xsize-76;	//Air Mode
	list.y = y0+135;
	list.w = 72;
	list.h = 16;
	list.def = "[air mode]";
	list.selected = airMode;
	list.items = airModeList;
	list.count = airModeListCount;
	
	list2.x = x0+xsize-76;	//Gravity Mode
	list2.y = y0+163;
	list2.w = 72;
	list2.h = 16;
	list2.def = "[gravity mode]";
	list2.selected = gravityMode;
	list2.items = gravityModeList;
	list2.count = gravityModeListCount;

	list3.x = x0+xsize-76;	//Edge Mode
	list3.y = y0+191;
	list3.w = 72;
	list3.h = 16;
	list3.def = "[gravity mode]";
	list3.selected = edgeMode;
	list3.items = edgeModeList;
	list3.count = edgeModeListCount;

	listUpdate.x = x0+xsize-100;	//Edge Mode
	listUpdate.y = y0+219;
	listUpdate.w = 96;
	listUpdate.h = 16;
	listUpdate.def = "[update server]";
	listUpdate.selected = doUpdates;
	listUpdate.items = updateList;
	listUpdate.count = updateListCount;

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
	{
		bq = b;
		b = mouse_get_state(&mx, &my);

		clearrect(vid_buf, x0-2, y0-2, xsize+4, ysize+4);
		drawrect(vid_buf, x0, y0, xsize, ysize, 192, 192, 192, 255);
		drawtext(vid_buf, x0+8, y0+8, "Simulation options", 255, 216, 32, 255);

		drawtext(vid_buf, x0+8, y0+26, "Heat simulation", 255, 255, 255, 255);
		drawtext(vid_buf, x0+12+textwidth("Heat simulation"), y0+26, "Introduced in version 34.", 255, 255, 255, 180);
		drawtext(vid_buf, x0+12, y0+40, "Older saves may behave oddly with this enabled.", 255, 255, 255, 120);
		
		drawtext(vid_buf, x0+8, y0+54, "Ambient heat simulation", 255, 255, 255, 255);
		drawtext(vid_buf, x0+12+textwidth("Ambient heat simulation"), y0+54, "Introduced in version 50.", 255, 255, 255, 180);
		drawtext(vid_buf, x0+12, y0+68, "Older saves may behave oddly with this enabled.", 255, 255, 255, 120);

		drawtext(vid_buf, x0+8, y0+82, "Newtonian gravity", 255, 255, 255, 255);
		drawtext(vid_buf, x0+12+textwidth("Newtonian gravity"), y0+82, "Introduced in version 48.", 255, 255, 255, 180);
		drawtext(vid_buf, x0+12, y0+96, "May also cause slow performance on older computers", 255, 255, 255, 120);

		drawtext(vid_buf, x0+8, y0+110, "Water Equalization Test", 255, 255, 255, 255);
		drawtext(vid_buf, x0+12+textwidth("Water Equalization Test"), y0+110, "Introduced in version 61.", 255, 255, 255, 180);
		drawtext(vid_buf, x0+12, y0+124, "May lag with lots of water.", 255, 255, 255, 120);
		
		drawtext(vid_buf, x0+8, y0+138, "Air Simulation Mode \bg(y)", 255, 255, 255, 255);
		drawtext(vid_buf, x0+12, y0+152, "airMode", 255, 255, 255, 120);
		
		drawtext(vid_buf, x0+8, y0+166, "Gravity Simulation Mode \bg(w)", 255, 255, 255, 255);
		drawtext(vid_buf, x0+12, y0+180, "gravityMode", 255, 255, 255, 120);

		drawtext(vid_buf, x0+8, y0+194, "Edge Mode", 255, 255, 255, 255);
		drawtext(vid_buf, x0+12, y0+208, "edgeMode", 255, 255, 255, 120);

		drawtext(vid_buf, x0+8, y0+222, "Update Check", 255, 255, 255, 255);
		drawtext(vid_buf, x0+12, y0+236, "doUpdates", 255, 255, 255, 120);
		
		draw_line(vid_buf, x0, y0+250, x0+xsize, y0+250, 150, 150, 150, XRES+BARSIZE);
		
		drawtext(vid_buf, x0+8, y0+256, "Large window", 255, 255, 255, 255);
		drawtext(vid_buf, x0+12+textwidth("Large window"), y0+256, "Double window size for small screens", 255, 255, 255, 180);
		
		drawtext(vid_buf, x0+8, y0+270, "Fullscreen", 255, 255, 255, 255);
		drawtext(vid_buf, x0+12+textwidth("Fullscreen"), y0+270, "Fill the entire screen", 255, 255, 255, 180);

		drawtext(vid_buf, x0+8, y0+284, "Fast Quit", 255, 255, 255, 255);
		drawtext(vid_buf, x0+12+textwidth("Fast Quit"), y0+284, "Hitting 'X' will always exit out of tpt", 255, 255, 255, 180);

		drawtext(vid_buf, x0+5, y0+ysize-11, "OK", 255, 255, 255, 255);
		drawrect(vid_buf, x0, y0+ysize-16, xsize, 16, 192, 192, 192, 255);

		ui_checkbox_draw(vid_buf, &cb);
		ui_checkbox_draw(vid_buf, &cb2);
		ui_checkbox_draw(vid_buf, &cb3);
		ui_checkbox_draw(vid_buf, &cb4);
		ui_checkbox_draw(vid_buf, &cb5);
		ui_checkbox_draw(vid_buf, &cb6);
		ui_checkbox_draw(vid_buf, &cb7);
		ui_list_draw(vid_buf, &list);
		ui_list_draw(vid_buf, &list2);
		ui_list_draw(vid_buf, &list3);
		ui_list_draw(vid_buf, &listUpdate);
#ifdef OGLR
		clearScreen(1.0f);
#endif
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
		ui_checkbox_process(mx, my, b, bq, &cb);
		ui_checkbox_process(mx, my, b, bq, &cb2);
		ui_checkbox_process(mx, my, b, bq, &cb3);
		ui_checkbox_process(mx, my, b, bq, &cb4);
		ui_checkbox_process(mx, my, b, bq, &cb5);
		ui_checkbox_process(mx, my, b, bq, &cb6);
		ui_checkbox_process(mx, my, b, bq, &cb7);
		ui_list_process(vid_buf, mx, my, b, &list);
		ui_list_process(vid_buf, mx, my, b, &list2);
		ui_list_process(vid_buf, mx, my, b, &list3);
		ui_list_process(vid_buf, mx, my, b, &listUpdate);

		if (b && !bq && mx>=x0 && mx<x0+xsize && my>=y0+ysize-16 && my<=y0+ysize)
			break;

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
			break;
	}

	water_equal_test = cb6.checked;
	legacy_enable = !cb.checked;
	aheat_enable = cb5.checked;
	new_scale = (cb3.checked)?2:1;
	new_kiosk = (cb4.checked)?1:0;
	if(list.selected>=0 && list.selected<=4)
		airMode = list.selected;
	if(list2.selected>=0 && list2.selected<=2)
		gravityMode = list2.selected;
	if(new_scale!=sdl_scale || new_kiosk!=kiosk_enable)
		set_scale(new_scale, new_kiosk);
	if(ngrav_enable != cb2.checked)
	{
		if(cb2.checked)
			start_grav_async();
		else
			stop_grav_async();
	}
	edgeMode = list3.selected;
	if(edgeMode == 1 && oldedgeMode != 1)
		draw_bframe();
	else if(edgeMode != 1 && oldedgeMode == 1)
		erase_bframe();
	doUpdates = listUpdate.selected;
	fastquit = cb7.checked;

	while (!sdl_poll())
	{
		b = mouse_get_state(&mx, &my);
		if (!b)
			break;
	}
}

Uint8 mouse_get_state(int *x, int *y)
{
	//Wrapper around SDL_GetMouseState to divide by sdl_scale
	Uint8 sdl_b;
	int sdl_x, sdl_y;
	sdl_b = SDL_GetMouseState(&sdl_x, &sdl_y);
	*x = sdl_x/sdl_scale;
	*y = sdl_y/sdl_scale;
	return sdl_b;
}

void mouse_coords_window_to_sim(int *sim_x, int *sim_y, int window_x, int window_y)
{
	//Change mouse coords to take zoom window into account
	if (zoom_en && window_x>=zoom_wx && window_y>=zoom_wy
		&& window_x<(zoom_wx+ZFACTOR*ZSIZE)
		&& window_y<(zoom_wy+ZFACTOR*ZSIZE))
	{
		*sim_x = (((window_x-zoom_wx)/ZFACTOR)+zoom_x);
		*sim_y = (((window_y-zoom_wy)/ZFACTOR)+zoom_y);
	}
	else
	{
		*sim_x = window_x;
		*sim_y = window_y;
	}
}
