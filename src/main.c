/**
 * Powder Toy - Main source
 *
 * Copyright (c) 2008 - 2011 Stanislaw Skowronek.
 * Copyright (c) 2010 - 2011 Simon Robertshaw
 * Copyright (c) 2010 - 2011 Skresanov Savely
 * Copyright (c) 2010 - 2011 Bryan Hoyle
 * Copyright (c) 2010 - 2011 Nathan Cousins
 * Copyright (c) 2010 - 2011 cracker64
 * Copyright (c) 2011 jacksonmj
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#include <defines.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>
#include <bzlib.h>
#include <time.h>
#include <pthread.h>

#ifdef WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <misc.h>
#include <font.h>
#include <powder.h>
#include "gravity.h"
#include <graphics.h>
#include <powdergraphics.h>
#include <version.h>
#include <http.h>
#include <md5.h>
#include <update.h>
#include <hmap.h>
#include <air.h>
#include <icon.h>
#include <console.h>
#ifdef LUACONSOLE
#include "luaconsole.h"
#endif
#include "save.h"

pixel *vid_buf;

#define NUM_SOUNDS 2
struct sample {
	Uint8 *data;
	Uint32 dpos;
	Uint32 dlen;
} sounds[NUM_SOUNDS];

void mixaudio(void *unused, Uint8 *stream, int len)
{
	int i;
	Uint32 amount;

	for ( i=0; i<NUM_SOUNDS; ++i ) {
		amount = (sounds[i].dlen-sounds[i].dpos);
		if ( amount > len ) {
			amount = len;
		}
		SDL_MixAudio(stream, &sounds[i].data[sounds[i].dpos], amount, SDL_MIX_MAXVOLUME);
		sounds[i].dpos += amount;
	}
}

//plays a .wav file (sounds must be enabled)
void play_sound(char *file)
{
	int index;
	SDL_AudioSpec wave;
	Uint8 *data;
	Uint32 dlen;
	SDL_AudioCVT cvt;

	if (!sound_enable) return;

	/* Look for an empty (or finished) sound slot */
	for ( index=0; index<NUM_SOUNDS; ++index ) {
		if ( sounds[index].dpos == sounds[index].dlen ) {
			break;
		}
	}
	if ( index == NUM_SOUNDS )
		return;

	/* Load the sound file and convert it to 16-bit stereo at 22kHz */
	if ( SDL_LoadWAV(file, &wave, &data, &dlen) == NULL ) {
		fprintf(stderr, "Couldn't load %s: %s\n", file, SDL_GetError());
		return;
	}
	SDL_BuildAudioCVT(&cvt, wave.format, wave.channels, wave.freq,
	                  AUDIO_S16,   2,             22050);
	cvt.buf = malloc(dlen*cvt.len_mult);
	memcpy(cvt.buf, data, dlen);
	cvt.len = dlen;
	SDL_ConvertAudio(&cvt);
	SDL_FreeWAV(data);

	/* Put the sound data in the slot (it starts playing immediately) */
	if ( sounds[index].data ) {
		free(sounds[index].data);
	}
	SDL_LockAudio();
	sounds[index].data = cvt.buf;
	sounds[index].dlen = cvt.len_cvt;
	sounds[index].dpos = 0;
	SDL_UnlockAudio();
}

static const char *it_msg =
    "\blThe Powder Toy - Version " MTOS(SAVE_VERSION) "." MTOS(MINOR_VERSION) " - http://powdertoy.co.uk, irc.freenode.net #powder\n"
    "\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\n"
    "\n"
    "\bgControl+C/V/X are Copy, Paste and cut respectively.\n"
    "\bgTo choose a material, hover over one of the icons on the right, it will show a selection of elements in that group.\n"
    "\bgPick your material from the menu using mouse left/right buttons.\n"
    "Draw freeform lines by dragging your mouse left/right button across the drawing area.\n"
    "Shift+drag will create straight lines of particles.\n"
    "Ctrl+drag will result in filled rectangles.\n"
    "Ctrl+Shift+click will flood-fill a closed area.\n"
    "Ctrl+Z will act as Undo.\n"
    "Middle click or Alt+Click to \"sample\" the particles.\n"
    "\n\boUse 'Z' for a zoom tool. Click to make the drawable zoom window stay around. Use the wheel to change the zoom strength\n"
    "Use 'S' to save parts of the window as 'stamps'.\n"
    "'L' will load the most recent stamp, 'K' shows a library of stamps you saved.\n"
    "'C' will cycle the display mode (Fire, Blob, Velocity, etc.). The numbers on the keyboard do the same\n"
    "Use the mouse scroll wheel to change the tool size for particles.\n"
    "The spacebar can be used to pause physics.\n"
    "'P' will take a screenshot and save it into the current directory.\n"
    "\n"
    "Contributors: \bgStanislaw K Skowronek (\brhttp://powder.unaligned.org\bg, \bbirc.unaligned.org #wtf\bg),\n"
    "\bgSimon Robertshaw, Skresanov Savely, cracker64, Catelite, Bryan Hoyle, Nathan Cousins, jacksonmj,\n"
	"\bgLieuwe Mosch, Anthony Boot, Matthew \"me4502\", MaksProg\n"
    "\n"
    "\bgTo use online features such as saving, you need to register at: \brhttp://powdertoy.co.uk/Register.html"
    ;

typedef struct
{
	int start, inc;
	pixel *vid;
} upstruc;

static const char *old_ver_msg_beta = "A new beta is available - click here!";
static const char *old_ver_msg = "A new version is available - click here!";
char new_message_msg[255];
float mheat = 0.0f;

int do_open = 0;
int sys_pause = 0;
int sys_shortcuts = 1;
int legacy_enable = 0; //Used to disable new features such as heat, will be set by save.
int aheat_enable; //Ambient heat
int decorations_enable = 1;
int hud_enable = 1;
int active_menu = 0;
int framerender = 0;
int pretty_powder = 0;
int amd = 1;
int FPSB = 0;
float FPSB2 = 0;
int MSIGN =-1;
int frameidx = 0;
//int CGOL = 0;
//int GSPEED = 1;//causes my .exe to crash..
int sound_enable = 0;
int favMenu[19] = {300,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17};
int alt_hud = 1;
int finding = 0;
int locked = 0;
int highesttemp = MAX_TEMP;
int lowesttemp = MIN_TEMP;
int heatmode = 0;
int maxframes = 25;
int secret_els = 0;
int save_as = 0;
int hud_modnormal[HUD_OPTIONS] = {1,0,1,0,0,0,0,0,1,0,1,0,0,0,0,1,0,0,2,0,0,0,0,2,0,2,1,2,0,0,0,2,0,2,0,2,0,0,0,0,0,0,2};
int hud_moddebug[HUD_OPTIONS] =  {1,1,1,2,1,0,0,0,1,0,1,1,1,0,0,1,1,0,4,1,0,0,0,4,0,4,1,4,1,1,1,4,0,4,0,4,0,0,0,0,0,0,4};
int hud_normal[HUD_OPTIONS] =    {0,0,1,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,2,0,0,0,0,2,0,2,1,2,0,0,0,2,0,2,0,2,0,0,0,0,0,0,2};
int hud_debug[HUD_OPTIONS] =     {0,1,1,0,1,0,1,1,1,1,1,1,0,1,1,1,0,0,2,1,1,0,0,2,0,2,1,2,1,1,1,2,0,2,0,2,0,0,0,0,0,0,2};
int hud_current[HUD_OPTIONS];

int drawinfo = 0;
int currentTime = 0;
int totaltime = 0;
int totalafktime = 0;
int afktime = 0;
double totalfps = 0;
int frames = 0;
double maxfps = 0;
int prevafktime = 0;
int timesplayed = 0;

int debug_flags = 0;
int debug_perf_istart = 1;
int debug_perf_iend = 0;
long debug_perf_frametime[DEBUG_PERF_FRAMECOUNT];
long debug_perf_partitime[DEBUG_PERF_FRAMECOUNT];
long debug_perf_time = 0;

sign signs[MAXSIGNS];

int numCores = 4;

int core_count()
{
	int numCPU = 1;
#ifdef MT
#ifdef WIN32
	SYSTEM_INFO sysinfo;
	GetSystemInfo( &sysinfo );
	numCPU = sysinfo.dwNumberOfProcessors;
#else
#ifdef MACOSX
	numCPU = 4;
#else
	numCPU = sysconf( _SC_NPROCESSORS_ONLN );
#endif
#endif

	printf("Cpus: %d\n", numCPU);
	if (numCPU>1)
		printf("Multithreading enabled\n");
	else
		printf("Multithreading disabled\n");
#endif
	return numCPU;
}

int mousex = 0, mousey = 0, lastx = 0, lasty = 0;  //They contain mouse position
int kiosk_enable = 0;

