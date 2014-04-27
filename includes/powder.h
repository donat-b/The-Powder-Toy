/**
 * Powder Toy - particle simulation (header)
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
#ifndef POWDER_H
#define POWDER_H

#include "air.h"
#include "graphics.h"
#include "defines.h"
#include "misc.h"

#define CM_COUNT 11
#define CM_CRACK 10
#define CM_LIFE 9
#define CM_GRAD 8
#define CM_NOTHING 7
#define CM_FANCY 6
#define CM_HEAT 5
#define CM_BLOB 4
#define CM_FIRE 3
#define CM_PERS 2
#define CM_PRESS 1
#define CM_VEL 0

#define BRUSH_REPLACEMODE 0x1
#define BRUSH_SPECIFIC_DELETE 0x2

#include "simulation/ElementNumbers.h"

#define R_TEMP 22
#define MAX_TEMP 9999
#define MIN_TEMP 0
#define O_MAX_TEMP 3500
#define O_MIN_TEMP -273

#define ST_NONE 0
#define ST_SOLID 1
#define ST_LIQUID 2
#define ST_GAS 3

#define TYPE_PART			0x0000001 //1 Powders
#define TYPE_LIQUID			0x0000002 //2 Liquids
#define TYPE_SOLID			0x0000004 //4 Solids
#define TYPE_GAS			0x0000008 //8 Gases (Includes plasma)
#define TYPE_ENERGY			0x0000010 //16 Energy (Thunder, Light, Neutrons etc.)
#define PROP_CONDUCTS		0x0000020 //32 Conducts electricity
#define PROP_BLACK			0x0000040 //64 Absorbs Photons (not currently implemented or used, a photwl attribute might be better)
#define PROP_NEUTPENETRATE	0x0000080 //128 Penetrated by neutrons
#define PROP_NEUTABSORB		0x0000100 //256 Absorbs neutrons, reflect is default
#define PROP_NEUTPASS		0x0000200 //512 Neutrons pass through, such as with glass
#define PROP_DEADLY			0x0000400 //1024 Is deadly for stickman
#define PROP_HOT_GLOW		0x0000800 //2048 Hot Metal Glow
#define PROP_LIFE			0x0001000 //4096 Is a GoL type
#define PROP_RADIOACTIVE	0x0002000 //8192 Radioactive
#define PROP_LIFE_DEC		0x0004000 //2^14 Life decreases by one every frame if > zero
#define PROP_LIFE_KILL		0x0008000 //2^15 Kill when life value is <= zero
#define PROP_LIFE_KILL_DEC	0x0010000 //2^16 Kill when life value is decremented to <= zero
#define PROP_INDESTRUCTIBLE	0x0020000 //2^17 Makes elements invincible, even to bomb/dest
#define PROP_CLONE			0x0040000 //2^18 Makes elements clone things that touch it
#define PROP_BREAKABLECLONE	0x0080000 //2^19 Makes breakable elements clone things that touch it
#define PROP_POWERED		0x0100000 //2^20 Makes an element turn on/off with PSCN/NSCN
#define PROP_SPARKSETTLE	0x0200000 //2^21 Allow Sparks/Embers to settle
#define PROP_NOAMBHEAT      0x0400000 //2^23 Don't transfer or receive heat from ambient heat.
#define PROP_MOVS			0x0800000 //2^24 Moving solids!
#define PROP_DRAWONCTYPE	0x1000000 //2^25 Set its ctype to another element if the element is drawn upon it (like what CLNE does)
#define PROP_NOCTYPEDRAW	0x2000000 // 2^26 When this element is drawn upon with, do not set ctype (like BCLN for CLNE) 

#define FLAG_STAGNANT	0x1
#define FLAG_SKIPMOVE	0x2  // Skip movement for one frame
#define FLAG_EXPLODE	0x4  // EXPL explosion
#define FLAG_INSTACTV	0x8  // Instant activation for powered electronics
#define FLAG_WATEREQUAL 0x10 // If a liquid was already checked during equalization
#define FLAG_DISAPPEAR	0x20 // Will disappear on next frame no matter what

class Simulation;
#define UPDATE_FUNC_ARGS Simulation *sim, int i, int x, int y, int surround_space, int nt
#define UPDATE_FUNC_SUBCALL_ARGS sim, i, x, y, surround_space, nt
#define GRAPHICS_FUNC_ARGS Simulation *sim, particle *cpart, int nx, int ny, int *pixel_mode, int* cola, int *colr, int *colg, int *colb, int *firea, int *firer, int *fireg, int *fireb
#define GRAPHICS_FUNC_SUBCALL_ARGS sim, cpart, nx, ny, pixel_mode, cola, colr, colg, colb, firea, firer, fireg, fireb

#include "simulation/Particle.h"

void TRON_init_graphics();

struct playerst
{
	char comm;           //command cell
	char pcomm;          //previous command
	int elem;            //element power
	float legs[16];      //legs' positions
	float accs[8];       //accelerations
	char spwn;           //if stick man was spawned
	unsigned int frames; //frames since last particle spawn - used when spawning LIGH
	int spawnID;         //id of the SPWN particle
	char rocketBoots;
};
typedef struct playerst playerst;

int run_stickman(playerst* playerp, UPDATE_FUNC_ARGS);
void STKM_init_legs(playerst* playerp, int i);
void STKM_interact(Simulation* sim, playerst* playerp, int i, int x, int y);
void PPIP_flood_trigger(int x, int y, int sparkedBy);

struct part_type
{
	char *name;
	pixel pcolors;
	float advection;
	float airdrag;
	float airloss;
	float loss;
	float collision;
	float gravity;
	float diffusion;
	float hotair;
	int falldown;
	int flammable;
	int explosive;
	int meltable;
	int hardness;
	int menu;
	int enabled;
	int weight;
	int menusection;
	float heat;
	unsigned char hconduct;
	char *descs;
	char state;
	unsigned int properties;
	int (*update_func) (UPDATE_FUNC_ARGS);
	int (*graphics_func) (GRAPHICS_FUNC_ARGS);
};
typedef struct part_type part_type;

struct part_transition
{
	float plv; // transition occurs if pv is lower than this
	int plt;
	float phv; // transition occurs if pv is higher than this
	int pht;
	float tlv; // transition occurs if t is lower than this
	int tlt;
	float thv; // transition occurs if t is higher than this
	int tht;
};
typedef struct part_transition part_transition;

// TODO: falldown, properties, state - should at least one of these be removed?
extern part_type ptypes[PT_NUM];
extern unsigned int platent[PT_NUM];

extern part_transition ptransitions[PT_NUM];

static int loverule[9][9] =
{
	{0,0,1,1,0,0,0,0,0},
	{0,1,0,0,1,1,0,0,0},
	{1,0,0,0,0,0,1,0,0},
	{1,0,0,0,0,0,0,1,0},
	{0,1,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,1,0},
	{1,0,0,0,0,0,1,0,0},
	{0,1,0,0,1,1,0,0,0},
	{0,0,1,1,0,0,0,0,0},
};
static int lolzrule[9][9] =
{
	{0,0,0,0,0,0,0,0,0},
	{1,0,0,0,0,0,1,0,0},
	{1,0,0,0,0,0,1,0,0},
	{1,0,0,1,1,0,0,1,0},
	{1,0,1,0,0,1,0,1,0},
	{1,0,1,0,0,1,0,1,0},
	{0,1,0,1,1,0,0,1,0},
	{0,1,0,0,0,0,0,1,0},
	{0,1,0,0,0,0,0,1,0},
};

#define FAV_START 300
#define FAV_MORE 300
#define FAV_BACK 301
#define FAV_FIND 302
#define FAV_INFO 303
#define FAV_ROTATE 304
#define FAV_HEAT 305
//#define FAV_SAVE 306
#define FAV_LUA 306
#define FAV_CUSTOMHUD 307
#define FAV_AUTOSAVE 308
#define FAV_REAL 309
#define FAV_FIND2 310
#define FAV_DATE 311
#define FAV_SECR 312
#define FAV_END 313

struct fav_menu
{
	const char *name;
	pixel colour;
	const char *description;
};
typedef struct fav_menu fav_menu;

static fav_menu fav[] =
{
	{"MORE", PIXPACK(0xFF7F00), "Display different options"},
	{"BACK", PIXPACK(0xFF7F00), "Go back to the favorites menu"},
	//{"HUD",  PIXPACK(0x20D8FF), "Left click to toggle a different HUD. Now using the "},
	{"FIND", PIXPACK(0xFF0000), "Finds the currently selected element on the screen and temporarily colors it red"},
	{"INFO", PIXPACK(0x00FF00), "Displays statistics and records about The Powder Toy. Left click to toggle display"},
	{"SPIN", PIXPACK(0x0010A0), "Makes moving solids rotate. Currently "},
	{"HEAT", PIXPACK(0xFF00D4), "Changes heat display mode. Current mode: "},
	//{"SAVE", PIXPACK(0x2B1AC9), "Makes saves/stamps compatible with: "},
	{"LUA",  PIXPACK(0xFFFF00), "Add Lua code to a save"},
	{"HUD2", PIXPACK(0x20D8FF), "Make a custom HUD"},
	{"AUTO", PIXPACK(0xDF1BFF), "Sets how often your work is autosaved. Currently: "},
	{"REAL", PIXPACK(0xFF6800), "Turns on realistic heat mode, by savask. Now "},
	{"FND2", PIXPACK(0xDF0000), "Alternate find mode, looks different but may find things better. Now "},
	{"DATE", PIXPACK(0x3FBB3F), "Change date and time format. Example: "},
	{"", PIXPACK(0x000000), ""}
};

#define CHANNELS ((int)(MAX_TEMP-73)/100+2)
extern const particle emptyparticle;

extern int force_stacking_check;
extern int ppip_changed;

extern playerst player;
extern playerst player2;

extern playerst fighters[256];
extern unsigned char fighcount;

extern int airMode;

extern particle *parts;
extern particle *cb_parts;

extern unsigned char bmap[YRES/CELL][XRES/CELL];
extern unsigned char emap[YRES/CELL][XRES/CELL];

extern unsigned char cb_bmap[YRES/CELL][XRES/CELL];
extern unsigned char cb_emap[YRES/CELL][XRES/CELL];

extern unsigned pmap[YRES][XRES];
extern unsigned cb_pmap[YRES][XRES];
extern int pmap_count[YRES][XRES];

extern unsigned photons[YRES][XRES];

extern int GRAV;
extern int GRAV_R;
extern int GRAV_G;
extern int GRAV_B;
extern int GRAV_R2;
extern int GRAV_G2;
extern int GRAV_B2;

extern int msindex[256];
extern int msnum[256];
extern float msvx[256];
extern float msvy[256];
extern float msrotation[256];
extern float newmsrotation[256];
extern int numballs;
extern int ms_rotation;

int get_normal_interp(int pt, float x0, float y0, float dx, float dy, float *nx, float *ny);
int get_wavelength_bin(int *wm);

int OutOfBounds(int x, int y);
int move(int i, int x, int y, float nxf, float nyf);
int do_move(int i, int x, int y, float nxf, float nyf);
int try_move(int i, int x, int y, int nx, int ny);
int eval_move(int pt, int nx, int ny, unsigned *rr);

extern unsigned char can_move[PT_NUM][PT_NUM];
void init_can_move();
int IsWallBlocking(int x, int y, int type);

static void create_cherenkov_photon(int pp);
static void create_gain_photon(int pp);

void kill_part(int i);

void detach(int i);
int interactWavelengths(particle* cpart, int origWl);
int getWavelengths(particle* cpart);

void part_change_type(int i, int x, int y, int t);

void get_gravity_field(int x, int y, float particleGrav, float newtonGrav, float *pGravX, float *pGravY);

int get_brush_flags();

int create_part(int p, int x, int y, int t);

void delete_part(int x, int y, int flags);

int is_wire(int x, int y);

int is_wire_off(int x, int y);

void set_emap(int x, int y);

int parts_avg(int ci, int ni, int t);

int nearest_part(int ci, int t, int max_d);

void stacking_check();

void GRAV_update();

void LOVELOLZ_update();

void WIRE_update();

void PPIP_update();

void LIFE_update();

void WIFI_update();

void decrease_life(int i);

int transfer_heat(int i, int surround[8]);

void particle_transitions(int i, int* t);

void update_particles_i();

void update_particles(pixel *vid);

void update_moving_solids();

void rotate_area(int area_x, int area_y, int area_w, int area_h, int invert);

void clear_area(int area_x, int area_y, int area_w, int area_h);

int INST_flood_spark(Simulation *sim, int x, int y);

int flood_water(int x, int y, int i, int originaly, int check);

void create_moving_solid(int x, int y, int type, Brush* brush);

void orbitalparts_get(int block1, int block2, int resblock1[], int resblock2[]);
void orbitalparts_set(int *block1, int *block2, int resblock1[], int resblock2[]);

void draw_bframe();
void erase_bframe();

#endif
