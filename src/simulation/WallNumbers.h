#ifndef WALLNUMBERS_H
#define WALLNUMBERS_H
#include <string>
#include <graphics.h> //pixel, PIXPACK

#define O_WL_WALLELEC	122
#define O_WL_EWALL		123
#define O_WL_DETECT		124
#define O_WL_STREAM		125
#define O_WL_SIGN		126
#define O_WL_FAN		127
#define O_WL_FANHELPER	255
#define O_WL_ALLOWLIQUID	128
#define O_WL_DESTROYALL	129
#define O_WL_ERASE		130
#define O_WL_WALL		131
#define O_WL_ALLOWAIR	132
#define O_WL_ALLOWSOLID	133
#define O_WL_ALLOWALLELEC	134
#define O_WL_EHOLE		135
#define O_WL_ALLOWGAS	140
#define O_WL_GRAV		142
#define O_WL_ALLOWENERGY 145
#define O_WL_ERASEALL	147

#define WL_ERASE		0
#define WL_WALLELEC		1
#define WL_EWALL		2
#define WL_DETECT		3
#define WL_STREAM		4
#define WL_FAN			5
#define WL_ALLOWLIQUID	6
#define WL_DESTROYALL	7
#define WL_WALL			8
#define WL_ALLOWAIR		9
#define WL_ALLOWSOLID	10
#define WL_ALLOWALLELEC	11
#define WL_EHOLE		12
#define WL_ALLOWGAS		13
#define WL_GRAV			14
#define WL_ALLOWENERGY	15
#define WL_ERASEALL		16
#define WL_FANHELPER	255

#define WALLCOUNT 17

struct wallType
{
	std::string name;
	pixel colour;
	pixel eglow; // if emap set, add this to fire glow
	int drawstyle;
	std::string descs;
};
typedef struct wallType wallType;

static wallType wallTypes[] =
{
	{"ERASE",			PIXPACK(0x808080), PIXPACK(0x000000), -1, "Erases walls."},
	{"CONDUCTIVE WALL",	PIXPACK(0xC0C0C0), PIXPACK(0x101010), 0,  "Blocks everything. Conductive."},
	{"EWALL",			PIXPACK(0x808080), PIXPACK(0x808080), 0,  "E-Wall. Becomes transparent when electricity is connected."},
	{"DETECTOR",		PIXPACK(0xFF8080), PIXPACK(0xFF2008), 1,  "Detector. Generates electricity when a particle is inside."},
	{"STREAMLINE",		PIXPACK(0x808080), PIXPACK(0x000000), 0,  "Streamline. Creates a line that follows air movement."},
	{"FAN",				PIXPACK(0x8080FF), PIXPACK(0x000000), 1,  "Fan. Accelerates air. Use the line tool to set direction and strength."},
	{"LIQUID WALL",		PIXPACK(0xC0C0C0), PIXPACK(0x101010), 2,  "Allows liquids, blocks all other particles. Conductive."},
	{"ABSORB WALL",		PIXPACK(0x808080), PIXPACK(0x000000), 1,  "Absorbs particles but lets air currents through."},
	{"WALL",			PIXPACK(0x808080), PIXPACK(0x000000), 3,  "Basic wall, blocks everything."},
	{"AIRONLY WALL",	PIXPACK(0x3C3C3C), PIXPACK(0x000000), 1,  "Allows air, but blocks all particles."},
	{"POWDER WALL",		PIXPACK(0x575757), PIXPACK(0x000000), 1,  "Allows powders, blocks all other particles."},
	{"CONDUCTOR",		PIXPACK(0xFFFF22), PIXPACK(0x101010), 2,  "Conductor. Allows all particles to pass through and conducts electricity."},
	{"EHOLE",			PIXPACK(0x242424), PIXPACK(0x101010), 0,  "E-Hole. absorbs particles, releases them when powered."},
	{"GAS WALL",		PIXPACK(0x579777), PIXPACK(0x000000), 1,  "Allows gases, blocks all other particles."},
	{"GRAVITY WALL",	PIXPACK(0xFFEE00), PIXPACK(0xAA9900), 4,  "Gravity wall. Newtonian Gravity has no effect inside a box drawn with this."},
	{"ENERGY WALL",		PIXPACK(0xFFAA00), PIXPACK(0xAA5500), 4,  "Allows energy particles, blocks all other particles."},
	{"ERASEALL",		PIXPACK(0x808080), PIXPACK(0x000000), -1, "Erases walls, particles, and signs."}
};

#endif