void sdl_seticon(void)
{
#ifdef WIN32
	//SDL_Surface *icon = SDL_CreateRGBSurfaceFrom(app_icon_w32, 32, 32, 32, 128, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	//SDL_WM_SetIcon(icon, NULL/*app_icon_mask*/);
#else
#ifdef MACOSX
	//SDL_Surface *icon = SDL_CreateRGBSurfaceFrom(app_icon_w32, 32, 32, 32, 128, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	//SDL_WM_SetIcon(icon, NULL/*app_icon_mask*/);
#else
	SDL_Surface *icon = SDL_CreateRGBSurfaceFrom(app_icon, 16, 16, 32, 64, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	SDL_WM_SetIcon(icon, NULL/*app_icon_mask*/);
#endif
#endif
}

int frame_idx=0;
void dump_frame(pixel *src, int w, int h, int pitch)
{
	char frame_name[32];
	int j,i;
	unsigned char c[3];
	FILE *f;
	sprintf(frame_name,"frame%04d.ppm",frame_idx);
	f=fopen(frame_name,"wb");
	fprintf(f,"P6\n%d %d\n255\n",w,h);
	for (j=0; j<h; j++)
	{
		for (i=0; i<w; i++)
		{
			c[0] = PIXR(src[i]);
			c[1] = PIXG(src[i]);
			c[2] = PIXB(src[i]);
			fwrite(c,3,1,f);
		}
		src+=pitch;
	}
	fclose(f);
	frame_idx++;
}

int invalid_element(int save_as, int el)
{
	if (save_as > 0 && (el >= PT_NORMAL_NUM || ptypes[el].enabled == 0))
		return 1;
	//if (save_as > 1 && (el == PT_ELEC || el == PT_FIGH || el == PT_ACEL || el == PT_DCEL || el == PT_BANG || el == PT_IGNT))
	//	return 1;
	return 0;
}

int check_save(int save_as)
{
	int i;
	for (i=0; i<NPART; i++)
	{
		if (invalid_element(save_as,parts[i].type))
		{
			char errortext[256] = "";
			sprintf(errortext,"Found %s at X:%i Y:%i, cannot save",ptypes[parts[i].type].name,(int)(parts[i].x+.5),(int)(parts[i].y+.5));
			info_ui(vid_buf,"Error",errortext);
			return 1;
		}
		if ((parts[i].type == PT_CLNE || parts[i].type == PT_PCLN || parts[i].type == PT_BCLN || parts[i].type == PT_PBCN || parts[i].type == PT_STOR || parts[i].type == PT_CONV || parts[i].type == PT_STKM || parts[i].type == PT_STKM2 || parts[i].type == PT_FIGH || parts[i].type == PT_LAVA) && invalid_element(save_as,parts[i].ctype))
		{
			char errortext[256] = "";
			sprintf(errortext,"Found %s at X:%i Y:%i, cannot save",ptypes[parts[i].ctype].name,(int)(parts[i].x+.5),(int)(parts[i].y+.5));
			info_ui(vid_buf,"Error",errortext);
			return 1;
		}
		if (parts[i].type == PT_PIPE && invalid_element(save_as,parts[i].tmp))
		{
			char errortext[256] = "";
			sprintf(errortext,"Found %s at X:%i Y:%i, cannot save",ptypes[parts[i].tmp].name,(int)(parts[i].x+.5),(int)(parts[i].y+.5));
			info_ui(vid_buf,"Error",errortext);
			return 1;
		}
	}
	return 0;
}

void clear_sim(void)
{
	int i,x, y;
	for (i=0; i<NPART; i++)
	{
		if (parts[i].animations)
		{
			free(parts[i].animations);
			parts[i].animations = NULL;
		}
	}
	memset(bmap, 0, sizeof(bmap));
	memset(emap, 0, sizeof(emap));
	memset(signs, 0, sizeof(signs));
	memset(parts, 0, sizeof(particle)*NPART);
	for (i=0; i<NPART-1; i++)
		parts[i].life = i+1;
	parts[NPART-1].life = -1;
	pfree = 0;
	parts_lastActiveIndex = 0;
	memset(pmap, 0, sizeof(pmap));
	memset(pv, 0, sizeof(pv));
	memset(vx, 0, sizeof(vx));
	memset(vy, 0, sizeof(vy));
	memset(fvx, 0, sizeof(fvx));
	memset(fvy, 0, sizeof(fvy));
	memset(photons, 0, sizeof(photons));
	memset(wireless, 0, sizeof(wireless));
	memset(gol2, 0, sizeof(gol2));
	memset(portalp, 0, sizeof(portalp));
	memset(fighters, 0, sizeof(fighters));
	fighcount = 0;
	ISSPAWN1 = ISSPAWN2 = 0;
	player.spwn = 0;
	player2.spwn = 0;
	memset(pers_bg, 0, (XRES+BARSIZE)*YRES*PIXELSIZE);
	memset(fire_r, 0, sizeof(fire_r));
	memset(fire_g, 0, sizeof(fire_g));
	memset(fire_b, 0, sizeof(fire_b));
	if(gravmask)
		memset(gravmask, 0xFFFFFFFF, (XRES/CELL)*(YRES/CELL)*sizeof(unsigned));
	if(gravy)
		memset(gravy, 0, (XRES/CELL)*(YRES/CELL)*sizeof(float));
	if(gravx)
		memset(gravx, 0, (XRES/CELL)*(YRES/CELL)*sizeof(float));
	if(gravp)
		memset(gravp, 0, (XRES/CELL)*(YRES/CELL)*sizeof(float));
	for(x = 0; x < XRES/CELL; x++){
		for(y = 0; y < YRES/CELL; y++){
			hv[y][x] = 273.15f+22.0f; //Set to room temperature
		}
	}
	numballs = 0;
	memset(msindex, 0, sizeof(msindex));
	memset(msnum, 0, sizeof(msnum));
	memset(msvx, 0, sizeof(msvx));
	memset(msvy, 0, sizeof(msvy));
	memset(msrotation, 0, sizeof(msrotation));
}

// stamps library

stamp stamps[STAMP_MAX];//[STAMP_X*STAMP_Y];

int stamp_count = 0;

unsigned last_time=0, last_name=0;
void stamp_gen_name(char *fn)
{
	unsigned t=(unsigned)time(NULL);

	if (last_time!=t)
	{
		last_time=t;
		last_name=0;
	}
	else
		last_name++;

	sprintf(fn, "%08x%02x", last_time, last_name);
}

void stamp_update(void)
{
	FILE *f;
	int i;
	f=fopen("stamps" PATH_SEP "stamps.def", "wb");
	if (!f)
		return;
	for (i=0; i<STAMP_MAX; i++)
	{
		if (!stamps[i].name[0])
			break;
		if (stamps[i].dodelete!=1)
		{
			fwrite(stamps[i].name, 1, 10, f);
		}
		else
		{
			char name[30] = {0};
			sprintf(name,"stamps%s%s.stm",PATH_SEP,stamps[i].name);
			remove(name);
		}
	}
	fclose(f);
}

void stamp_gen_thumb(int i)
{
	char fn[64];
	void *data;
	int size, factor_x, factor_y;
	pixel *tmp;

	if (stamps[i].thumb)
	{
		free(stamps[i].thumb);
		stamps[i].thumb = NULL;
	}

	sprintf(fn, "stamps" PATH_SEP "%s.stm", stamps[i].name);
	data = file_load(fn, &size);

	if (data)
	{
		stamps[i].thumb = prerender_save(data, size, &(stamps[i].thumb_w), &(stamps[i].thumb_h));
		if (stamps[i].thumb && (stamps[i].thumb_w>XRES/GRID_S || stamps[i].thumb_h>YRES/GRID_S))
		{
			factor_x = (int)ceil((float)stamps[i].thumb_w/(float)(XRES/GRID_S));
			factor_y = (int)ceil((float)stamps[i].thumb_h/(float)(YRES/GRID_S));
			if (factor_y > factor_x)
				factor_x = factor_y;
			tmp = rescale_img(stamps[i].thumb, stamps[i].thumb_w, stamps[i].thumb_h, &(stamps[i].thumb_w), &(stamps[i].thumb_h), factor_x);
			free(stamps[i].thumb);
			stamps[i].thumb = tmp;
		}
	}

	free(data);
}

int clipboard_ready = 0;
void *clipboard_data = 0;
int clipboard_length = 0;

void stamp_save(int x, int y, int w, int h)
{
	FILE *f;
	int n;
	char fn[64], sn[16];
	void *s=build_save(&n, x, y, w, h, bmap, vx, vy, pv, fvx, fvy, signs, parts);
	if (!s)
		return;

#ifdef WIN32
	_mkdir("stamps");
#else
	mkdir("stamps", 0755);
#endif

	stamp_gen_name(sn);
	sprintf(fn, "stamps" PATH_SEP "%s.stm", sn);

	f = fopen(fn, "wb");
	if (!f)
		return;
	fwrite(s, n, 1, f);
	fclose(f);

	free(s);

	if (stamps[STAMP_MAX-1].thumb)
		free(stamps[STAMP_MAX-1].thumb);
	memmove(stamps+1, stamps, sizeof(struct stamp)*(STAMP_MAX-1));
	memset(stamps, 0, sizeof(struct stamp));
	if (stamp_count<STAMP_MAX)
		stamp_count++;

	strcpy(stamps[0].name, sn);
	stamp_gen_thumb(0);

	stamp_update();
}

void *stamp_load(int i, int *size)
{
	void *data;
	char fn[64];
	struct stamp tmp;

	if (!stamps[i].thumb || !stamps[i].name[0])
		return NULL;

	sprintf(fn, "stamps" PATH_SEP "%s.stm", stamps[i].name);
	data = file_load(fn, size);
	if (!data)
		return NULL;

	if (i>0)
	{
		memcpy(&tmp, stamps+i, sizeof(struct stamp));
		memmove(stamps+1, stamps, sizeof(struct stamp)*i);
		memcpy(stamps, &tmp, sizeof(struct stamp));

		stamp_update();
	}

	return data;
}

void stamp_init(void)
{
	int i;
	FILE *f;

	memset(stamps, 0, sizeof(stamps));

	f=fopen("stamps" PATH_SEP "stamps.def", "rb");
	if (!f)
		return;
	for (i=0; i<STAMP_MAX; i++)
	{
		fread(stamps[i].name, 1, 10, f);
		if (!stamps[i].name[0])
			break;
		stamp_count++;
		stamp_gen_thumb(i);
	}
	fclose(f);
}

void del_stamp(int d)
{
	stamps[d].dodelete = 1;
	stamp_update();
	stamp_count = 0;
	stamp_init();
}

void thumb_cache_inval(char *id);

char *thumb_cache_id[THUMB_CACHE_SIZE];
void *thumb_cache_data[THUMB_CACHE_SIZE];
int thumb_cache_size[THUMB_CACHE_SIZE];
int thumb_cache_lru[THUMB_CACHE_SIZE];

void thumb_cache_inval(char *id)
{
	int i,j;
	for (i=0; i<THUMB_CACHE_SIZE; i++)
		if (thumb_cache_id[i] && !strcmp(id, thumb_cache_id[i]))
			break;
	if (i >= THUMB_CACHE_SIZE)
		return;
	free(thumb_cache_id[i]);
	free(thumb_cache_data[i]);
	thumb_cache_id[i] = NULL;
	for (j=0; j<THUMB_CACHE_SIZE; j++)
		if (thumb_cache_lru[j] > thumb_cache_lru[i])
			thumb_cache_lru[j]--;
}
void thumb_cache_add(char *id, void *thumb, int size)
{
	int i,m=-1,j=-1;
	thumb_cache_inval(id);
	for (i=0; i<THUMB_CACHE_SIZE; i++)
	{
		if (!thumb_cache_id[i])
			break;
		if (thumb_cache_lru[i] > m)
		{
			m = thumb_cache_lru[i];
			j = i;
		}
	}
	if (i >= THUMB_CACHE_SIZE)
	{
		thumb_cache_inval(thumb_cache_id[j]);
		i = j;
	}
	for (j=0; j<THUMB_CACHE_SIZE; j++)
		thumb_cache_lru[j] ++;
	thumb_cache_id[i] = mystrdup(id);
	thumb_cache_data[i] = malloc(size);
	memcpy(thumb_cache_data[i], thumb, size);
	thumb_cache_size[i] = size;
	thumb_cache_lru[i] = 0;
}
int thumb_cache_find(char *id, void **thumb, int *size)
{
	int i,j;
	for (i=0; i<THUMB_CACHE_SIZE; i++)
		if (thumb_cache_id[i] && !strcmp(id, thumb_cache_id[i]))
			break;
	if (i >= THUMB_CACHE_SIZE)
		return 0;
	for (j=0; j<THUMB_CACHE_SIZE; j++)
		if (thumb_cache_lru[j] < thumb_cache_lru[i])
			thumb_cache_lru[j]++;
	thumb_cache_lru[i] = 0;
	*thumb = malloc(thumb_cache_size[i]);
	*size = thumb_cache_size[i];
	memcpy(*thumb, thumb_cache_data[i], *size);
	return 1;
}

char http_proxy_string[256] = "";

unsigned char last_major=0, last_minor=0, last_build=0, update_flag=0;

char *tag = "(c) 2008-9 Stanislaw Skowronek";
int itc = 0;
char itc_msg[64] = "[?]";

char my_uri[] = "http://" SERVER "/Update.api?Action=Download&Architecture="
#if defined WIN32
                "Windows32"
#elif defined LIN32
                "Linux32"
#elif defined LIN64
                "Linux64"
#elif defined MACOSX
                "MacOSX"
#else
                "Unknown"
#endif
                "&InstructionSet="
#if defined X86_SSE3
                "SSE3"
#elif defined X86_SSE2
                "SSE2"
#elif defined X86_SSE
                "SSE"
#else
                "SSE"
#endif
                ;

int set_scale(int scale, int kiosk){
	int old_scale = sdl_scale, old_kiosk = kiosk_enable;
	sdl_scale = scale;
	kiosk_enable = kiosk;
	if (!sdl_open())
	{
		sdl_scale = old_scale;
		kiosk_enable = old_kiosk;
		sdl_open();
		return 0;
	}
	return 1;
}

void timestring(int currtime, char *string, int length)
{
	int years, days, hours, minutes, seconds, milliseconds;
	if (length == 0)
	{
		years = currtime/31557600000;
		currtime = currtime%31557600000;
		days = currtime/86400000;
		currtime = currtime%86400000;
	}
	if (length <= 1)
	{
		hours = currtime/3600000;
		currtime = currtime%3600000;
	}
	minutes = currtime/60000;
	currtime = currtime%60000;
	seconds = currtime/1000;
	currtime = currtime%1000;
	milliseconds = currtime;
	if (length == 0)
		sprintf(string,"%i year%s, %i day%s, %i hour%s, %i minute%s, %i second%s, %i millisecond%s",years,(years == 1)?"":"s",days,(days == 1)?"":"s",hours,(hours == 1)?"":"s",minutes,(minutes == 1)?"":"s",seconds,(seconds == 1)?"":"s",milliseconds,(milliseconds == 1)?"":"s");
	else if (length == 1)
		sprintf(string,"%i hour%s, %i minute%s, %i second%s",hours,(hours == 1)?"":"s",minutes,(minutes == 1)?"":"s",seconds,(seconds == 1)?"":"s");
	else if (length == 2)
		sprintf(string,"%i minute%s, %i second%s",minutes,(minutes == 1)?"":"s",seconds,(seconds == 1)?"":"s");
}

#ifdef RENDERER
int main(int argc, char *argv[])
{
	pixel *vid_buf = calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	int load_size, i=0, j=0;
	void *load_data = file_load(argv[1], &load_size);
	unsigned char c[3];
	char ppmfilename[256], ptifilename[256], ptismallfilename[256];
	FILE *f;
	
	colour_mode = COLOUR_DEFAULT;
	init_display_modes();

	sys_pause = 1;
	parts = calloc(sizeof(particle), NPART);
	for (i=0; i<NPART-1; i++)
		parts[i].life = i+1;
	parts[NPART-1].life = -1;
	pfree = 0;

	pers_bg = calloc((XRES+BARSIZE)*YRES, PIXELSIZE);
	
	prepare_alpha(CELL, 1.0f);
	prepare_graphicscache();
	flm_data = generate_gradient(flm_data_colours, flm_data_pos, flm_data_points, 200);
	plasma_data = generate_gradient(plasma_data_colours, plasma_data_pos, plasma_data_points, 200);
	
	player.elem = player2.elem = PT_DUST;
	player.frames = player2.frames = 0;

	sprintf(ppmfilename, "%s.ppm", argv[2]);
	sprintf(ptifilename, "%s.pti", argv[2]);
	sprintf(ptismallfilename, "%s-small.pti", argv[2]);
	
	if(load_data && load_size){
		int parsestate = 0;
		//parsestate = parse_save(load_data, load_size, 1, 0, 0);
		parsestate = parse_save(load_data, load_size, 1, 0, 0, bmap, vx, vy, pv, fvx, fvy, signs, parts, pmap);
		
		for(i=0; i<30; i++){
			memset(vid_buf, 0, (XRES+BARSIZE)*YRES*PIXELSIZE);
			draw_walls(vid_buf);
			update_particles(vid_buf);
			render_parts(vid_buf);
			render_fire(vid_buf);
		}
		
		render_signs(vid_buf);
		
		if(parsestate>0){
			//return 0;
			info_box(vid_buf, "Save file invalid or from newer version");
		}

		//Save PTi images
		char * datares = NULL, *scaled_buf;
		int res = 0, sw, sh;
		datares = ptif_pack(vid_buf, XRES, YRES, &res);
		if(datares!=NULL){
			f=fopen(ptifilename, "wb");
			fwrite(datares, res, 1, f);
			fclose(f);
			free(datares);
			datares = NULL;
		}
		scaled_buf = resample_img(vid_buf, XRES, YRES, XRES/GRID_Z, YRES/GRID_Z);
		datares = ptif_pack(scaled_buf, XRES/GRID_Z, YRES/GRID_Z, &res);
		if(datares!=NULL){
			f=fopen(ptismallfilename, "wb");
			fwrite(datares, res, 1, f);
			fclose(f);
			free(datares);
			datares = NULL;
		}
		free(scaled_buf);
		//Save PPM image
		f=fopen(ppmfilename, "wb");
		fprintf(f,"P6\n%d %d\n255\n",XRES,YRES);
		for (j=0; j<YRES; j++)
		{
			for (i=0; i<XRES; i++)
			{
				c[0] = PIXR(vid_buf[i]);
				c[1] = PIXG(vid_buf[i]);
				c[2] = PIXB(vid_buf[i]);
				fwrite(c,3,1,f);
			}
			vid_buf+=XRES+BARSIZE;
		}
		fclose(f);
		
		return 1;
	}
	
	return 0;
}
#else
int main(int argc, char *argv[])
{
	pixel *part_vbuf; //Extra video buffer
	pixel *part_vbuf_store;
	char uitext[512] = "";
	char heattext[256] = "";
	char heattext2[256] = "";
	char coordtext[128] = "";
	char infotext[512] = "";
	char timeinfotext[512] = "";
	char tempstring[256] = "";
	int FPS = 0, pastFPS = 0, elapsedTime = 0; 
	void *http_ver_check, *http_session_check = NULL;
	char *ver_data=NULL, *check_data=NULL, *tmp;
	//char console_error[255] = "";
	int result, i, j, bq, bc = 0, fire_fc=0, do_check=0, do_s_check=0, old_version=0, http_ret=0,http_s_ret=0, major, minor, buildnum, is_beta = 0, old_ver_len, new_message_len=0, afk = 0, afkstart = 0;
#ifdef INTERNAL
	int vs = 0;
#endif
	int wavelength_gfx = 0;
	int x, y, line_x, line_y, b = 0, sl=1, sr=0, su=0, c, lb = 0, lx = 0, ly = 0, lm = 0;//, tx, ty;
	int da = 0, dae = 0, db = 0, it = 2047, mx, my, bsx = 2, bsy = 2, quickoptions_tooltip_fade_invert, it_invert = 0;
	float nfvx, nfvy;
	int load_mode=0, load_w=0, load_h=0, load_x=0, load_y=0, load_size=0;
	void *load_data=NULL;
	pixel *load_img=NULL;//, *fbi_img=NULL;
	int save_mode=0, save_x=0, save_y=0, save_w=0, save_h=0, copy_mode=0;
	unsigned int rgbSave = PIXRGB(127,0,0);
	SDL_AudioSpec fmt;
	int username_flash = 0, username_flash_t = 1;
#ifdef PTW32_STATIC_LIB
    pthread_win32_process_attach_np();
    pthread_win32_thread_attach_np();
#endif
	limitFPS = 60;
	vid_buf = calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	part_vbuf = calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE); //Extra video buffer
	part_vbuf_store = part_vbuf;
	pers_bg = calloc((XRES+BARSIZE)*YRES, PIXELSIZE);
	memcpy(hud_current,hud_modnormal,sizeof(hud_current));

	gravity_init();
	GSPEED = 1;

	/* Set 16-bit stereo audio at 22Khz */
	fmt.freq = 22050;
	fmt.format = AUDIO_S16;
	fmt.channels = 2;
	fmt.samples = 512;
	fmt.callback = mixaudio;
	fmt.userdata = NULL;

#ifdef MT
	numCores = core_count();
#endif
//TODO: Move out version stuff
	menu_count();
	parts = calloc(sizeof(particle), NPART);
	cb_parts = calloc(sizeof(particle), NPART);
	init_can_move();
	clear_sim();
	
#ifdef LUACONSOLE
	luacon_open();
#endif
	
	colour_mode = COLOUR_DEFAULT;
	init_display_modes();

	//fbi_img = render_packed_rgb(fbi, FBI_W, FBI_H, FBI_CMP);

	for (i=1; i<argc; i++)
	{
		if (!strncmp(argv[i], "ddir", 4) && i+1<argc)
		{
			chdir(argv[i+1]);
			i++;
		}
		else if (!strncmp(argv[i], "open", 4) && i+1<argc)
		{
			int size;
			void *file_data;
			file_data = file_load(argv[i+1], &size);
			if (file_data)
			{
				svf_last = file_data;
				svf_lsize = size;
				if(!parse_save(file_data, size, 1, 0, 0, bmap, fvx, fvy, vx, vy, pv, signs, parts, pmap))
				{
					it=0;
					svf_filename[0] = 0;
					svf_fileopen = 1;
				} else {
					svf_last = NULL;
					svf_lsize = 0;
					free(file_data);
					file_data = NULL;
				}
			}
			i++;
		}

	}
	
	load_presets();
	timesplayed = timesplayed + 1;
	if (alt_hud == 1)
	{
		if (DEBUG_MODE)
			memcpy(hud_current,hud_moddebug,sizeof(hud_current));
		else
			memcpy(hud_current,hud_modnormal,sizeof(hud_current));
	}
	else
	{
		if (DEBUG_MODE)
			memcpy(hud_current,hud_debug,sizeof(hud_current));
		else
			memcpy(hud_current,hud_normal,sizeof(hud_current));
	}

	for (i=1; i<argc; i++)
	{
		if (!strncmp(argv[i], "scale:", 6))
		{
			sdl_scale = (argv[i][6]=='2') ? 2 : 1;
		}
		else if (!strncmp(argv[i], "proxy:", 6))
		{
			memset(http_proxy_string, 0, sizeof(http_proxy_string));
			strncpy(http_proxy_string, argv[i]+6, 255);
		}
		else if (!strncmp(argv[i], "nohud", 5))
		{
			hud_enable = 0;
		}
		else if (!strncmp(argv[i], "kiosk", 5))
		{
			kiosk_enable = 1;
			//sdl_scale = 2; //Removed because some displays cannot handle the resolution
			hud_enable = 0;
		}
		else if (!strncmp(argv[i], "sound", 5))
		{
			/* Open the audio device and start playing sound! */
			if ( SDL_OpenAudio(&fmt, NULL) < 0 )
			{
				fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
			}
			else
			{
				sound_enable = 1;
				SDL_PauseAudio(0);
			}
		}
		else if (!strncmp(argv[i], "scripts", 5))
		{
			file_script = 1;
		}
		else if (!strncmp(argv[i], "open", 4) && i+1<argc)
		{
			i++;
		}
		else if (!strncmp(argv[i], "ddir", 4) && i+1<argc)
		{
			i++;
		}

	}

	make_kernel();

	stamp_init();

	if (!sdl_open())
	{
		sdl_scale = 1;
		kiosk_enable = 0;
		if (!sdl_open()) exit(1);
	}
	save_presets(0);
	http_init(http_proxy_string[0] ? http_proxy_string : NULL);

	prepare_alpha(CELL, 1.0f);
	prepare_graphicscache();
	flm_data = generate_gradient(flm_data_colours, flm_data_pos, flm_data_points, 200);
	plasma_data = generate_gradient(plasma_data_colours, plasma_data_pos, plasma_data_points, 200);

	if (cpu_check())
	{
		error_ui(vid_buf, 0, "Unsupported CPU. Try another version.");
		return 1;
	}

	http_ver_check = http_async_req_start(NULL, "http://" SERVER "/Update.api?Action=CheckVersion", NULL, 0, 0);
	if (svf_login) {
		http_auth_headers(http_ver_check, svf_user_id, NULL, svf_session_id); //Add authentication so beta checking can be done from user basis
		http_session_check = http_async_req_start(NULL, "http://" SERVER "/Login.api?Action=CheckSession", NULL, 0, 0);
		http_auth_headers(http_session_check, svf_user_id, NULL, svf_session_id);
	}
#ifdef LUACONSOLE
	luacon_eval("dofile(\"autorun.lua\")"); //Autorun lua script
#endif
	while (!sdl_poll()) //the main loop
	{
		frameidx++;
		frameidx %= 30;
		if (!sys_pause||framerender) //only update air if not paused
		{
			update_air();
			if(aheat_enable)
				update_airh();
		}

#ifdef OGLR
		part_vbuf = vid_buf;
#else
		if(ngrav_enable && display_mode & DISPLAY_WARP)
		{
			part_vbuf = part_vbuf_store;
			memset(vid_buf, 0, (XRES+BARSIZE)*YRES*PIXELSIZE);
		} else {
			part_vbuf = vid_buf;
		}
#endif

		if(gravwl_timeout)
		{
			if(gravwl_timeout==1)
				gravity_mask();
			gravwl_timeout--;
		}
#ifdef OGLR
		if (display_mode & DISPLAY_PERS)//save background for persistent, then clear
		{
			clearScreen(0.01f);
			memset(part_vbuf, 0, (XRES+BARSIZE)*YRES*PIXELSIZE);
        }
		else //clear screen every frame
		{
            clearScreen(1.0f);
			memset(part_vbuf, 0, (XRES+BARSIZE)*YRES*PIXELSIZE);
			if (display_mode & DISPLAY_AIR)//air only gets drawn in these modes
			{
				draw_air(part_vbuf);
			}
		}
#else
		if (display_mode & DISPLAY_AIR)//air only gets drawn in these modes
		{
			draw_air(part_vbuf);
		}
		else if (display_mode & DISPLAY_PERS)//save background for persistent, then clear
		{
			memcpy(part_vbuf, pers_bg, (XRES+BARSIZE)*YRES*PIXELSIZE);
			memset(part_vbuf+((XRES+BARSIZE)*YRES), 0, ((XRES+BARSIZE)*YRES*PIXELSIZE)-((XRES+BARSIZE)*YRES*PIXELSIZE));
        }
		else //clear screen every frame
		{
			memset(part_vbuf, 0, (XRES+BARSIZE)*YRES*PIXELSIZE);
		}
#endif

		//Can't be too sure (Limit the cursor size)
		if (bsx>1180)
			bsx = 1180;
		if (bsx<0)
			bsx = 0;
		if (bsy>1180)
			bsy = 1180;
		if (bsy<0)
			bsy = 0;
			
		//Pretty powders, colour cycle
		//sandcolour_r = 0;
		//sandcolour_g = 0;
		sandcolour_b = sandcolour_r = sandcolour_g = (int)(20.0f*sin((float)sandcolour_frame*(M_PI/180.0f)));
		sandcolour_frame++;
		sandcolour_frame%=360;
		
		if(ngrav_enable && drawgrav_enable)
			draw_grav(vid_buf);
		draw_walls(part_vbuf);
		
		if(debug_flags & (DEBUG_PERFORMANCE_CALC|DEBUG_PERFORMANCE_FRAME))
		{
			#ifdef WIN32
			#elif defined(MACOSX)
			#else
				struct timespec ts;
				clock_gettime(CLOCK_REALTIME, &ts);
				debug_perf_time = ts.tv_nsec;
			#endif
		}
		
		update_particles(part_vbuf); //update everything
			
		if(debug_flags & (DEBUG_PERFORMANCE_CALC|DEBUG_PERFORMANCE_FRAME))
		{
			#ifdef WIN32
			#elif defined(MACOSX)
			#else
				struct timespec ts;
				clock_gettime(CLOCK_REALTIME, &ts);
				
				debug_perf_partitime[debug_perf_iend]  = ts.tv_nsec - debug_perf_time;
				
				debug_perf_time = ts.tv_nsec;
			#endif
		}
		
		render_parts(part_vbuf); //draw particles
		draw_other(part_vbuf);
		
		if(debug_flags & (DEBUG_PERFORMANCE_CALC|DEBUG_PERFORMANCE_FRAME))
		{
			#ifdef WIN32
			#elif defined(MACOSX)
			#else
				struct timespec ts;
				clock_gettime(CLOCK_REALTIME, &ts);
				
				debug_perf_frametime[debug_perf_iend]  = ts.tv_nsec - debug_perf_time;
			#endif
			debug_perf_iend++;
			debug_perf_iend %= DEBUG_PERF_FRAMECOUNT;
			debug_perf_istart++;
			debug_perf_istart %= DEBUG_PERF_FRAMECOUNT;
		}
		
		if(sl == WL_GRAV+100 || sr == WL_GRAV+100)
			draw_grav_zones(part_vbuf);
		
		gravity_update_async(); //Check for updated velocity maps from gravity thread
		if (!sys_pause||framerender) //Only update if not paused
			memset(gravmap, 0, (XRES/CELL)*(YRES/CELL)*sizeof(float)); //Clear the old gravmap

		if (framerender) {
			framerender = 0;
			sys_pause = 1;
		}

		if (display_mode & DISPLAY_PERS)
		{
			if (!fire_fc)//fire_fc has nothing to do with fire... it is a counter for diminishing persistent view every 3 frames
			{
				dim_copy_pers(pers_bg, vid_buf);
			}
			else
			{
				memcpy(pers_bg, vid_buf, (XRES+BARSIZE)*YRES*PIXELSIZE);
			}
			fire_fc = (fire_fc+1) % 3;
		}
		
#ifndef OGLR
		if (render_mode & FIREMODE)
			render_fire(part_vbuf);
#endif

		render_signs(part_vbuf);

#ifndef OGLR
		if(ngrav_enable && display_mode & DISPLAY_WARP)
			render_gravlensing(part_vbuf, vid_buf);
#endif

		memset(vid_buf+((XRES+BARSIZE)*YRES), 0, (PIXELSIZE*(XRES+BARSIZE))*MENUSIZE);//clear menu areas
		clearrect(vid_buf, XRES-1, 0, BARSIZE+1, YRES);

		draw_svf_ui(vid_buf, sdl_mod & (KMOD_LCTRL|KMOD_RCTRL));
		
		if(debug_flags)
		{
			draw_debug_info(vid_buf, lm, lx, ly, x, y, line_x, line_y);
		}

		if (http_ver_check)
		{
			if (!do_check && http_async_req_status(http_ver_check))
			{
				ver_data = http_async_req_stop(http_ver_check, &http_ret, NULL);
				if (http_ret==200 && ver_data)
				{
					if (sscanf(ver_data, "%d.%d.%d.%d", &major, &minor, &is_beta, &buildnum)==4)
						if (buildnum>BUILD_NUM)
							old_version = 1;
						if (is_beta)
						{
							old_ver_len = textwidth((char*)old_ver_msg_beta);
						}
						else
						{
							old_ver_len = textwidth((char*)old_ver_msg);
						}
					free(ver_data);
				}
				http_ver_check = NULL;
			}
			do_check = (do_check+1) & 15;
		}
		if (http_session_check)
		{
			if (!do_s_check && http_async_req_status(http_session_check))
			{
				check_data = http_async_req_stop(http_session_check, &http_s_ret, NULL);
				if (http_ret==200 && check_data)
				{
					if (!strncmp(check_data, "EXPIRED", 7))
					{
						//Session expired
						printf("EXPIRED");
						strcpy(svf_user, "");
						strcpy(svf_pass, "");
						strcpy(svf_user_id, "");
						strcpy(svf_session_id, "");
						svf_login = 0;
						svf_own = 0;
						svf_admin = 0;
						svf_mod = 0;
						svf_messages = 0;
					}
					else if (!strncmp(check_data, "BANNED", 6))
					{
						//User banned
						printf("BANNED");
						strcpy(svf_user, "");
						strcpy(svf_pass, "");
						strcpy(svf_user_id, "");
						strcpy(svf_session_id, "");
						svf_login = 0;
						svf_own = 0;
						svf_admin = 0;
						svf_mod = 0;
						svf_messages = 0;
						error_ui(vid_buf, 0, "Unable to log in\nYour account has been suspended, consider reading the rules.");
					}
					else if (!strncmp(check_data, "OK", 2))
					{
						//Session valid
						if (strlen(check_data)>2) {
							//User is elevated
							if (!strncmp(check_data+3, "ADMIN", 5))
							{
								//Check for messages
								svf_messages = atoi(check_data+9);
								svf_admin = 1;
								svf_mod = 0;
							}
							else if (!strncmp(check_data+3, "MOD", 3))
							{
								//Check for messages
								svf_messages = atoi(check_data+7);
								svf_admin = 0;
								svf_mod = 1;
							} else {
								//Check for messages
								svf_messages = atoi(check_data+3);
								if (!strncmp(svf_user,"jacob1",10))
									svf_admin = 1;
							}
						}
					}
					else
					{
						//No idea, but log the user out anyway
						strcpy(svf_user, "");
						strcpy(svf_pass, "");
						strcpy(svf_user_id, "");
						strcpy(svf_session_id, "");
						svf_login = 0;
						svf_own = 0;
						svf_admin = 0;
						svf_mod = 0;
						svf_messages = 0;
					}
					save_presets(0);
					free(check_data);
				} else {
					//Unable to check session, YOU WILL BE TERMINATED
					strcpy(svf_user, "");
					strcpy(svf_pass, "");
					strcpy(svf_user_id, "");
					strcpy(svf_session_id, "");
					svf_login = 0;
					svf_own = 0;
					svf_admin = 0;
					svf_mod = 0;
					svf_messages = 0;
				}
				http_session_check = NULL;
			} else {
				clearrect(vid_buf, XRES-125+BARSIZE/*385*/, YRES+(MENUSIZE-16), 91, 14);
				drawrect(vid_buf, XRES-125+BARSIZE/*385*/, YRES+(MENUSIZE-16), 91, 14, 255, 255, 255, 255);
				drawtext(vid_buf, XRES-122+BARSIZE/*388*/, YRES+(MENUSIZE-13), "\x84", 255, 255, 255, 255);
				if (username_flash>30) {
					username_flash_t = -1;
					username_flash = 30;
				} else if (username_flash<0) {
					username_flash_t = 1;
					username_flash = 0;
				}
				username_flash += username_flash_t;
				if (svf_login)
					drawtext(vid_buf, XRES-104+BARSIZE/*406*/, YRES+(MENUSIZE-12), svf_user, 255, 255, 255, 175-(username_flash*5));
				else
					drawtext(vid_buf, XRES-104+BARSIZE/*406*/, YRES+(MENUSIZE-12), "[checking]", 255, 255, 255, 255);
			}
			do_s_check = (do_s_check+1) & 15;
		}
#ifdef LUACONSOLE
	if(sdl_key){
		if(!luacon_keyevent(sdl_key, sdl_mod, LUACON_KDOWN))
			sdl_key = 0;
	}
	if(sdl_rkey){
		if(!luacon_keyevent(sdl_rkey, sdl_mod, LUACON_KUP))
			sdl_rkey = 0;
	}
#endif
		if (sys_shortcuts==1)//all shortcuts can be disabled by python scripts
		{
			if (sdl_key=='q' || sdl_key==SDLK_ESCAPE)
			{
				if (confirm_ui(vid_buf, "You are about to quit", "Are you sure you want to quit?", "Quit"))
				{
					break;
				}
			}
			if (sdl_key=='i' && (sdl_mod & KMOD_CTRL))
			{
				if(confirm_ui(vid_buf, "Install Powder Toy", "You are about to install The Powder Toy", "Install"))
				{
					if(register_extension())
					{
						info_ui(vid_buf, "Install success", "Powder Toy has been installed!");
					}
					else
					{
						error_ui(vid_buf, 0, "Install failed - You may not have permission or you may be on a platform that does not support installation");
					}
				}
			}
			if (sdl_key=='f')
			{
				framerender = 1;
			}
			if ((sdl_key=='l' || sdl_key=='k') && stamps[0].name[0])
			{
				if (load_mode)
				{
					free(load_img);
					free(load_data);
					load_mode = 0;
					load_data = NULL;
					load_img = NULL;
				}
				if (it > 50)
					it = 50;
				if (sdl_key=='k' && stamps[1].name[0])
				{
					j = stamp_ui(vid_buf);
					if (j>=0)
						load_data = stamp_load(j, &load_size);
					else
						load_data = NULL;
				}
				else
					load_data = stamp_load(0, &load_size);
				if (load_data)
				{
					load_img = prerender_save(load_data, load_size, &load_w, &load_h);
					if (load_img)
						load_mode = 1;
					else
						free(load_data);
				}
			}
			if (sdl_key=='s' && ((sdl_mod & (KMOD_CTRL)) || !player2.spwn))
			{
				if (it > 50)
					it = 50;
				save_mode = 1;
			}
			if (sdl_key=='1')
			{
				if (sdl_mod & (KMOD_CTRL))
					display_mode = display_mode&DISPLAY_AIRV ? display_mode&!DISPLAY_AIRV:display_mode|DISPLAY_AIRV;
				else
					set_cmode(CM_VEL);
			}
			if (sdl_key=='2')
			{
				if (sdl_mod & (KMOD_CTRL))
					display_mode = display_mode&DISPLAY_AIRP ? display_mode&!DISPLAY_AIRP:display_mode|DISPLAY_AIRP;
				else
					set_cmode(CM_PRESS);
			}
			if (sdl_key=='3')
			{
				if (sdl_mod & (KMOD_CTRL))
				{
					display_mode = display_mode&DISPLAY_PERS ? display_mode&!DISPLAY_PERS:display_mode|DISPLAY_PERS;
					memset(pers_bg, 0, (XRES+BARSIZE)*YRES*PIXELSIZE);
				}
				else
					set_cmode(CM_PERS);
			}
			if (sdl_key=='4')
			{
				set_cmode(CM_FIRE);
			}
			if (sdl_key=='5')
			{
				if (sdl_mod & (KMOD_CTRL))
					display_mode = display_mode&DISPLAY_BLOB ? display_mode&!DISPLAY_BLOB:display_mode|DISPLAY_BLOB;
				else
					set_cmode(CM_BLOB);
			}
			if (sdl_key=='6')
			{
				if (sdl_mod & (KMOD_CTRL))
					colour_mode = colour_mode == COLOUR_HEAT ? 0:COLOUR_HEAT;
				else
					set_cmode(CM_HEAT);
			}
			if (sdl_key=='7')
			{
				if (sdl_mod & (KMOD_CTRL))
					display_mode = display_mode&DISPLAY_WARP ? display_mode&!DISPLAY_WARP:display_mode|DISPLAY_WARP;
				else
					set_cmode(CM_FANCY);
			}
			if (sdl_key=='8')
			{
				set_cmode(CM_NOTHING);
			}
			if (sdl_key=='9')
			{
				if (sdl_mod & (KMOD_CTRL))
					colour_mode = colour_mode == COLOUR_GRAD ? 0:COLOUR_GRAD;
				else
					set_cmode(CM_GRAD);
			}
			if (sdl_key=='0')
			{
				if (sdl_mod & (KMOD_CTRL))
					display_mode = display_mode&DISPLAY_AIRC ? display_mode&!DISPLAY_AIRC:display_mode|DISPLAY_AIRC;
				else
				set_cmode(CM_CRACK);
			}
			if (sdl_key=='1'&& (sdl_mod & (KMOD_SHIFT)) && DEBUG_MODE)
			{
				if (sdl_mod & (KMOD_CTRL))
					colour_mode = colour_mode == COLOUR_LIFE ? 0:COLOUR_LIFE;
				else
					set_cmode(CM_LIFE);
			}
			if (active_menu == SC_FAV2 && (sdl_mod & KMOD_RCTRL) && (sdl_mod & KMOD_RSHIFT))
			{
				active_menu = SC_CRACKER;
			}
			if (sdl_key==SDLK_TAB)
			{
				CURRENT_BRUSH =(CURRENT_BRUSH + 1)%BRUSH_NUM ;
			}
			if (sdl_key==SDLK_LEFTBRACKET) {
				if (sdl_zoom_trig)
				{
					ZSIZE -= 1;
					if (ZSIZE>60)
						ZSIZE = 60;
					if (ZSIZE<2)
						ZSIZE = 2;
					ZFACTOR = 256/ZSIZE;
				}
				else
				{
					if (sdl_mod & (KMOD_LALT|KMOD_RALT) && !(sdl_mod & (KMOD_SHIFT|KMOD_CTRL)))
					{
						bsx -= 1;
						bsy -= 1;
					}
					else if (sdl_mod & (KMOD_SHIFT) && !(sdl_mod & (KMOD_CTRL)))
					{
						bsx -= 1;
					}
					else if (sdl_mod & (KMOD_CTRL) && !(sdl_mod & (KMOD_SHIFT)))
					{
						bsy -= 1;
					}
					else
					{
						bsx = bsx - (int)ceil((bsx/5)+0.5f);
						bsy = bsy - (int)ceil((bsy/5)+0.5f);
					}
					if (bsx>1180)
						bsx = 1180;
					if (bsy>1180)
						bsy = 1180;
					if (bsx<0)
						bsx = 0;
					if (bsy<0)
						bsy = 0;
				}
			}
			if (sdl_key==SDLK_RIGHTBRACKET) {
				if (sdl_zoom_trig)
				{
					ZSIZE += 1;
					if (ZSIZE>60)
						ZSIZE = 60;
					if (ZSIZE<2)
						ZSIZE = 2;
					ZFACTOR = 256/ZSIZE;
				}
				else
				{
					if (sdl_mod & (KMOD_LALT|KMOD_RALT) && !(sdl_mod & (KMOD_SHIFT|KMOD_CTRL)))
					{
						bsx += 1;
						bsy += 1;
					}
					else if (sdl_mod & (KMOD_SHIFT) && !(sdl_mod & (KMOD_CTRL)))
					{
						bsx += 1;
					}
					else if (sdl_mod & (KMOD_CTRL) && !(sdl_mod & (KMOD_SHIFT)))
					{
						bsy += 1;
					}
					else
					{
						bsx = bsx + (int)ceil((bsx/5)+0.5f);
						bsy = bsy + (int)ceil((bsy/5)+0.5f);
					}
					if (bsx>1180)
						bsx = 1180;
					if (bsy>1180)
						bsy = 1180;
					if (bsx<0)
						bsx = 0;
					if (bsy<0)
						bsy = 0;
				}
			}
			if (sdl_key=='d' && ((sdl_mod & (KMOD_CTRL)) || !player2.spwn))
			{
				if (alt_hud == 1)
				{
					if (DEBUG_MODE)
						memcpy(hud_current,hud_modnormal,sizeof(hud_current));
					else
						memcpy(hud_current,hud_moddebug,sizeof(hud_current));
				}
				else
				{
					if (DEBUG_MODE)
						memcpy(hud_current,hud_normal,sizeof(hud_current));
					else
						memcpy(hud_current,hud_debug,sizeof(hud_current));
				}
				DEBUG_MODE = !DEBUG_MODE;
			}
			if (sdl_key=='i')
			{
				int nx, ny;
				for (nx = 0; nx<XRES/CELL; nx++)
					for (ny = 0; ny<YRES/CELL; ny++)
					{
						pv[ny][nx] = -pv[ny][nx];
						vx[ny][nx] = -vx[ny][nx];
						vy[ny][nx] = -vy[ny][nx];
					}
			}
			if (sdl_key==SDLK_INSERT || sdl_key == SDLK_SEMICOLON)// || sdl_key==SDLK_BACKQUOTE)
				REPLACE_MODE = !REPLACE_MODE;
			if (sdl_key==SDLK_BACKQUOTE)
			{
				console_mode = !console_mode;
				//hud_enable = !console_mode;
			}
			if (sdl_key=='b')
			{
				if (sdl_mod & KMOD_CTRL)
				{
					decorations_enable = !decorations_enable;
					itc = 51;
					if (decorations_enable) strcpy(itc_msg, "Decorations layer: On");
					else strcpy(itc_msg, "Decorations layer: Off");
				}
				else
				{
					rgbSave = decorations_ui(vid_buf,&bsx,&bsy,rgbSave);//decoration_mode = !decoration_mode;
					decorations_enable = 1;
					sys_pause=1;
				}
			}
			if (sdl_key=='g')
			{
				if(sdl_mod & (KMOD_CTRL))
				{
					drawgrav_enable =! drawgrav_enable;
				}
				else
				{
					if (sdl_mod & (KMOD_SHIFT))
						GRID_MODE = (GRID_MODE+9)%10;
					else
						GRID_MODE = (GRID_MODE+1)%10;
				}
			}
			if (sdl_key=='m')
			{
				if(sl!=sr)
				{
					sl ^= sr;
					sr ^= sl;
					sl ^= sr;
				}
				dae = 51;
			}
			if (sdl_key=='=')
			{
				int nx, ny;
				if (sdl_mod & (KMOD_CTRL))
				{
					for (i=0; i<NPART; i++)
						if (parts[i].type==PT_SPRK)
						{
							parts[i].type = parts[i].ctype;
							parts[i].life = 0;
						}
				}
				else
				{
					for (nx = 0; nx<XRES/CELL; nx++)
						for (ny = 0; ny<YRES/CELL; ny++)
						{
							pv[ny][nx] = 0;
							vx[ny][nx] = 0;
							vy[ny][nx] = 0;
						}
				}
			}

			if (sdl_key=='w' && (!player2.spwn || (sdl_mod & (KMOD_SHIFT)))) //Gravity, by Moach
			{
				++gravityMode; // cycle gravity mode
				itc = 51;

				switch (gravityMode)
				{
				default:
					gravityMode = 0;
				case 0:
					strcpy(itc_msg, "Gravity: Vertical");
					break;
				case 1:
					strcpy(itc_msg, "Gravity: Off");
					break;
				case 2:
					strcpy(itc_msg, "Gravity: Radial");
					break;

				}
			}
			if (sdl_key=='y')
			{
				++airMode;
				itc = 52;

				switch (airMode)
				{
				default:
					airMode = 0;
				case 0:
					strcpy(itc_msg, "Air: On");
					break;
				case 1:
					strcpy(itc_msg, "Air: Pressure Off");
					break;
				case 2:
					strcpy(itc_msg, "Air: Velocity Off");
					break;
				case 3:
					strcpy(itc_msg, "Air: Off");
					break;
				case 4:
					strcpy(itc_msg, "Air: No Update");
					break;
				}
			}

			if (sdl_key=='t')
				VINE_MODE = !VINE_MODE;
			if (sdl_key==SDLK_SPACE)
				sys_pause = !sys_pause;
			if (sdl_key=='u')
				aheat_enable = !aheat_enable;
			if (sdl_key=='h' && !(sdl_mod & KMOD_LCTRL))
			{
				hud_enable = !hud_enable;
			}
			if (sdl_key==SDLK_F1 || (sdl_key=='h' && (sdl_mod & KMOD_LCTRL)))
			{
				if(!it)
				{
					it = 8047;
				}
				else
				{
					it = 0;
				}
			}
			if (sdl_key=='n')
				pretty_powder = !pretty_powder;
			if (sdl_key=='p')
				dump_frame(vid_buf, XRES, YRES, XRES+BARSIZE);
			if (sdl_key=='v'&&(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL)))
			{
				if (clipboard_ready==1)
				{
					load_data = malloc(clipboard_length);
					memcpy(load_data, clipboard_data, clipboard_length);
					load_size = clipboard_length;
					if (load_data)
					{
						load_img = prerender_save(load_data, load_size, &load_w, &load_h);
						if (load_img)
							load_mode = 1;
						else
							free(load_data);
					}
				}
			}
			if (load_mode==1)
			{
				matrix2d transform = m2d_identity;
				vector2d translate = v2d_zero;
				void *ndata;
				int doTransform = 0;
				if (sdl_key=='r'&&(sdl_mod & (KMOD_CTRL))&&(sdl_mod & (KMOD_SHIFT)))
				{
					transform = m2d_new(-1,0,0,1); //horizontal invert
					doTransform = 1;
				}
				else if (sdl_key=='r'&&(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL)))
				{
					transform = m2d_new(0,1,-1,0); //rotate anticlockwise 90 degrees
					doTransform = 1;
				}
				else if (sdl_mod & (KMOD_CTRL))
				{
					doTransform = 1;
					if (sdl_key==SDLK_LEFT) translate = v2d_new(-1,0);
					else if (sdl_key==SDLK_RIGHT) translate = v2d_new(1,0);
					else if (sdl_key==SDLK_UP) translate = v2d_new(0,-1);
					else if (sdl_key==SDLK_DOWN) translate = v2d_new(0,1);
					else doTransform = 0;
				}
				if (0 && doTransform)
				{
					ndata = transform_save(load_data, &load_size, transform, translate);
					if (ndata!=load_data) free(load_data);
					free(load_img);
					load_data = ndata;
					load_img = prerender_save(load_data, load_size, &load_w, &load_h);
				}
			}
			if (sdl_key=='r'&&!(sdl_mod & (KMOD_CTRL|KMOD_SHIFT)))
				GENERATION = 0;
			if (sdl_key=='x'&&(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL)))
			{
				save_mode = 1;
				copy_mode = 2;
			}
			if (sdl_key=='c'&&(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL)))
			{
				save_mode = 1;
				copy_mode = 1;
			}
			//TODO: Superseded by new display mode switching, need some keyboard shortcuts
			/*else if (sdl_key=='c')
			{
				set_cmode((cmode+1) % CM_COUNT);
				if (it > 50)
					it = 50;
			}*/
			if (sdl_key=='z'&&(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL))) // Undo
			{
				int cbx, cby, cbi;

				for (cbi=0; cbi<NPART; cbi++)
					parts[cbi] = cb_parts[cbi];
				parts_lastActiveIndex = NPART-1;

				for (cby = 0; cby<YRES; cby++)
					for (cbx = 0; cbx<XRES; cbx++)
						pmap[cby][cbx] = cb_pmap[cby][cbx];

				for (cby = 0; cby<(YRES/CELL); cby++)
					for (cbx = 0; cbx<(XRES/CELL); cbx++)
					{
						vx[cby][cbx] = cb_vx[cby][cbx];
						vy[cby][cbx] = cb_vy[cby][cbx];
						pv[cby][cbx] = cb_pv[cby][cbx];
						hv[cby][cbx] = cb_hv[cby][cbx];
						bmap[cby][cbx] = cb_bmap[cby][cbx];
						emap[cby][cbx] = cb_emap[cby][cbx];
					}
			}
		}
