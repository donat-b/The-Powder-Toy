#ifndef GOLNUMBERS_H
#define GOLNUMBERS_H
#include <string>
#include "graphics.h"

//Old IDs for GOL types
#define GT_GOL 78
#define GT_HLIF 79
#define GT_ASIM 80
#define GT_2x2 81
#define GT_DANI 82
#define GT_AMOE 83
#define GT_MOVE 84
#define GT_PGOL 85
#define GT_DMOE 86
#define GT_34 87
#define GT_LLIF 88
#define GT_STAN 89
#define GT_SEED 134
#define GT_MAZE 135
#define GT_COAG 136
#define GT_WALL 137
#define GT_GNAR 138
#define GT_REPL 139
#define GT_MYST 140
#define GT_LOTE 142
#define GT_FRG2 143
#define GT_STAR 144
#define GT_FROG 145
#define GT_BRAN 146
 
//New IDs for GOL types
#define NGT_GOL 0
#define NGT_HLIF 1
#define NGT_ASIM 2
#define NGT_2x2 3
#define NGT_DANI 4
#define NGT_AMOE 5
#define NGT_MOVE 6
#define NGT_PGOL 7
#define NGT_DMOE 8
#define NGT_34 9
#define NGT_LLIF 10
#define NGT_STAN 11
#define NGT_SEED 12
#define NGT_MAZE 13
#define NGT_COAG 14
#define NGT_WALL 15
#define NGT_GNAR 16
#define NGT_REPL 17
#define NGT_MYST 18
#define NGT_LOTE 19
#define NGT_FRG2 20
#define NGT_STAR 21
#define NGT_FROG 22
#define NGT_BRAN 23

struct golType
{
	std::string name;
	std::string identifier;
	pixel colour;
	std::string description;
};
typedef struct golType golType;

static golType golTypes[NGOL] = 
{
	{"GOL",		"DEFAULT_PT_LIFE_GOL",	PIXPACK(0x0CAC00), "Game Of Life: Begin 3/Stay 23"},
	{"HLIF",	"DEFAULT_PT_LIFE_HLIF",	PIXPACK(0xFF0000), "High Life: B36/S23"},
	{"ASIM",	"DEFAULT_PT_LIFE_ASIM",	PIXPACK(0x0000FF), "Assimilation: B345/S4567"},
	{"2x2",		"DEFAULT_PT_LIFE_2x2",	PIXPACK(0xFFFF00), "2x2: B36/S125"},
	{"DANI",	"DEFAULT_PT_LIFE_DANI",	PIXPACK(0x00FFFF), "Day and Night: B3678/S34678"},
	{"AMOE",	"DEFAULT_PT_LIFE_AMOE",	PIXPACK(0xFF00FF), "Amoeba: B357/S1358"},
	{"MOVE",	"DEFAULT_PT_LIFE_MOVE",	PIXPACK(0xFFFFFF), "'Move' particles. Does not move things.. it is a life type: B368/S245"},
	{"PGOL",	"DEFAULT_PT_LIFE_PGOL",	PIXPACK(0xE05010), "Pseudo Life: B357/S238"},
	{"DMOE",	"DEFAULT_PT_LIFE_DMOE",	PIXPACK(0x500000), "Diamoeba: B35678/S5678"},
	{"34",		"DEFAULT_PT_LIFE_34",	PIXPACK(0x500050), "34: B34/S34"},
	{"LLIF",	"DEFAULT_PT_LIFE_LLIF",	PIXPACK(0x505050), "Long Life: B345/S5"},
	{"STAN",	"DEFAULT_PT_LIFE_STAN",	PIXPACK(0x5000FF), "Stains: B3678/S235678"},
	{"SEED",	"DEFAULT_PT_LIFE_SEED",	PIXPACK(0xFBEC7D), "Seeds: B2/S"},
	{"MAZE",	"DEFAULT_PT_LIFE_MAZE",	PIXPACK(0xA8E4A0), "Maze: B3/S12345"},
	{"COAG",	"DEFAULT_PT_LIFE_COAG",	PIXPACK(0x9ACD32), "Coagulations: B378/S235678"},
	{"WALL",	"DEFAULT_PT_LIFE_WALL",	PIXPACK(0x0047AB), "Walled cities: B45678/S2345"},
	{"GNAR",	"DEFAULT_PT_LIFE_GNAR",	PIXPACK(0xE5B73B), "Gnarl: B1/S1"},
	{"REPL",	"DEFAULT_PT_LIFE_REPL",	PIXPACK(0x259588), "Replicator: B1357/S1357"},
	{"MYST",	"DEFAULT_PT_LIFE_MYST",	PIXPACK(0x0C3C00), "Mystery: B3458/S05678"},
	{"LOTE",	"DEFAULT_PT_LIFE_LOTE",	PIXPACK(0xFF0000), "Living on the Edge: B37/S3458/4"},
	{"FRG2",	"DEFAULT_PT_LIFE_FRG2",	PIXPACK(0x00FF00), "Like Frogs rule: B3/S124/3"},
	{"STAR",	"DEFAULT_PT_LIFE_STAR",	PIXPACK(0x0000FF), "Like Star Wars rule: B278/S3456/6"},
	{"FROG",	"DEFAULT_PT_LIFE_FROG",	PIXPACK(0x00AA00), "Frogs: B34/S12/3"},
	{"BRAN",	"DEFAULT_PT_LIFE_BRAN",	PIXPACK(0xCCCC00), "Brian 6: B246/S6/3"}
};

static int oldgolTypes[NGOL] =
{
	GT_GOL,
	GT_HLIF,
	GT_ASIM,
	GT_2x2,
	GT_DANI,
	GT_AMOE,
	GT_MOVE,
	GT_PGOL,
	GT_DMOE,
	GT_34,
	GT_LLIF,
	GT_STAN,
	GT_SEED,
	GT_MAZE,
	GT_COAG,
	GT_WALL,
	GT_GNAR,
	GT_REPL,
	GT_MYST,
	GT_LOTE,
	GT_FRG2,
	GT_STAR,
	GT_FROG,
	GT_BRAN,
};

#endif
