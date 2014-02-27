#ifndef TOOLNUMBERS_H
#define TOOLNUMBERS_H
#include <string>
#include "graphics.h"

#define TOOL_HEAT	0
#define TOOL_COOL	1
#define TOOL_AIR	2
#define TOOL_VAC	3
#define TOOL_PGRV	4
#define TOOL_NGRV	5
#define TOOL_WIND	6
#define TOOL_PROP	7
#define TOOL_SIGN	8
#define TOOLCOUNT	9

#define OLD_PT_WIND	147
#define OLD_WL_SIGN	126
#define OLD_SPC_AIR	236
#define SPC_AIR		256

struct toolType
{
	std::string name;
	std::string identifier;
	pixel color;
	std::string descs;
};
typedef struct toolType toolType;

static toolType toolTypes[] =
{
	{"HEAT", "DEFAULT_TOOL_HEAT",	PIXPACK(0xFFBB00), "Heats the targeted element."},
	{"COOL", "DEFAULT_TOOL_COOL",	PIXPACK(0x00BBFF), "Cools the targeted element."},
	{"AIR",  "DEFAULT_TOOL_AIR",	PIXPACK(0xFFFFFF), "Air, creates airflow and pressure."},
	{"VAC",  "DEFAULT_TOOL_VAC",	PIXPACK(0x303030), "Vacuum, reduces air pressure."},
	{"PGRV", "DEFAULT_TOOL_PGRV",	PIXPACK(0xCCCCFF), "Creates a short-lasting gravity well."},
	{"NGRV", "DEFAULT_TOOL_NGRV",	PIXPACK(0xAACCFF), "Creates a short-lasting negative gravity well."},
	{"WIND", "DEFAULT_UI_WIND",		PIXPACK(0x000000), "Creates air movement."},
	{"PROP", "DEFAULT_UI_PROPERTY",	PIXPACK(0xFFAA00), "Property drawing tool."},
	{"SIGN", "DEFAULT_UI_SIGN",		PIXPACK(0x808080), "Sign. Displays text. Click on a sign to edit it or anywhere else to place a new one."}
};

#define DECO_DRAW		0
#define DECO_CLEAR		1
#define DECO_ADD		2
#define DECO_SUBTRACT	3
#define DECO_MULTIPLY	4
#define DECO_DIVIDE		5
#define DECO_SMUDGE		6
#define DECO_LIGHTEN	7
#define DECO_DARKEN		8
#define DECOCOUNT		9

struct decoType
{
	std::string name;
	std::string identifier;
	pixel color;
	std::string descs;
};
typedef struct decoType decoType;

static decoType decoTypes[] =
{
	{"SET", "DEFAULT_DECOR_SET",	PIXPACK(0xFF0000), "Draw decoration."},
	{"CLR", "DEFAULT_DECOR_CLR",	PIXPACK(0x000000), "Erase decoration."},
	{"ADD", "DEFAULT_DECOR_ADD",	PIXPACK(0x323232), "Color blending: Add."},
	{"SUB", "DEFAULT_DECOR_SUB",	PIXPACK(0x323232), "Color blending: Subtract."},
	{"MUL", "DEFAULT_DECOR_MUL",	PIXPACK(0x323232), "Color blending: Multiply."},
	{"DIV", "DEFAULT_DECOR_DIV",	PIXPACK(0x323232), "Color blending: Divide."},
	{"SMDG", "DEFAULT_DECOR_SMDG",	PIXPACK(0x00FF00), "Smudge tool, blends surrounding deco together."},
	{"LIGH", "DEFAULT_DECOR_LIGH",	PIXPACK(0xDDDDDD), "Lighten deco color."},
	{"DARK", "DEFAULT_DECOR_DARK",	PIXPACK(0x111111), "Darken deco color."}
};

#endif