#ifdef INTERNAL
		int counterthing;
		if (sdl_key=='v'&&!(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL)))//frame capture
		{
			if (sdl_mod & (KMOD_SHIFT)) {
				if (vs>=1)
					vs = 0;
				else
					vs = 3;//every other frame
			}
			else
			{
				if (vs>=1)
					vs = 0;
				else
					vs = 1;
			}
			counterthing = 0;
		}
		if (vs)
		{
			if (counterthing+1>=vs)
			{
				dump_frame(vid_buf, XRES, YRES, XRES+BARSIZE);
				counterthing = 0;
			}
			counterthing = (counterthing+1)%3;
		}
#endif

		if (sdl_wheel)
		{
			if (sdl_zoom_trig)//zoom window change
			{
				ZSIZE += sdl_wheel;
				if (ZSIZE>60)
					ZSIZE = 60;
				if (ZSIZE<2)
					ZSIZE = 2;
				ZFACTOR = 256/ZSIZE;
				sdl_wheel = 0;
			}
			else //change brush size
			{
				if (!(sdl_mod & (KMOD_SHIFT|KMOD_CTRL)))
				{
					bsx += sdl_wheel;
					bsy += sdl_wheel;
				}
				else if (sdl_mod & (KMOD_SHIFT) && !(sdl_mod & (KMOD_CTRL)))
				{
					bsx += sdl_wheel;
				}
				else if (sdl_mod & (KMOD_CTRL) && !(sdl_mod & (KMOD_SHIFT)))
				{
					bsy += sdl_wheel;
				}
				if (bsx>1180)
					bsx = 1180;
				if (bsx<0)
					bsx = 0;
				if (bsy>1180)
					bsy = 1180;
				if (bsy<0)
					bsy = 0;
				sdl_wheel = 0;
				/*if(su >= PT_NUM) {
					if(sl < PT_NUM)
						su = sl;
					if(sr < PT_NUM)
						su = sr;
				}*/
			}
		}

		bq = bc; // bq is previous mouse state
		bc = b = SDL_GetMouseState(&x, &y); // b is current mouse state

