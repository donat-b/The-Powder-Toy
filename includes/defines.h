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

#ifdef WIN
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif
 
//VersionInfoStart
#define SAVE_VERSION 90
#define MINOR_VERSION 2
#define BUILD_NUM 322
//VersionInfoEnd

#define MOD_VERSION 32
#define MOD_MINOR_VERSION 3
#define MOD_SAVE_VERSION 21 //This is not the version number of my mod anymore, it's only changed when I change the saving code
#define MOD_BUILD_VERSION 64 //For update checks
#define BETA_VERSION 91
#define BETA_MINOR_VER 0
#define RELEASE_VERSION 90
#define RELEASE_MINOR_VER 2

#define MOBILE_MAJOR 0
#define MOBILE_MINOR 3
#define MOBILE_BUILD 102

#define IDENT_VERSION "G" //Change this if you're not Simon! It should be a single letter

#define MTOS_EXPAND(str) #str
#define MTOS(str) MTOS_EXPAND(str)

#ifndef SERVER
#define SERVER "powdertoy.co.uk"
#endif
#ifndef STATICSERVER
#define STATICSERVER "static.powdertoy.co.uk"
#endif
#ifndef UPDATESERVER
#define UPDATESERVER "starcatcher.us/TPT"
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
#ifdef TOUCHUI
#define BARSIZE 30
#else
#define BARSIZE 17
#endif
#endif
#define XRES	800
#define YRES	600
#define NPART XRES*YRES
const int menuStartPosition = XRES+BARSIZE-17;
const int menuIconWidth = 17;

#define XCNTR   XRES/2
#define YCNTR   YRES/2

#define MAX_DISTANCE sqrt(pow(XRES, 2.0f)+pow(YRES, 2.0f))

#define GRAV_DIFF

#define TAG_MAX 256

#define CELL    4
#define ISTP    (CELL/2)
#define CFDS	(4.0f/CELL)
#define SIM_MAXVELOCITY 1e4f

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

#define SURF_RANGE     10
#define NORMAL_MIN_EST 3
#define NORMAL_INTERP  20
#define NORMAL_FRAC    16

#define REFRACT        0x80000000

/* heavy flint glass, for awesome refraction/dispersion
   this way you can make roof prisms easily */
#define GLASS_IOR      1.9f
#define GLASS_DISP     0.07f

#ifdef _MSC_VER
#pragma warning(disable: 4100) // unreferenced formal parameter
#endif

#if defined(WIN) && defined(__GNUC__)
#define TH_ENTRY_POINT __attribute__((force_align_arg_pointer))
#else
#define TH_ENTRY_POINT
#endif

enum PropertyType { ParticleType, Colour, Integer, UInteger, Float, String, Char, UChar };
union PropertyValue
{
	int Integer;
	unsigned int UInteger;
	float Float;
};

#define SDEUT

#define DEBUG_PARTS				0x0001
#define DEBUG_ELEMENTPOPULATION	0x0002
#define DEBUG_DRAWTOOL			0x0004
#define DEBUG_PARTICLE_UPDATES	0x0008

typedef unsigned char uint8;

extern char * saveDataOpen;
extern int saveDataOpenSize;

#ifdef INTERNAL
	extern int vs;
#endif

extern bool firstRun;
extern float FPSB2;
extern int main_loop;
extern int elapsedTime;

extern int NUM_PARTS;

extern int legacy_enable;
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
extern int finding;
extern int locked;
extern int highesttemp;
extern int lowesttemp;
extern int heatmode;
extern int secret_els;
extern int tab_num;
extern int num_tabs;
extern int show_tabs;
#ifdef __cplusplus
class Tool;
extern Tool* activeTools[3];
extern Tool* regularTools[3];
extern Tool* decoTools[3];
#endif
extern int activeToolID;
extern float toolStrength;
extern int autosave;
extern int realistic;
extern int loop_time;
extern int unlockedstuff;
extern int old_menu;
extern int decobox_hidden;

extern int drawinfo;
extern int currentTime;
extern int totaltime;
extern int totalafktime;
extern int afktime;
extern int frames;
extern double totalfps;
extern int prevafktime;
extern int timesplayed;

extern int deco_disablestuff;

extern bool doUpdates;

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

#include "graphics/Pixel.h"
struct stamp
{
	char name[11];
	pixel *thumb;
	int thumb_w, thumb_h, dodelete;
};
typedef struct stamp stamp;

extern int console_mode;
extern bool REPLACE_MODE;
extern bool SPECIFIC_DELETE;
extern int GRID_MODE;
extern int DEBUG_MODE;
extern stamp stamps[STAMP_MAX];
extern int stamp_count;

extern int ptsaveOpenID;
extern int saveURIOpen;
extern int do_open;
extern int sys_pause;
extern int sys_shortcuts;
extern int legacy_enable; //Used to disable new features such as heat, will be set by commandline or save.
extern pixel *vid_buf;

extern int scrollSpeed;
extern float scrollDeceleration;

extern unsigned short last_major, last_minor, update_flag, last_build;

extern char http_proxy_string[256];

//Functions in main.c
void thumb_cache_inval(char *id);
void thumb_cache_add(char *id, void *thumb, int size);
bool thumb_cache_find(char *id, void **thumb, int *size);
void dump_frame(pixel *src, int w, int h, int pitch);
void clear_sim();
void NewSim();
char* stamp_save(int x, int y, int w, int h);
void tab_save(int num, char reloadButton);
void *stamp_load(int i, int *size, int reorder);
int tab_load(int tabNum, bool del = false);
void stamp_init();
void del_stamp(int d);
int set_scale(int scale, int kiosk);
void dump_frame(pixel *src, int w, int h, int pitch);
void ctrlzSnapshot();

extern bool sendNewEvents;
extern bool openConsole;
extern bool openSign;
extern bool openProp;
#ifdef __cplusplus
class PowderToy;
extern PowderToy *the_game;
#endif
int main_loop_temp(int b, int bq, int sdl_key, int sdl_rkey, unsigned short sdl_mod, int x, int y, int sdl_wheel);
void main_end_hack();
#endif
