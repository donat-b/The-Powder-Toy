/**
 * Powder Toy - Main source
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
#ifndef DEFINE_H
#define DEFINE_H

#ifdef WIN32
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif
 
//VersionInfoStart
#define SAVE_VERSION 88
#define MINOR_VERSION 0
#define BUILD_NUM 271
//VersionInfoEnd

#define MOD_VERSION 24
#define MOD_MINOR_VERSION 0
#define MOD_SAVE_VERSION 15 //This is not the version number of my mod anymore, it's only changed when I change the saving code
#define MOD_BUILD_VERSION 20 //For update checks
#define BETA_VERSION 88
#define BETA_MINOR_VER 0
#define RELEASE_VERSION 87
#define RELEASE_MINOR_VER 2

#define IDENT_VERSION "G" //Change this if you're not Simon! It should be a single letter

#define MTOS_EXPAND(str) #str
#define MTOS(str) MTOS_EXPAND(str)

#define SERVER "powdertoy.co.uk"
#define SCRIPTSERVER "powdertoy.co.uk"
#define STATICSERVER "static.powdertoy.co.uk"

#ifndef UPDATESERVER
	#define UPDATESERVER "178.219.36.155" // change this to check for updates on a different server
	#define UPDATESERVERALT "mniip.com" // alternate update server (same as above)
#endif

#define LOCAL_SAVE_DIR "Saves"

#define THUMB_CACHE_SIZE 256

#ifndef M_PI
#define M_PI 3.14159265f
#endif
#ifndef M_GRAV
#define M_GRAV 6.67300e-1
#endif

#define IMGCONNS 6
#define TIMEOUT 100
#define HTTP_TIMEOUT 10

#ifdef RENDERER
#define MENUSIZE 0
#define BARSIZE 0
#else
#define MENUSIZE 40
#define BARSIZE 17
#endif
#define XRES	612
#define YRES	384
#define NPART XRES*YRES

#define XCNTR   306
#define YCNTR   192

#define MAX_DISTANCE sqrt(pow(XRES, 2.0f)+pow(YRES, 2.0f))

#define GRAV_DIFF

#define MAXSIGNS 16
#define TAG_MAX 256

#define ZSIZE_D	16
#define ZFACTOR_D	8
extern unsigned char ZFACTOR;
extern unsigned char ZSIZE;

#define CELL    4
#define ISTP    (CELL/2)
#define CFDS	(4.0f/CELL)

#define AIR_TSTEPP 0.3f
#define AIR_TSTEPV 0.4f
#define AIR_VADV 0.3f
#define AIR_VLOSS 0.999f
#define AIR_PLOSS 0.9999f

#define GRID_X 5
#define GRID_Y 4
#define GRID_P 3
#define GRID_S 6
#define GRID_Z 3

#define CATALOGUE_X 4
#define CATALOGUE_Y 3
#define CATALOGUE_S 6
#define CATALOGUE_Z 3

#define STAMP_MAX 500

//#define SAVE_OPS
//#define REALISTIC

#define NGOL 24

#define CIRCLE_BRUSH 0
#define SQUARE_BRUSH 1
#define TRI_BRUSH 2
#define BRUSH_NUM 3


#define GRAVFFT
#define LUACONSOLE

#ifdef PIX16
typedef unsigned short pixel;
#else
typedef unsigned int pixel;
#endif

#define SURF_RANGE     10
#define NORMAL_MIN_EST 3
#define NORMAL_INTERP  20
#define NORMAL_FRAC    16

#define REFRACT        0x80000000

/* heavy flint glass, for awesome refraction/dispersion
   this way you can make roof prisms easily */
#define GLASS_IOR      1.9
#define GLASS_DISP     0.07

#ifdef WIN32
#define strcasecmp stricmp
#endif
#if defined(WIN32) && !defined(__GNUC__)
#define fmin min
#define fminf min
#define fmax max
#define fmaxf max
#endif
//#if _MSC_VER
//#define snprintf _snprintf
//#endif

#ifdef __cplusplus
#define TPT_INLINE
#define TPT_GNU_INLINE
#else
#if defined(WIN32) && !defined(__GNUC__)
#define TPT_INLINE _inline
#define TPT_GNU_INLINE
#else
#define TPT_INLINE inline
#define TPT_GNU_INLINE inline
#endif
#endif

#define SDEUT

#define DEBUG_PARTS		0x0001
#define DEBUG_PARTCOUNT	0x0002
#define DEBUG_DRAWTOOL	0x0004
#define DEBUG_PERFORMANCE_CALC 0x0008
#define DEBUG_PERFORMANCE_FRAME 0x0010