#ifdef LUACONSOLE
		if(bc && bq){
			if(!luacon_mouseevent(x/sdl_scale, y/sdl_scale, bc, LUACON_MPRESS)){
				b = 0;
			}
		}
		else if(bc && !bq){
			if(!luacon_mouseevent(x/sdl_scale, y/sdl_scale, bc, LUACON_MDOWN)){
				b = 0;
			}
		}
		else if(!bc && bq){
			if(!luacon_mouseevent(x/sdl_scale, y/sdl_scale, bq, LUACON_MUP)){
				b = 0;
			}
		}
		luacon_step(x/sdl_scale, y/sdl_scale,sl,sr);
		readluastuff();
#endif

		quickoptions_menu(vid_buf, b, bq, x, y);

		for (i=0; i<SC_TOTAL; i++)//draw all the menu sections
		{
			draw_menu(vid_buf, i, active_menu);
		}

		for (i=0; i<SC_TOTAL; i++)//check mouse position to see if it is on a menu section
		{
			if (!b&&x>=sdl_scale*(XRES-2) && x<sdl_scale*(XRES+BARSIZE-1) &&y>= sdl_scale*((i*16)+YRES+MENUSIZE-16-(SC_TOTAL*16)) && y<sdl_scale*((i*16)+YRES+MENUSIZE-16-(SC_TOTAL*16)+15))
			{
				active_menu = i;
			}
		}
		menu_ui_v3(vid_buf, active_menu, &sl, &sr, &dae, b, bq, x, y); //draw the elements in the current menu
		if (zoom_en && x>=sdl_scale*zoom_wx && y>=sdl_scale*zoom_wy //change mouse position while it is in a zoom window
		        && x<sdl_scale*(zoom_wx+ZFACTOR*ZSIZE)
		        && y<sdl_scale*(zoom_wy+ZFACTOR*ZSIZE))
		{
			x = (((x/sdl_scale-zoom_wx)/ZFACTOR)+zoom_x)*sdl_scale;
			y = (((y/sdl_scale-zoom_wy)/ZFACTOR)+zoom_y)*sdl_scale;
		}
		sprintf(heattext,""); sprintf(coordtext,"");
		if (y>=0 && y<sdl_scale*YRES && x>=0 && x<sdl_scale*XRES)
		{
			int cr,wl = 0; //cr is particle under mouse, for drawing HUD information
			char nametext[50] = "";
			if (photons[y/sdl_scale][x/sdl_scale]) {
				cr = photons[y/sdl_scale][x/sdl_scale];
			} else {
				cr = pmap[y/sdl_scale][x/sdl_scale];
			}
			if (!cr && alt_hud == 1)
			{
				wl = bmap[y/sdl_scale/CELL][x/sdl_scale/CELL];
			}
			sprintf(heattext2,""); sprintf(tempstring,"");
			if (cr)
			{
				if (hud_current[10])
				{
					if ((cr&0xFF)==PT_LIFE && parts[cr>>8].ctype>=0 && parts[cr>>8].ctype<NGOLALT)
					{
						sprintf(nametext, "%s (%s),", ptypes[cr&0xFF].name, gmenu[parts[cr>>8].ctype].name);
					}
					else if (hud_current[13] && (cr&0xFF)==PT_LAVA && parts[cr>>8].ctype > 0 && parts[cr>>8].ctype < PT_NUM )
					{
						sprintf(nametext, "Molten %s,", ptypes[parts[cr>>8].ctype].name);
					}
					else if (hud_current[14] && (cr&0xFF)==PT_PIPE && (parts[cr>>8].tmp&0xFF) > 0 && (parts[cr>>8].tmp&0xFF) < PT_NUM )
					{
						char lowername[6];
						int ix;
						strcpy(lowername, ptypes[parts[cr>>8].tmp&0xFF].name);
						for (ix = 0; lowername[ix]; ix++)
							lowername[ix] = tolower(lowername[ix]);

						sprintf(nametext, "Pipe with %s,", lowername);
					}
					else if (hud_current[11])
					{
						int tctype = parts[cr>>8].ctype;
						if ((cr&0xFF)==PT_PIPE && !hud_current[12])
						{
							tctype = parts[cr>>8].tmp&0xFF;
						}
						if (!hud_current[12] && (tctype>=PT_NUM || tctype<0 || (cr&0xFF)==PT_PHOT))
							tctype = 0;
						if (tctype >= 0 && tctype < PT_NUM)
							sprintf(nametext, "%s (%s),", ptypes[cr&0xFF].name, ptypes[tctype].name);
						else
							sprintf(nametext, "%s (%d),", ptypes[cr&0xFF].name, tctype);
					}
					else
					{
						sprintf(nametext, "%s,", ptypes[cr&0xFF].name);
					}
				}
				else if (hud_current[11])
				{
					if (parts[cr>>8].ctype > 0 && parts[cr>>8].ctype < PT_NUM)
						sprintf(nametext," Ctype: %s", ptypes[parts[cr>>8].ctype].name);
					else if (hud_current[12])
						sprintf(nametext," Ctype: %d", parts[cr>>8].ctype);
				}
				else
					sprintf(nametext,"");
				strncpy(heattext2,nametext,50);
				if (hud_current[15])
				{
					sprintf(tempstring," Temp: %0.*f C,",hud_current[18],parts[cr>>8].temp-273.15f);
					strappend(heattext2,tempstring);
				}
				if (hud_current[16])
				{
					sprintf(tempstring," Temp: %0.*f F,",hud_current[18],((parts[cr>>8].temp-273.15f)*9/5)+32);
					strappend(heattext2,tempstring);
				}
				if (hud_current[17])
				{
					sprintf(tempstring," Temp: %0.*f K,",hud_current[18],parts[cr>>8].temp);
					strappend(heattext2,tempstring);
				}
				if (hud_current[19])
				{
					sprintf(tempstring," Life: %d,",parts[cr>>8].life);
					strappend(heattext2,tempstring);
				}
				if (hud_current[20])
				{
					sprintf(tempstring," Tmp: %d,",parts[cr>>8].tmp);
					strappend(heattext2,tempstring);
				}
				if (hud_current[21])
				{
					sprintf(tempstring," Tmp2: %d,",parts[cr>>8].tmp2);
					strappend(heattext2,tempstring);
				}
				if (hud_current[22])
				{
					sprintf(tempstring," X: %0.*f, Y: %0.*f,",hud_current[23],parts[cr>>8].x,hud_current[23],parts[cr>>8].y);
					strappend(heattext2,tempstring);
				}
				if (hud_current[24])
				{
					sprintf(tempstring," Vx: %0.*f, Vy: %0.*f,",hud_current[25],parts[cr>>8].vx,hud_current[25],parts[cr>>8].vy);
					strappend(heattext2,tempstring);
				}
				if ((cr&0xFF)==PT_PHOT) wavelength_gfx = parts[cr>>8].ctype;
			}
			else if (wl)
			{
				if (hud_current[10])
					sprintf(heattext2, "%s,", wtypes[wl-UI_ACTUALSTART].name);
			}
			else
			{
				if (hud_current[10])
					sprintf(heattext2,"Empty,");
			}
			if (hud_current[26])
			{
				sprintf(tempstring," Pressure: %0.*f,",hud_current[27],pv[(y/sdl_scale)/CELL][(x/sdl_scale)/CELL]);
				strappend(heattext2,tempstring);
			}
			heattext2[strlen(heattext2)-1] = '\0'; // delete comma at end
			strcpy(heattext,heattext2);

			if (hud_current[28] && cr)
			{
				if (hud_current[29] || (ngrav_enable && hud_current[30]))
					sprintf(tempstring," #%d,",cr>>8);
				else
					sprintf(tempstring," #%d",cr>>8);
				strappend(coordtext,tempstring);
			}
			if (hud_current[29])
			{
				sprintf(tempstring," X:%d Y:%d",x/sdl_scale,y/sdl_scale);
				strappend(coordtext,tempstring);
			}
			if (hud_current[30] && ngrav_enable)
			{
				sprintf(tempstring," GX: %0.*f GY: %0.*f",hud_current[31],gravx[(((y/sdl_scale)/CELL)*(XRES/CELL))+((x/sdl_scale)/CELL)],hud_current[31],gravy[(((y/sdl_scale)/CELL)*(XRES/CELL))+((x/sdl_scale)/CELL)]);
				strappend(coordtext,tempstring);
			}
			if (hud_current[34] && aheat_enable)
			{
				sprintf(tempstring," A.Heat: %0.*f K",hud_current[35],hv[(y/sdl_scale)/CELL][(x/sdl_scale)/CELL]);
				strappend(coordtext,tempstring);
			}
			if (hud_current[32])
			{
				sprintf(tempstring," Pressure: %0.*f",hud_current[33],pv[(y/sdl_scale)/CELL][(x/sdl_scale)/CELL]);
				strappend(coordtext,tempstring);
			}
		}
		else
		{
			if (hud_current[10])
				sprintf(heattext, "Empty");
			if (hud_current[29])
				sprintf(coordtext, "X:%d Y:%d", x/sdl_scale, y/sdl_scale);
		}


		mx = x;
		my = y;
		if (b && !bq && x>=(XRES-19-new_message_len)*sdl_scale &&
		        x<=(XRES-14)*sdl_scale && y>=(YRES-37)*sdl_scale && y<=(YRES-24)*sdl_scale && svf_messages)
		{
			open_link("http://" SERVER "/Conversations.html");
		}
		if (update_flag)
		{
			info_box(vid_buf, "Finalizing update...");
			if (last_build>BUILD_NUM)
			{
				update_cleanup();
				error_ui(vid_buf, 0, "Update failed - try downloading a new version.");
			}
			else
			{
				if (update_finish())
					error_ui(vid_buf, 0, "Update failed - try downloading a new version.");
				else
					info_ui(vid_buf, "Update success", "You have successfully updated the Powder Toy!");
			}
			update_flag = 0;
		}

		old_version = 0;
		if (b && !bq && x>=(XRES-19-old_ver_len)*sdl_scale &&
		        x<=(XRES-14)*sdl_scale && y>=(YRES-22)*sdl_scale && y<=(YRES-9)*sdl_scale && old_version)
		{
			tmp = malloc(128);
#ifdef BETA
			if (is_beta)
			{
				sprintf(tmp, "Your version: %d.%d Beta (%d)\nNew version: %d.%d Beta (%d)", SAVE_VERSION, MINOR_VERSION, BUILD_NUM, major, minor, buildnum);
			}
			else
			{
				sprintf(tmp, "Your version: %d.%d Beta (%d)\nNew version: %d.%d (%d)", SAVE_VERSION, MINOR_VERSION, BUILD_NUM, major, minor, buildnum);
			}
#else
			if (is_beta)
			{
				sprintf(tmp, "Your version: %d.%d (%d)\nNew version: %d.%d Beta (%d)", SAVE_VERSION, MINOR_VERSION, BUILD_NUM, major, minor, buildnum);
			}
			else
			{
				sprintf(tmp, "Your version: %d.%d (%d)\nNew version: %d.%d (%d)", SAVE_VERSION, MINOR_VERSION, BUILD_NUM, major, minor, buildnum);
			}
#endif
			if (b == 1 && confirm_ui(vid_buf, "Do you want to update The Powder Toy?", tmp, "Update"))
			{
				free(tmp);
				tmp = download_ui(vid_buf, my_uri, &i);
				if (tmp)
				{
					save_presets(1);
					if (update_start(tmp, i))
					{
						update_cleanup();
						save_presets(0);
						error_ui(vid_buf, 0, "Update failed - try downloading a new version.");
					}
					else
						return 0;
				}
			}
			else
			{
				free(tmp);
				old_version = 0;
			}
		}
		if (y>=sdl_scale*(YRES+(MENUSIZE-20))) //mouse checks for buttons at the bottom, to draw mouseover texts
		{
			if (x>=189*sdl_scale && x<=202*sdl_scale && svf_login && svf_open && svf_myvote==0)
			{
				db = svf_own ? 275 : 272;
				if (da < 51)
					da ++;
			}
			else if (x>=204 && x<=217 && svf_login && svf_open && svf_myvote==0)
			{
				db = svf_own ? 275 : 272;
				if (da < 51)
					da ++;
			}
			else if (x>=189 && x<=217 && svf_login && svf_open && svf_myvote!=0)
			{
				db = (svf_myvote==1) ? 273 : 274;
				if (da < 51)
					da ++;
			}
			else if (x>=219*sdl_scale && x<=((XRES+BARSIZE-(510-349))*sdl_scale) && svf_login && svf_open)
			{
				db = svf_own ? 257 : 256;
				if (da < 51)
					da ++;
			}
			else if (x>=((XRES+BARSIZE-(510-351))*sdl_scale) && x<((XRES+BARSIZE-(510-366))*sdl_scale))
			{
				db = 270;
				if (da < 51)
					da ++;
			}
			else if (x>=((XRES+BARSIZE-(510-367))*sdl_scale) && x<((XRES+BARSIZE-(510-383))*sdl_scale))
			{
				db = 266;
				if (da < 51)
					da ++;
			}
			else if (x>=37*sdl_scale && x<=187*sdl_scale)
			{
				if(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL))
				{
					db = 277;
					if (da < 51)
						da ++;
				}
				else if(svf_login)
				{
					db = 259;
					if (svf_open && svf_own && x<=55*sdl_scale)
						db = 258;
					if (da < 51)
						da ++;
				}
			}
			else if (x>=((XRES+BARSIZE-(510-385))*sdl_scale) && x<=((XRES+BARSIZE-(510-476))*sdl_scale))
			{
				db = svf_login ? 261 : 260;
				if (svf_admin)
				{
					db = 268;
				}
				else if (svf_mod)
				{
					db = 271;
				}
				if (da < 51)
					da ++;
			}
			else if (x>=sdl_scale && x<=17*sdl_scale)
			{
				if(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL))
					db = 276;
				else
					db = 262;
				if (da < 51)
					da ++;
			}
			else if (x>=((XRES+BARSIZE-(510-494))*sdl_scale) && x<=((XRES+BARSIZE-(510-509))*sdl_scale))
			{
				db = sys_pause ? 264 : 263;
				if (da < 51)
					da ++;
			}
			else if (x>=((XRES+BARSIZE-(510-476))*sdl_scale) && x<=((XRES+BARSIZE-(510-491))*sdl_scale))
			{
				db = 267;
				if (da < 51)
					da ++;
			}
			else if (x>=19*sdl_scale && x<=35*sdl_scale && svf_open)
			{
				db = 265;
				if (da < 51)
					da ++;
			}
			else if (da > 0)
				da --;
		}
		else if (da > 0)//fade away mouseover text
			da --;

		if (dae > 0) //Fade away selected elements
			dae --;
		
		if (!sdl_zoom_trig && zoom_en==1)
			zoom_en = 0;

		if (sdl_key=='z' && zoom_en==2 && sys_shortcuts==1)
			zoom_en = 1;

		if (load_mode)
		{
			load_x = CELL*((mx/sdl_scale-load_w/2+CELL/2)/CELL);
			load_y = CELL*((my/sdl_scale-load_h/2+CELL/2)/CELL);
			if (load_x+load_w>XRES) load_x=XRES-load_w;
			if (load_y+load_h>YRES) load_y=YRES-load_h;
			if (load_x<0) load_x=0;
			if (load_y<0) load_y=0;
			if (bq==1 && !b)
			{
				parse_save(load_data, load_size, 0, load_x, load_y, bmap, vx, vy, pv, fvx, fvy, signs, parts, pmap);
				free(load_data);
				free(load_img);
				load_mode = 0;
			}
			else if (bq==4 && !b)
			{
				free(load_data);
				free(load_img);
				load_mode = 0;
			}
		}
		else if (save_mode==1)//getting the area you are selecting
		{
			save_x = mx/sdl_scale;
			save_y = my/sdl_scale;
			if (save_x >= XRES) save_x = XRES-1;
			if (save_y >= YRES) save_y = YRES-1;
			save_w = 1;
			save_h = 1;
			if (b==1)
			{
				save_mode = 2;
			}
			else if (b==4)
			{
				save_mode = 0;
				copy_mode = 0;
			}
		}
		else if (save_mode==2)
		{
			save_w = mx/sdl_scale + 1 - save_x;
			save_h = my/sdl_scale + 1 - save_y;
			if (save_w+save_x>XRES) save_w = XRES-save_x;
			if (save_h+save_y>YRES) save_h = YRES-save_y;
			if (save_w<1) save_w = 1;
			if (save_h<1) save_h = 1;
			if (!b)
			{
				if (copy_mode==1)//CTRL-C, copy
				{
					clipboard_data=build_save(&clipboard_length, save_x, save_y, save_w, save_h, bmap, vx, vy, pv, fvx, fvy, signs, parts);
					clipboard_ready = 1;
					save_mode = 0;
					copy_mode = 0;
				}
				else if (copy_mode==2)//CTRL-X, cut
				{
					clipboard_data=build_save(&clipboard_length, save_x, save_y, save_w, save_h, bmap, vx, vy, pv, fvx, fvy, signs, parts);
					clipboard_ready = 1;
					save_mode = 0;
					copy_mode = 0;
					clear_area(save_x, save_y, save_w, save_h);
				}
				else//normal save
				{
					stamp_save(save_x, save_y, save_w, save_h);
					save_mode = 0;
				}
			}
		}
		else if (sdl_zoom_trig && zoom_en<2)
		{
			x /= sdl_scale;
			y /= sdl_scale;
			x -= ZSIZE/2;
			y -= ZSIZE/2;
			if (x<0) x=0;
			if (y<0) y=0;
			if (x>XRES-ZSIZE) x=XRES-ZSIZE;
			if (y>YRES-ZSIZE) y=YRES-ZSIZE;
			zoom_x = x;
			zoom_y = y;
			zoom_wx = (x<XRES/2) ? XRES-ZSIZE*ZFACTOR : 0;
			zoom_wy = 0;
			zoom_en = 1;
			if (!b && bq)
			{
				zoom_en = 2;
				sdl_zoom_trig = 0;
			}
		}
		else if (b)//there is a click
		{
			if (it > 50)
				it = 50;
			x /= sdl_scale;
			y /= sdl_scale;
			if (y>=YRES+(MENUSIZE-20))//check if mouse is on menu buttons
			{
				if (!lb)//mouse is NOT held down, so it is a first click
				{
					if (x>=189 && x<=202 && svf_login && svf_open && svf_myvote==0 && svf_own==0)
					{
						if (execute_vote(vid_buf, svf_id, "Up"))
						{
							svf_myvote = 1;
						}
					}
					if (x>=204 && x<=217 && svf_login && svf_open && svf_myvote==0 && svf_own==0)
					{
						if (execute_vote(vid_buf, svf_id, "Down"))
						{
							svf_myvote = -1;
						}
					}
					if (x>=219 && x<=(XRES+BARSIZE-(510-349)) && svf_login && svf_open)
						tag_list_ui(vid_buf);
					if (x>=(XRES+BARSIZE-(510-351)) && x<(XRES+BARSIZE-(510-366)) && !bq)
					{
						//legacy_enable = !legacy_enable;
						simulation_ui(vid_buf);
					}
					if (x>=(XRES+BARSIZE-(510-367)) && x<=(XRES+BARSIZE-(510-383)) && !bq)
					{
						clear_sim();
						for (i=0; i<NPART-1; i++)
							parts[i].life = i+1;
						parts[NPART-1].life = -1;
						pfree = 0;

						legacy_enable = 0;
						svf_filename[0] = 0;
						svf_fileopen = 0;
						svf_myvote = 0;
						svf_open = 0;
						svf_publish = 0;
						svf_own = 0;
						svf_id[0] = 0;
						svf_name[0] = 0;
						svf_tags[0] = 0;
						svf_description[0] = 0;
						gravityMode = 0;
						airMode = 0;
					}
					if (x>=(XRES+BARSIZE-(510-385)) && x<=(XRES+BARSIZE-(510-476)))
					{
						login_ui(vid_buf);
						if (svf_login) {
							save_presets(0);
							http_session_check = NULL;
						}
					}
					if(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL))
					{
						if (x>=37 && x<=187)
						{
							save_filename_ui(vid_buf);
									
						}
						if (x>=1 && x<=17)
						{
							catalogue_ui(vid_buf);
						}
					} else {
						if (x>=37 && x<=187 && svf_login)
						{
							if (!svf_open || !svf_own || x>51)
							{
								if (save_name_ui(vid_buf)) {
									execute_save(vid_buf);
									if (svf_id[0]) {
										copytext_ui(vid_buf, "Save ID", "Saved successfully!", svf_id);
									}
								}
							}
							else
								execute_save(vid_buf);
							while (!sdl_poll())
								if (!SDL_GetMouseState(&x, &y))
									break;
							b = bq = 0;
						}
						if (x>=1 && x<=17)
						{
							search_ui(vid_buf);
							memset(pers_bg, 0, (XRES+BARSIZE)*YRES*PIXELSIZE);
							memset(fire_r, 0, sizeof(fire_r));
							memset(fire_g, 0, sizeof(fire_g));
							memset(fire_b, 0, sizeof(fire_b));
						}
					}
					if (x>=19 && x<=35 && svf_last && (svf_open || svf_fileopen) && !bq) {
						//int tpval = sys_pause;
						parse_save(svf_last, svf_lsize, 1, 0, 0, bmap, vx, vy, pv, fvx, fvy, signs, parts, pmap);
						//sys_pause = tpval;
					}
					if (x>=(XRES+BARSIZE-(510-476)) && x<=(XRES+BARSIZE-(510-491)) && !bq)
					{
						render_ui(vid_buf, XRES+BARSIZE-(510-491), YRES-2, 3);
					}
					if (x>=(XRES+BARSIZE-(510-494)) && x<=(XRES+BARSIZE-(510-509)) && !bq)
						sys_pause = !sys_pause;
					lb = 0;
				}
			}
			else if (y<YRES)// mouse is in playing field
			{
				int signi;

				c = (b&1) ? sl : sr; //c is element to be spawned
				su = c;

				if (c!=WL_SIGN+100 && c!=PT_FIGH)
				{
					if (!bq)
						for (signi=0; signi<MAXSIGNS; signi++)
							if (sregexp(signs[signi].text, "^{c:[0-9]*|.*}$")==0)
							{
								int signx, signy, signw, signh;
								get_sign_pos(signi, &signx, &signy, &signw, &signh);
								if (x>=signx && x<=signx+signw && y>=signy && y<=signy+signh)
								{
									char buff[256];
									int sldr;

									memset(buff, 0, sizeof(buff));

									for (sldr=3; signs[signi].text[sldr] != '|'; sldr++)
										buff[sldr-3] = signs[signi].text[sldr];

									buff[sldr-3] = '\0';
									open_ui(vid_buf, buff, 0);
								}
							}
				}

				if (c==WL_SIGN+100)
				{
					if (!bq)
						add_sign_ui(vid_buf, x, y);
				}
				else if (c==PT_FIGH)
				{
					if (!bq)
						create_part(-1, x, y, PT_FIGH);
				}
				//for the click functions, lx and ly, are the positions of where the FIRST click happened.  x,y are current mouse position.
				else if (lb)//lb means you are holding mouse down
				{
					if (lm == 1)//line tool
					{
						if (sdl_mod & KMOD_ALT)
						{
							float snap_angle = floor(atan2(y-ly, x-lx)/(M_PI*0.25)+0.5)*M_PI*0.25;
							float line_mag = sqrtf(pow(x-lx,2)+pow(y-ly,2));
							line_x = (int)(line_mag*cos(snap_angle)+lx+0.5f);
							line_y = (int)(line_mag*sin(snap_angle)+ly+0.5f);
						}
						else
						{
							line_x = x;
							line_y = y;
						}
						xor_line(lx, ly, line_x, line_y, vid_buf);
						if (c==WL_FAN+100 && lx>=0 && ly>=0 && lx<XRES && ly<YRES && bmap[ly/CELL][lx/CELL]==WL_FAN)
						{
							nfvx = (line_x-lx)*0.005f;
							nfvy = (line_y-ly)*0.005f;
							flood_parts(lx, ly, WL_FANHELPER, -1, WL_FAN, 0);
							for (j=0; j<YRES/CELL; j++)
								for (i=0; i<XRES/CELL; i++)
									if (bmap[j][i] == WL_FANHELPER)
									{
										fvx[j][i] = nfvx;
										fvy[j][i] = nfvy;
										bmap[j][i] = WL_FAN;
									}
						}
						if (c == SPC_WIND)
						{
							for (j=-bsy; j<=bsy; j++)
								for (i=-bsx; i<=bsx; i++)
									if (lx+i>0 && ly+j>0 && lx+i<XRES && ly+j<YRES && InCurrentBrush(i,j,bsx,bsy))
									{
										vx[(ly+j)/CELL][(lx+i)/CELL] += (line_x-lx)*0.002f;
										vy[(ly+j)/CELL][(lx+i)/CELL] += (line_y-ly)*0.002f;
									}
						}
					}
					else if (lm == 2)//box tool
					{
						xor_line(lx, ly, lx, y, vid_buf);
						xor_line(lx, y, x, y, vid_buf);
						xor_line(x, y, x, ly, vid_buf);
						xor_line(x, ly, lx, ly, vid_buf);
					}
					else//while mouse is held down, it draws lines between previous and current positions
					{
						if (c == SPC_WIND)
						{
							for (j=-bsy; j<=bsy; j++)
								for (i=-bsx; i<=bsx; i++)
									if (x+i>0 && y+j>0 && x+i<XRES && y+j<YRES && InCurrentBrush(i,j,bsx,bsy))
									{
										vx[(y+j)/CELL][(x+i)/CELL] += (x-lx)*0.01f;
										vy[(y+j)/CELL][(x+i)/CELL] += (y-ly)*0.01f;
									}
						}
						else if (c != PT_MOVS)
						{
							create_line(lx, ly, x, y, bsx, bsy, c, get_brush_flags());
						}
						lx = x;
						ly = y;
					}
				}
				else //it is the first click
				{
					//start line tool
					if ((sdl_mod & (KMOD_SHIFT)) && !(sdl_mod & (KMOD_CTRL)))
					{
						lx = x;
						ly = y;
						lb = b;
						lm = 1;//line
					}
					//start box tool
					else if ((sdl_mod & (KMOD_CTRL)) && !(sdl_mod & (KMOD_SHIFT|KMOD_ALT)))
					{
						lx = x;
						ly = y;
						lb = b;
						lm = 2;//box
					}
					//flood fill
					else if ((sdl_mod & (KMOD_CTRL)) && (sdl_mod & (KMOD_SHIFT)) && !(sdl_mod & (KMOD_ALT)))
					{
						if (sdl_mod & (KMOD_CAPS))
							c = 0;
						if (c!=WL_STREAM+100&&c!=SPC_AIR&&c!=SPC_HEAT&&c!=SPC_COOL&&c!=SPC_VACUUM&&!REPLACE_MODE&&c!=SPC_WIND&&c!=SPC_PGRV&&c!=SPC_NGRV)
							flood_parts(x, y, c, -1, -1, get_brush_flags());
						if (c==SPC_HEAT || c==SPC_COOL)
							create_parts(x, y, bsx, bsy, c, get_brush_flags(), 1);
						lx = x;
						ly = y;
						lb = 0;
						lm = 0;
					}
					//sample
					else if (((sdl_mod & (KMOD_ALT)) && !(sdl_mod & (KMOD_SHIFT|KMOD_CTRL))) || b==SDL_BUTTON_MIDDLE)
					{
						if (y>=0 && y<YRES && x>=0 && x<XRES)
						{
							int cr;
							cr = pmap[y][x];
							if (!cr)
								cr = photons[y][x];
							if (cr)
							{
								c = sl = cr&0xFF;
								if (c==PT_LIFE)
									c = sl = (parts[cr>>8].ctype << 8) | c;
							}
							else
							{
								//Something
							}
						}
						lx = x;
						ly = y;
						lb = 0;
						lm = 0;
					}
					else //normal click, spawn element
					{
						//Copy state before drawing any particles (for undo)7
						int cbx, cby, cbi;

						for (cbi=0; cbi<NPART; cbi++)
							cb_parts[cbi] = parts[cbi];

						for (cby = 0; cby<YRES; cby++)
							for (cbx = 0; cbx<XRES; cbx++)
								cb_pmap[cby][cbx] = pmap[cby][cbx];

						for (cby = 0; cby<(YRES/CELL); cby++)
							for (cbx = 0; cbx<(XRES/CELL); cbx++)
							{
								cb_vx[cby][cbx] = vx[cby][cbx];
								cb_vy[cby][cbx] = vy[cby][cbx];
								cb_pv[cby][cbx] = pv[cby][cbx];
								cb_hv[cby][cbx] = hv[cby][cbx];
								cb_bmap[cby][cbx] = bmap[cby][cbx];
								cb_emap[cby][cbx] = emap[cby][cbx];
							}
						if (c == PT_MOVS)
							create_moving_solid(x,y,bsx,bsy);
						else
							create_parts(x, y, bsx, bsy, c, get_brush_flags(), 1);
						lx = x;
						ly = y;
						lb = b;
						lm = 0;
					}
				}
			}
		}
		else
		{
			if (lb && lm) //lm is box/line tool
			{
				x /= sdl_scale;
				y /= sdl_scale;
				c = (lb&1) ? sl : sr;
				su = c;
				if (lm == 1)//line
				{
					if (c!=WL_FAN+100 || lx<0 || ly<0 || lx>=XRES || ly>=YRES || bmap[ly/CELL][lx/CELL]!=WL_FAN)
						create_line(lx, ly, line_x, line_y, bsx, bsy, c, get_brush_flags());
				}
				else//box
					create_box(lx, ly, x, y, c, get_brush_flags());
				lm = 0;
			}
			lb = 0;
		}

		if (load_mode)//draw preview of stamp
		{
			draw_image(vid_buf, load_img, load_x, load_y, load_w, load_h, 128);
			xor_rect(vid_buf, load_x, load_y, load_w, load_h);
		}

		if (save_mode)//draw dotted lines for selection
		{
			xor_rect(vid_buf, save_x, save_y, save_w, save_h);
			da = 51;//draws mouseover text for the message
			db = 269;//the save message
		}

		if (zoom_en!=1 && !load_mode && !save_mode)//draw normal cursor
		{
			render_cursor(vid_buf, mx/sdl_scale, my/sdl_scale, su, bsx, bsy);
			mousex = mx/sdl_scale;
			mousey = my/sdl_scale;
		}
#ifdef OGLR
		draw_parts_fbo();
#endif		
		if (zoom_en)
			render_zoom(vid_buf);

		if (da)
			switch (db)//various mouseover messages, da is the alpha
			{
			case 256:
				drawtext(vid_buf, 16, YRES-24, "Add simulation tags.", 255, 255, 255, da*5);
				break;
			case 257:
				drawtext(vid_buf, 16, YRES-24, "Add and remove simulation tags.", 255, 255, 255, da*5);
				break;
			case 258:
				drawtext(vid_buf, 16, YRES-24, "Save the simulation under the current name.", 255, 255, 255, da*5);
				break;
			case 259:
				drawtext(vid_buf, 16, YRES-24, "Save the simulation under a new name.", 255, 255, 255, da*5);
				break;
			case 260:
				drawtext(vid_buf, 16, YRES-24, "Sign into the Simulation Server.", 255, 255, 255, da*5);
				break;
			case 261:
				drawtext(vid_buf, 16, YRES-24, "Sign into the Simulation Server under a new name.", 255, 255, 255, da*5);
				break;
			case 262:
				drawtext(vid_buf, 16, YRES-24, "Find & open a simulation", 255, 255, 255, da*5);
				break;
			case 263:
				drawtext(vid_buf, 16, YRES-24, "Pause the simulation", 255, 255, 255, da*5);
				break;
			case 264:
				drawtext(vid_buf, 16, YRES-24, "Resume the simulation", 255, 255, 255, da*5);
				break;
			case 265:
				drawtext(vid_buf, 16, YRES-24, "Reload the simulation", 255, 255, 255, da*5);
				break;
			case 266:
				drawtext(vid_buf, 16, YRES-24, "Erase all particles and walls", 255, 255, 255, da*5);
				break;
			case 267:
				drawtext(vid_buf, 16, YRES-24, "Change display mode", 255, 255, 255, da*5);
				break;
			case 268:
				drawtext(vid_buf, 16, YRES-24, "Annuit C\245ptis", 255, 255, 255, da*5);
				break;
			case 269:
				drawtext(vid_buf, 16, YRES-24, "Click-and-drag to specify a rectangle to copy (right click = cancel).", 255, 216, 32, da*5);
				break;
			case 270:
				drawtext(vid_buf, 16, YRES-24, "Simulation options", 255, 255, 255, da*5);
				break;
			case 271:
				drawtext(vid_buf, 16, YRES-24, "You're a moderator", 255, 255, 255, da*5);
				break;
			case 272:
				drawtext(vid_buf, 16, YRES-24, "Like/Dislike this save.", 255, 255, 255, da*5);
				break;
			case 273:
				drawtext(vid_buf, 16, YRES-24, "You like this.", 255, 255, 255, da*5);
				break;
			case 274:
				drawtext(vid_buf, 16, YRES-24, "You dislike this.", 255, 255, 255, da*5);
				break;
			case 275:
				drawtext(vid_buf, 16, YRES-24, "You cannot vote on your own save.", 255, 255, 255, da*5);
				break;
			case 276:
				drawtext(vid_buf, 16, YRES-24, "Open a simulation from your hard drive.", 255, 255, 255, da*5);
				break;
			case 277:
				drawtext(vid_buf, 16, YRES-24, "Save the simulation to your hard drive.", 255, 255, 255, da*5);
				break;
			default:
				drawtext(vid_buf, 16, YRES-24, (char *)ptypes[db].descs, 255, 255, 255, da*5);
			}
		if (itc)//message in the middle of the screen, such as view mode changes
		{
			itc--;
			drawtext_outline(vid_buf, (XRES-textwidth(itc_msg))/2, ((YRES/2)-10), itc_msg, 255, 255, 255, itc>51?255:itc*5, 0, 0, 0, itc>51?255:itc*5);
		}
		if (it)//intro message
		{
			it--;
			drawtext(vid_buf, 16, 20, it_msg, 255, 255, 255, it>51?255:it*5);
		}

		if (old_version)
		{
			clearrect(vid_buf, XRES-21-old_ver_len, YRES-24, old_ver_len+9, 17);
			if (is_beta)
			{
				drawtext(vid_buf, XRES-16-old_ver_len, YRES-19, old_ver_msg_beta, 255, 216, 32, 255);
			}
			else
			{
				drawtext(vid_buf, XRES-16-old_ver_len, YRES-19, old_ver_msg, 255, 216, 32, 255);
			}
			drawrect(vid_buf, XRES-19-old_ver_len, YRES-22, old_ver_len+5, 13, 255, 216, 32, 255);
		}
		
		if (svf_messages)
		{
			sprintf(new_message_msg, "You have %d new message%s, Click to view", svf_messages, (svf_messages>1)?"s":"");
			new_message_len = textwidth(new_message_msg);
			
			clearrect(vid_buf, XRES-21-new_message_len, YRES-39, new_message_len+9, 17);
			drawtext(vid_buf, XRES-16-new_message_len, YRES-34, new_message_msg, 255, 186, 32, 255);
			drawrect(vid_buf, XRES-19-new_message_len, YRES-37, new_message_len+5, 13, 255, 186, 32, 255);
		}

		FPS++;
		currentTime = SDL_GetTicks();
		elapsedTime = currentTime-pastFPS;
		if ((FPS>2 || elapsedTime>1000*2/limitFPS) && elapsedTime && FPS*1000/elapsedTime>limitFPS)
		{
			while ((float)(FPS*1000)/elapsedTime>limitFPS)
			{
				SDL_Delay(1);
				currentTime = SDL_GetTicks();
				elapsedTime = currentTime-pastFPS;
			}
		}
		if (elapsedTime>=1000)
		{
			FPSB = FPS;
			FPSB2 = (float)(FPS*1000)/elapsedTime;
			FPS = 0;
			pastFPS = currentTime;
			if (elapsedTime != currentTime)
			{
				frames = frames + 1;
				totalfps = totalfps + FPSB2;
				if (FPSB2 > maxfps)
					maxfps = FPSB2;
			}
		}
		if (lastx == mousex && lasty == mousey)
		{
			if (!afk)
			{
				afk = 1;
				afkstart = currentTime;
			}
		}
		else if (afk)
		{
			afk = 0;
			totalafktime = totalafktime + afktime;
			afktime = 0;
		}
		if (afk && currentTime - afkstart > 30000)
			afktime = currentTime - afkstart - 30000;
		lastx = mousex;
		lasty = mousey;

		if (hud_enable)
		{
#ifdef BETA
			if (hud_current[0] && hud_current[1])
				sprintf(uitext, "Version %d Beta %d (%d) ", SAVE_VERSION, MINOR_VERSION, BUILD_NUM);
			else if (hud_current[0] && !hud_current[1])
				sprintf(uitext, "Version %d Beta %d ", SAVE_VERSION, MINOR_VERSION);
			else if (!hud_current[0] && hud_current[1])
				sprintf(uitext, "Beta Build %d ", BUILD_NUM);
#else
			if (hud_current[0] && hud_current[1])
				sprintf(uitext, "Version %d.%d (%d) ", SAVE_VERSION, MINOR_VERSION, BUILD_NUM);
			else if (hud_current[0] && !hud_current[1])
				sprintf(uitext, "Version %d.%d ", SAVE_VERSION, MINOR_VERSION);
			else if (!hud_current[0] && hud_current[1])
				sprintf(uitext, "Build %d ", BUILD_NUM);
#endif
			else
				sprintf(uitext,"");
			if (hud_current[36] || hud_current[37] || hud_current[38])
			{
				time_t time2 = time(0);
				struct tm* time = localtime(&time2);
				uitext[strlen(uitext)-1] = ',';
				strappend(uitext," ");
				if (hud_current[36])
				{
					sprintf(tempstring,asctime(time));
					tempstring[7] = 0;
					strappend(uitext,tempstring);
					sprintf(tempstring," %i ",time->tm_mday);
					strappend(uitext,tempstring);
				}
				if (hud_current[37])
				{
					int hour = time->tm_hour%12;
					if (hour == 0)
						hour = 12;
					sprintf(tempstring,"%i:%.2i:%.2i ",hour,time->tm_min,time->tm_sec);
					strappend(uitext,tempstring);
				}
				if (hud_current[38])
				{
					sprintf(tempstring,"%i ",time->tm_year+1900);
					strappend(uitext,tempstring);
				}
				uitext[strlen(uitext)-1] = ',';
				strappend(uitext," ");
			}
			if (hud_current[2])
			{
				sprintf(tempstring,"FPS:%0.*f ",hud_current[3],FPSB2);
				strappend(uitext,tempstring);
			}
			if (hud_current[4])
			{
				sprintf(tempstring,"Parts:%d ",NUM_PARTS);
				strappend(uitext,tempstring);
			}
			if (hud_current[5])
			{
				sprintf(tempstring,"Generation:%d ",GENERATION);
				strappend(uitext,tempstring);
			}
			if (hud_current[6])
			{
				sprintf(tempstring,"Gravity:%d ",gravityMode);
				strappend(uitext,tempstring);
			}
			if (hud_current[7])
			{
				sprintf(tempstring,"Air:%d ",airMode);
				strappend(uitext,tempstring);
			}
			if (hud_current[39])
			{
				timestring(currentTime-totalafktime-afktime, timeinfotext, 2);
				sprintf(tempstring,"Time Played: %s ", timeinfotext);
				strappend(uitext,tempstring);
			}
			if (hud_current[40])
			{
				timestring(totaltime+currentTime-totalafktime-afktime, timeinfotext, 1);
				sprintf(tempstring,"Total Time Played: %s ", timeinfotext);
				strappend(uitext,tempstring);
			}
			if (hud_current[41] && frames)
			{
				sprintf(tempstring,"Average FPS: %0.*f ", hud_current[42], totalfps/frames);
				strappend(uitext,tempstring);
			}
			if (REPLACE_MODE && hud_current[8])
				strappend(uitext, "[REPLACE MODE] ");
			if (sdl_mod&(KMOD_CAPS) && hud_current[8])
				strappend(uitext, "[CAP LOCKS] ");
			if (finding && hud_current[8])
				strappend(uitext, "[FIND] ");
			if (GRID_MODE && hud_current[9])
			{
				sprintf(tempstring, "[GRID: %d] ", GRID_MODE);
				strappend(uitext, tempstring);
			}
#ifdef INTERNAL
			if (vs)
				strappend(uitext, "[FRAME CAPTURE]");
#endif
			quickoptions_tooltip_fade_invert = 255 - (quickoptions_tooltip_fade*20);
			it_invert = 50 - it;
			if(it_invert < 0)
				it_invert = 0;
			if(it_invert > 50)
				it_invert = 50;
			if (sdl_zoom_trig||zoom_en)
			{
				if (zoom_x<XRES/2)
				{
					fillrect(vid_buf, XRES-20-textwidth(heattext), 266, textwidth(heattext)+8, 15, 0, 0, 0, (int)(quickoptions_tooltip_fade_invert*0.5));
					drawtext(vid_buf, XRES-16-textwidth(heattext), 270, heattext, 255, 255, 255, (int)(quickoptions_tooltip_fade_invert*0.75));
					if (DEBUG_MODE)
					{
						fillrect(vid_buf, XRES-20-textwidth(coordtext), 280, textwidth(coordtext)+8, 13, 0, 0, 0, (int)(quickoptions_tooltip_fade_invert*0.5));
						drawtext(vid_buf, XRES-16-textwidth(coordtext), 282, coordtext, 255, 255, 255, (int)(quickoptions_tooltip_fade_invert*0.75));
					}
					if (wavelength_gfx)
						draw_wavelengths(vid_buf,XRES-20-textwidth(heattext),265,2,wavelength_gfx);
				}
				else
				{
					fillrect(vid_buf, 12, 266, textwidth(heattext)+8, 15, 0, 0, 0, (int)(quickoptions_tooltip_fade_invert*0.5));
					drawtext(vid_buf, 16, 270, heattext, 255, 255, 255, (int)(quickoptions_tooltip_fade_invert*0.75));
					if (DEBUG_MODE)
					{
						fillrect(vid_buf, 12, 280, textwidth(coordtext)+8, 13, 0, 0, 0, (int)(quickoptions_tooltip_fade_invert*0.5));
						drawtext(vid_buf, 16, 282, coordtext, 255, 255, 255, (int)(quickoptions_tooltip_fade_invert*0.75));
					}
					if (wavelength_gfx)
						draw_wavelengths(vid_buf,12,265,2,wavelength_gfx);
				}
			}
			else
			{
				fillrect(vid_buf, XRES-20-textwidth(heattext), 12, textwidth(heattext)+8, 15, 0, 0, 0, (int)(quickoptions_tooltip_fade_invert*0.5));
				drawtext(vid_buf, XRES-16-textwidth(heattext), 16, heattext, 255, 255, 255, (int)(quickoptions_tooltip_fade_invert*0.75));
				if (DEBUG_MODE)
				{
					fillrect(vid_buf, XRES-20-textwidth(coordtext), 26, textwidth(coordtext)+8, 11, 0, 0, 0, (int)(quickoptions_tooltip_fade_invert*0.5));
					drawtext(vid_buf, XRES-16-textwidth(coordtext), 27, coordtext, 255, 255, 255, (int)(quickoptions_tooltip_fade_invert*0.75));
				}
				if (wavelength_gfx)
					draw_wavelengths(vid_buf,XRES-20-textwidth(heattext),11,2,wavelength_gfx);
			}
			wavelength_gfx = 0;
			fillrect(vid_buf, 12, 12, textwidth(uitext)+8, 15, 0, 0, 0, (int)(it_invert*2.5));
			drawtext(vid_buf, 16, 16, uitext, 32, 216, 255, it_invert * 4);

			if (drawinfo)
			{
				int ytop = 230, num_parts = 0, totalselected = 0;
				float totaltemp = 0, totalpressure = 0;
				for (i=0; i<NPART; i++)
				{
					if (parts[i].type)
					{
						totaltemp += parts[i].temp;
						num_parts++;
					}
					if (parts[i].type == sl)
						totalselected++;
				}
				for (y=0; y<YRES/CELL; y++)
					for (x=0; x<XRES/CELL; x++)
					{
						totalpressure += pv[y][x];
					}

				timestring(currentTime-totalafktime-afktime, timeinfotext, 0);
				sprintf(infotext,"Time Played: %s", timeinfotext);
				fillrect(vid_buf, 12, ytop-4, textwidth(infotext)+8, 15, 0, 0, 0, 140);
				drawtext(vid_buf, 16, ytop, infotext, 255, 255, 255, 200);
				timestring(totaltime+currentTime-totalafktime-afktime, timeinfotext, 0);
				sprintf(infotext,"Total Time Played: %s", timeinfotext);
				fillrect(vid_buf, 12, ytop+10, textwidth(infotext)+8, 15, 0, 0, 0, 140);
				drawtext(vid_buf, 16, ytop+14, infotext, 255, 255, 255, 200);
				timestring(totalafktime+afktime+prevafktime, timeinfotext, 0);
				sprintf(infotext,"Total AFK Time: %s", timeinfotext);
				fillrect(vid_buf, 12, ytop+24, textwidth(infotext)+8, 15, 0, 0, 0, 140);
				drawtext(vid_buf, 16, ytop+28, infotext, 255, 255, 255, 200);
				if (frames)
				{
					sprintf(infotext,"Average FPS: %f", totalfps/frames);
					fillrect(vid_buf, 12, ytop+38, textwidth(infotext)+8, 15, 0, 0, 0, 140);
					drawtext(vid_buf, 16, ytop+42, infotext, 255, 255, 255, 200);
				}
				sprintf(infotext,"Max FPS: %f", maxfps);
				fillrect(vid_buf, 12, ytop+52, textwidth(infotext)+8, 15, 0, 0, 0, 140);
				drawtext(vid_buf, 16, ytop+56, infotext, 255, 255, 255, 200);
				sprintf(infotext,"Number of Times Played: %i", timesplayed);
				fillrect(vid_buf, 12, ytop+66, textwidth(infotext)+8, 15, 0, 0, 0, 140);
				drawtext(vid_buf, 16, ytop+70, infotext, 255, 255, 255, 200);
				if (timesplayed)
				{
					timestring((totaltime+currentTime-totalafktime-afktime)/timesplayed, timeinfotext, 0);
					sprintf(infotext,"Average Time Played: %s", timeinfotext);
					fillrect(vid_buf, 12, ytop+80, textwidth(infotext)+8, 15, 0, 0, 0, 140);
					drawtext(vid_buf, 16, ytop+84, infotext, 255, 255, 255, 200);
				}
				if (num_parts)
				{
					sprintf(infotext,"Average Temp: %f C", totaltemp/num_parts-273.15f);
					fillrect(vid_buf, 12, ytop+94, textwidth(infotext)+8, 15, 0, 0, 0, 140);
					drawtext(vid_buf, 16, ytop+98, infotext, 255, 255, 255, 200);
				}
				sprintf(infotext,"Average Pressure: %f", totalpressure/(XRES*YRES/CELL/CELL));
				fillrect(vid_buf, 12, ytop+108, textwidth(infotext)+8, 15, 0, 0, 0, 140);
				drawtext(vid_buf, 16, ytop+112, infotext, 255, 255, 255, 200);
				if (num_parts && sl >= 0 && sl < PT_NUM)
				{
					if (sl != 0)
						sprintf(infotext,"%%%s: %f", ptypes[sl].name,(float)totalselected/num_parts*100);
					else
						sprintf(infotext,"%%Empty: %f", (float)totalselected/XRES/YRES*100);
					fillrect(vid_buf, 12, ytop+122, textwidth(infotext)+8, 15, 0, 0, 0, 140);
					drawtext(vid_buf, 16, ytop+126, infotext, 255, 255, 255, 200);
				}
			}

		}

		if (console_mode)
		{
#ifdef LUACONSOLE
			char *console;
			sys_pause = 1;
			console = console_ui(vid_buf, console_error, console_more);
			console = mystrdup(console);
			strcpy(console_error,"");
			if (process_command_lua(vid_buf, console, console_error)==-1)
			{
				free(console);
				break;
			}
			free(console);
			if (!console_mode)
				hud_enable = 1;
#else
			char *console;
			sys_pause = 1;
			console = console_ui(vid_buf, console_error, console_more);
			console = mystrdup(console);
			strcpy(console_error,"");
			if (process_command_old(vid_buf, console, console_error)==-1)
			{
				free(console);
				break;
			}
			free(console);
			if (!console_mode)
				hud_enable = 1;
#endif
		}

		//execute python step hook
		sdl_blit(0, 0, XRES+BARSIZE, YRES+MENUSIZE, vid_buf, XRES+BARSIZE);

		//Setting an element for the stick man
		if (player.spwn==0)
		{
			if ((sr<PT_NUM && ptypes[sr].falldown>0) || sr==SPC_AIR || sr == PT_NEUT || sr == PT_PHOT || sr == PT_LIGH)
				player.elem = sr;
			else
				player.elem = PT_DUST;
		}
		if (player2.spwn==0)
		{
			if ((sr<PT_NUM && ptypes[sr].falldown>0) || sr==SPC_AIR || sr == PT_NEUT || sr == PT_PHOT || sr == PT_LIGH)
				player2.elem = sr;
			else
				player2.elem = PT_DUST;
		}
	}
	save_presets(0);
	
	SDL_CloseAudio();
	http_done();
	gravity_cleanup();
#ifdef LUACONSOLE
	luacon_close();
#endif
#ifdef PTW32_STATIC_LIB
    pthread_win32_thread_detach_np();
    pthread_win32_process_detach_np();
#endif
	save_presets(0);
	return 0;
}
#endif