typedef unsigned char uint8;

extern char * saveDataOpen;
extern int saveDataOpenSize;

#ifdef INTERNAL
	extern int vs;
#endif
extern int amd;

extern float FPSB2;
extern int main_loop;
extern int elapsedTime;

extern int NUM_PARTS;

extern int legacy_enable;
extern int sound_enable;
extern int kiosk_enable;
extern int aheat_enable;
extern int decorations_enable;
extern int active_menu;
extern int last_active_menu;
extern int last_fav_menu;
extern int hud_enable;
extern int pretty_powder;
extern int drawgrav_enable;
extern int ngrav_enable;
extern char edgeMode;
extern int limitFPS;
extern int water_equal_test;
extern int quickoptionsToolTipFade;
extern int favMenu[19];
extern int finding;
extern int locked;
extern int highesttemp;
extern int lowesttemp;
extern int heatmode;
extern int maxframes;
extern int secret_els;
extern int save_as;
extern void *prop_value;
extern int prop_format;
extern unsigned int prop_offset;
extern int tab_num;
extern int num_tabs;
extern int show_tabs;
extern int sl;
extern int last_sl;
extern int sr;
extern int su;
extern float toolStrength;
extern int autosave;
extern int realistic;
extern int loop_time;
extern int unlockedstuff;
extern int old_menu;
extern int h;
extern int over_el;
extern int loop_time;
extern int mod_save;
extern int decobox_hidden;

extern int drawinfo;
extern int currentTime;
extern int totaltime;
extern int totalafktime;
extern int afktime;
extern int frames;
extern double totalfps;
extern double maxfps;
extern int prevafktime;
extern int timesplayed;

extern unsigned int decocolor;
extern int deco_disablestuff;

extern int doUpdates;

extern int debug_flags;
#define DEBUG_PERF_FRAMECOUNT 256
extern int debug_perf_istart;
extern int debug_perf_iend;
extern long debug_perf_frametime[DEBUG_PERF_FRAMECOUNT];
extern long debug_perf_partitime[DEBUG_PERF_FRAMECOUNT];
extern long debug_perf_time;

extern int active_menu;

extern int sys_pause;
extern int framerender;

extern int mousex, mousey;

struct sign
{
	int x,y,ju;
	char text[256];
};
typedef struct sign sign;

struct stamp
{
	char name[11];
	pixel *thumb;
	int thumb_w, thumb_h, dodelete;
};
typedef struct stamp stamp;

extern int frameidx;
extern int MSIGN;
extern int CGOL;
extern int ISGOL;
extern int ISLOVE;
extern int ISLOLZ;
extern int ISGRAV;
extern int ISWIRE;
extern int ISANIM;
extern int GSPEED;
extern int love[XRES/9][YRES/9];
extern int lolz[XRES/9][YRES/9];
extern unsigned char gol[YRES][XRES];
extern short gol2[YRES][XRES][9];
extern int SEC;
extern int SEC2;
extern int console_mode;
extern int REPLACE_MODE;
extern int CURRENT_BRUSH;
extern int GRID_MODE;
extern int DEBUG_MODE;
extern int GENERATION;
extern int ISSPAWN1;
extern int ISSPAWN2;
extern sign signs[MAXSIGNS];
extern stamp stamps[STAMP_MAX];
extern int stamp_count;
extern int itc;
extern char itc_msg[128];

extern int ptsaveOpenID;
extern int saveURIOpen;
extern int do_open;
extern int sys_pause;
extern int sys_shortcuts;
extern int legacy_enable; //Used to disable new features such as heat, will be set by commandline or save.
extern int framerender;
extern pixel *vid_buf;

extern unsigned char last_major, last_minor, update_flag, last_build;

extern char http_proxy_string[256];

//Functions in main.c
void thumb_cache_inval(char *id);
void thumb_cache_add(char *id, void *thumb, int size);
int thumb_cache_find(char *id, void **thumb, int *size);
void dump_frame(pixel *src, int w, int h, int pitch);
void clear_sim();
void NewSim();
char* stamp_save(int x, int y, int w, int h);
void tab_save(int num, char reloadButton);
void *stamp_load(int i, int *size, int reorder);
int tab_load(int tabNum);
void del_stamp(int d);
void sdl_seticon(void);
void play_sound(char *file);
int set_scale(int scale, int kiosk);
void dump_frame(pixel *src, int w, int h, int pitch);
void ctrlzSnapshot();
#endif
