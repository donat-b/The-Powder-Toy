#include <powder.h>

/* Weight Help
 * 1   = Gas   ||
 * 2   = Light || Liquids  0-49
 * 98  = Heavy || Powder  50-99
 * 100 = Solid ||
 * -1 is Neutrons and Photons
 */
part_type ptypes[PT_NUM] =
{
	//Name		Colour				Advec	Airdrag			Airloss	Loss	Collid	Grav	Diffus	Hotair			Fal	Burn	Exp	Mel	Hrd M	Use	Weight	Section			H						Ins		Description
	{"",		PIXPACK(0x000000),	0.0f,	0.00f * CFDS,	1.00f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	251,	"Erases particles.", ST_NONE, 0, NULL, NULL},
	{"DUST",	PIXPACK(0xFFE0A0),	0.7f,	0.02f * CFDS,	0.96f,	0.80f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	10,		0,	0,	30,	1,	1,	85,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	70,		"Very light dust. Flammable.", ST_SOLID, TYPE_PART|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC, NULL, &graphics_DUST},
	{"WATR",	PIXPACK(0x2030D0),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	1,	30,		SC_LIQUID,		R_TEMP-2.0f	+273.15f,	29,		"Liquid. Conducts electricity. Freezes. Extinguishes fires.", ST_LIQUID, TYPE_LIQUID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_NEUTPENETRATE, &update_WATR, NULL},
	{"OIL",		PIXPACK(0x404010),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	20,		0,	0,	5,	1,	1,	20,		SC_LIQUID,		R_TEMP+0.0f	+273.15f,	42,		"Liquid. Flammable.", ST_LIQUID, TYPE_LIQUID, NULL, NULL},
	{"FIRE",	PIXPACK(0xFF1000),	0.9f,	0.04f * CFDS,	0.97f,	0.20f,	0.0f,	-0.1f,	0.00f,	0.001f	* CFDS,	1,	0,		0,	0,	1,	1,	1,	2,		SC_EXPLOSIVE,	R_TEMP+400.0f+273.15f,	88,		"Ignites flammable materials. Heats air.", ST_GAS, TYPE_GAS|PROP_LIFE_DEC|PROP_LIFE_KILL, &update_PYRO, &graphics_FIRE},
	{"STNE",	PIXPACK(0xA0A0A0),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.3f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	5,	1,	1,	1,	90,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	150,	"Heavy particles. Meltable.", ST_SOLID, TYPE_PART, NULL, NULL},
	{"LAVA",	PIXPACK(0xE05010),	0.3f,	0.02f * CFDS,	0.95f,	0.80f,	0.0f,	0.15f,	0.00f,	0.0003f	* CFDS,	2,	0,		0,	0,	2,	1,	1,	45,		SC_LIQUID,		R_TEMP+1500.0f+273.15f,	60,		"Heavy liquid. Ignites flammable materials. Solidifies when cold.", ST_LIQUID, TYPE_LIQUID|PROP_LIFE_DEC, &update_PYRO, &graphics_LAVA},
	{"GUN",		PIXPACK(0xC0C0D0),	0.7f,	0.02f * CFDS,	0.94f,	0.80f,	-0.1f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	600,	1,	0,	10,	1,	1,	85,		SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	97,		"Light dust. Explosive.", ST_SOLID, TYPE_PART, NULL, NULL},
	{"NITR",	PIXPACK(0x20E010),	0.5f,	0.02f * CFDS,	0.92f,	0.97f,	0.0f,	0.2f,	0.00f,	0.000f	* CFDS,	2,	1000,	2,	0,	3,	1,	1,	23,		SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	50,		"Liquid. Pressure sensitive explosive.", ST_LIQUID, TYPE_LIQUID, NULL, NULL},
	{"CLNE",	PIXPACK(0xFFD010),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	251,	"Solid. Duplicates any particles it touches.", ST_SOLID, TYPE_SOLID|PROP_CLONE, NULL, NULL},
	{"GAS",		PIXPACK(0xE0FF20),	1.0f,	0.01f * CFDS,	0.99f,	0.30f,	-0.1f,	0.0f,	0.75f,	0.001f	* CFDS,	0,	600,	0,	0,	1,	1,	1,	1,		SC_GAS,			R_TEMP+2.0f	+273.15f,	42,		"Gas. Diffuses. Flammable. Liquefies under pressure.", ST_GAS, TYPE_GAS, NULL, NULL},
	{"C-4",		PIXPACK(0xD080E0),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	1000,	2,	50,	1,	1,	1,	100,	SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	88,		"Solid. Pressure sensitive explosive.", ST_SOLID, TYPE_SOLID | PROP_NEUTPENETRATE, NULL, NULL},
	{"GOO",		PIXPACK(0x804000),	0.0f,	0.00f * CFDS,	0.97f,	0.50f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	12,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	75,		"Solid. Deforms and disappears under pressure.", ST_SOLID, TYPE_SOLID | PROP_NEUTPENETRATE|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC, &update_GOO, NULL},
	{"ICE",		PIXPACK(0xA0C0FF),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	-0.0003f* CFDS,	0,	0,		0,	0,	20,	1,	1,	100,	SC_SOLIDS,		R_TEMP-50.0f+273.15f,	46,		"Solid. Freezes water. Crushes under pressure. Cools down air.", ST_SOLID, TYPE_SOLID|PROP_LIFE_DEC, &update_ICEI, NULL},
	{"METL",	PIXPACK(0x404060),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Solid. Conducts electricity. Meltable.", ST_SOLID, TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_HOT_GLOW, NULL, NULL},
	{"SPRK",	PIXPACK(0xFFFF80),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.001f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Electricity. Conducted by metal and water.", ST_SOLID, TYPE_SOLID|PROP_LIFE_DEC, &update_SPRK, &graphics_SPRK},
	{"SNOW",	PIXPACK(0xC0E0FF),	0.7f,	0.01f * CFDS,	0.96f,	0.90f,	-0.1f,	0.05f,	0.01f,	-0.00005f* CFDS,1,	0,		0,	0,	20,	1,	1,	50,		SC_POWDERS,		R_TEMP-30.0f+273.15f,	46,		"Light particles.", ST_SOLID, TYPE_PART|PROP_LIFE_DEC, &update_ICEI, NULL},
	{"WOOD",	PIXPACK(0xC0A040),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	20,		0,	0,	15,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	164,	"Solid. Flammable.", ST_SOLID, TYPE_SOLID | PROP_NEUTPENETRATE, NULL, NULL},
	{"NEUT",	PIXPACK(0x20E0FF),	0.0f,	0.00f * CFDS,	1.00f,	1.00f,	-0.99f,	0.0f,	0.01f,	0.002f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	-1,		SC_NUCLEAR,		R_TEMP+4.0f	+273.15f,	60,		"Neutrons. Interact with matter in odd ways.", ST_GAS, TYPE_ENERGY|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC, &update_NEUT, &graphics_NEUT},
	{"PLUT",	PIXPACK(0x407020),	0.4f,	0.01f * CFDS,	0.99f,	0.95f,	0.0f,	0.4f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	0,	1,	1,	90,		SC_NUCLEAR,		R_TEMP+4.0f	+273.15f,	251,	"Heavy particles. Fissile. Generates neutrons under pressure.", ST_SOLID, TYPE_PART|PROP_NEUTPENETRATE|PROP_RADIOACTIVE, &update_PLUT, NULL},
	{"PLNT",	PIXPACK(0x0CAC00),	0.0f,	0.00f * CFDS,	0.95f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	20,		0,	0,	10,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	65,		"Plant, drinks water and grows.", ST_SOLID, TYPE_SOLID|PROP_NEUTPENETRATE|PROP_LIFE_DEC, &update_PLNT, NULL},
	{"ACID",	PIXPACK(0xED55FF),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	40,		0,	0,	1,	1,	1,	10,		SC_LIQUID,		R_TEMP+0.0f	+273.15f,	34,		"Dissolves almost everything.", ST_LIQUID, TYPE_LIQUID|PROP_DEADLY, &update_ACID, &graphics_ACID},
	{"VOID",	PIXPACK(0x790B0B),	0.0f,	0.00f * CFDS,	1.00f,	0.00f,	0.0f,	0.0f,	0.00f,	-0.0003f* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	251,	"Hole, will drain away any particles.", ST_SOLID, TYPE_SOLID, NULL, NULL},
	{"WTRV",	PIXPACK(0xA0A0FF),	1.0f,	0.01f * CFDS,	0.99f,	0.30f,	-0.1f,	-0.1f,	0.75f,	0.0003f	* CFDS,	0,	0,		0,	0,	4,	1,	1,	1,		SC_GAS,			R_TEMP+100.0f+273.15f,	48,		"Steam, heats up air, produced from hot water.", ST_GAS, TYPE_GAS, &update_WTRV, NULL},
	{"CNCT",	PIXPACK(0xC0C0C0),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.3f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	2,	2,	1,	1,	55,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	100,	"Concrete, stronger than stone.", ST_SOLID, TYPE_PART|PROP_HOT_GLOW, NULL, NULL},
	{"DSTW",	PIXPACK(0x1020C0),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	1,	30,		SC_LIQUID,		R_TEMP-2.0f	+273.15f,	23,		"Distilled water, does not conduct electricity.", ST_LIQUID, TYPE_LIQUID|PROP_NEUTPENETRATE, &update_DSTW, NULL},
	{"SALT",	PIXPACK(0xFFFFFF),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.3f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	5,	1,	1,	1,	75,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	110,	"Salt, dissolves in water.", ST_SOLID, TYPE_PART, NULL, NULL},
	{"SLTW",	PIXPACK(0x4050F0),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	1,	35,		SC_LIQUID,		R_TEMP+0.0f	+273.15f,	75,		"Saltwater, conducts electricity, difficult to freeze.", ST_LIQUID, TYPE_LIQUID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_NEUTPENETRATE, &update_SLTW, NULL},
	{"DMND",	PIXPACK(0xCCFFFF),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	501,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	186,	"Diamond. Indestructible.", ST_SOLID, TYPE_SOLID|PROP_INDESTRUCTIBLE, NULL, NULL},
	{"BMTL",	PIXPACK(0x505070),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	251,	"Breakable metal.", ST_SOLID, TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_HOT_GLOW, &update_BMTL, NULL},
	{"BRMT",	PIXPACK(0x705060),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.3f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	2,	2,	1,	1,	90,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	211,	"Broken metal.", ST_SOLID, TYPE_PART|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_HOT_GLOW, &update_BRMT, NULL},
	{"PHOT",	PIXPACK(0xFFFFFF),	0.0f,	0.00f * CFDS,	1.00f,	1.00f,	-0.99f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	-1,		SC_NUCLEAR,		R_TEMP+900.0f+273.15f,	251,	"Photons. Travel in straight lines.", ST_GAS, TYPE_ENERGY|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC, &update_PHOT, &graphics_PHOT},
	{"URAN",	PIXPACK(0x707020),	0.4f,	0.01f * CFDS,	0.99f,	0.95f,	0.0f,	0.4f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	0,	1,	1,	90,		SC_NUCLEAR,		R_TEMP+30.0f+273.15f,	251,	"Heavy particles. Generates heat under pressure.", ST_SOLID, TYPE_PART | PROP_RADIOACTIVE, &update_URAN, NULL},
	{"WAX",		PIXPACK(0xF0F0BB),  0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	10,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	44,		"Wax. Melts at moderately high temperatures.", ST_SOLID, TYPE_SOLID, NULL, NULL},
	{"MWAX",	PIXPACK(0xE0E0AA),	0.3f,	0.02f * CFDS,	0.95f,	0.80f,	0.0f,	0.15f,	0.00f,	0.000001f* CFDS,2,	5,		0,	0,	2,	1,	1,	25,		SC_LIQUID,		R_TEMP+28.0f+273.15f,	44,		"Liquid Wax.", ST_LIQUID, TYPE_LIQUID, NULL, NULL},
	{"PSCN",	PIXPACK(0x805050),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"P-Type Silicon, Will transfer current to any conductor.", ST_SOLID, TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC, NULL, NULL},
	{"NSCN",	PIXPACK(0x505080),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"N-Type Silicon, Will not transfer current to P-Type Silicon.", ST_SOLID, TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC, NULL, NULL},
	{"LN2",		PIXPACK(0x80A0DF),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	0,	1,	1,	30,		SC_LIQUID,		70.15f,					70,		"Liquid Nitrogen. Very cold.", ST_SOLID, TYPE_LIQUID, NULL, NULL},
	{"INSL",	PIXPACK(0x9EA3B6),	0.0f,	0.00f * CFDS,	0.95f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	7,		0,	0,	10,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	0,		"Insulator, does not conduct heat or electricity.", ST_SOLID, TYPE_SOLID, NULL, NULL},
	{"VACU",	PIXPACK(0x303030),	0.0f,	0.00f * CFDS,	0.95f,	0.00f,	0.0f,	0.0f,	0.00f,	-0.01f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_SPECIAL,		R_TEMP+70.0f+273.15f,	255,	"Vacuum, sucks in other particles and heats up.", ST_NONE, TYPE_SOLID, NULL, NULL},
	{"VENT",	PIXPACK(0xEFEFEF),	0.0f,	0.00f * CFDS,	0.95f,	0.00f,	0.0f,	0.0f,	0.00f,	0.010f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_SPECIAL,		R_TEMP-16.0f+273.15f,	255,	"Air vent, creates pressure and pushes other particles away.", ST_NONE, TYPE_SOLID, NULL, NULL},
	{"RBDM",	PIXPACK(0xCCCCCC),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	1000,	1,	50,	1,	1,	1,	100,	SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	240,	"Rubidium, explosive, especially on contact with water, low melting point", ST_SOLID, TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC, NULL, NULL},
	{"LRBD",	PIXPACK(0xAAAAAA),	0.3f,	0.02f * CFDS,	0.95f,	0.80f,	0.0f,	0.15f,	0.00f,	0.000001f* CFDS,2,	1000,	1,	0,	2,	1,	1,	45,		SC_EXPLOSIVE,	R_TEMP+45.0f+273.15f,	170,	"Liquid Rubidium.", ST_LIQUID, TYPE_LIQUID|PROP_CONDUCTS|PROP_LIFE_DEC, NULL, NULL},
	{"NTCT",	PIXPACK(0x505040),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Semi-conductor. Only conducts electricity when hot (More than 100C)", ST_SOLID, TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC, &update_NPTCT, NULL},
	{"SAND",	PIXPACK(0xFFD090),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.3f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	5,	1,	1,	1,	90,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	150,	"Sand, Heavy particles. Meltable.", ST_SOLID, TYPE_PART, NULL, NULL},
	{"GLAS",	PIXPACK(0x404040),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	150,	"Solid. Meltable. Shatters under pressure", ST_SOLID, TYPE_SOLID | PROP_NEUTPASS | PROP_HOT_GLOW | PROP_SPARKSETTLE, &update_GLAS, NULL},
	{"PTCT",	PIXPACK(0x405050),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Semi-conductor. Only conducts electricity when cold (Less than 100C)", ST_SOLID, TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC, &update_NPTCT, NULL},
	{"BGLA",	PIXPACK(0x606060),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.3f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	5,	2,	1,	1,	90,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	150,	"Broken Glass, Heavy particles. Meltable. Bagels.", ST_SOLID, TYPE_PART | PROP_HOT_GLOW, NULL, NULL},
	{"THDR",	PIXPACK(0xFFFFA0),	0.0f,	0.00f * CFDS,	1.0f,	0.30f,	-0.99f,	0.6f,	0.62f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	1,		SC_EXPLOSIVE,	9000.0f		+273.15f,	1,		"Lightning! Very hot, inflicts damage upon most materials, transfers current to metals.", ST_NONE, TYPE_ENERGY, &update_THDR, &graphics_THDR},
	{"PLSM",	PIXPACK(0xBB99FF),	0.9f,	0.04f * CFDS,	0.97f,	0.20f,	0.0f,	-0.1f,	0.30f,	0.001f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	1,		SC_GAS,			10000.0f	+273.15f,	5,		"Plasma, extremely hot.", ST_NONE, TYPE_GAS|PROP_LIFE_DEC|PROP_LIFE_KILL, &update_PYRO, &graphics_PLSM},
	{"ETRD",	PIXPACK(0x404040),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Electrode. Creates a surface that allows Plasma arcs. (Use sparingly)", ST_NONE, TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC, NULL, NULL},
	{"NICE",	PIXPACK(0xC0E0FF),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	-0.0005f* CFDS,	0,	0,		0,	0,	20,	1,	1,	100,	SC_SOLIDS,		35.0f,					46,		"Nitrogen Ice.", ST_SOLID, TYPE_SOLID, NULL, NULL},
	{"NBLE",	PIXPACK(0xEB4917),	1.0f,	0.01f * CFDS,	0.99f,	0.30f,	-0.1f,	0.0f,	0.75f,	0.001f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	1,		SC_GAS,			R_TEMP+2.0f	+273.15f,	106,	"Noble Gas. Diffuses. Conductive. Ionizes into plasma when introduced to electricity", ST_GAS, TYPE_GAS|PROP_CONDUCTS|PROP_LIFE_DEC, &update_NBLE, NULL},
	{"BTRY",	PIXPACK(0x858505),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Solid. Generates Electricity.", ST_SOLID, TYPE_SOLID, &update_BTRY, NULL},
	{"LCRY",	PIXPACK(0x505050),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_POWERED,		R_TEMP+0.0f	+273.15f,	251,	"Liquid Crystal. Changes colour when charged. (PSCN Charges, NSCN Discharges)", ST_SOLID, TYPE_SOLID, &update_LCRY, &graphics_LCRY},
	{"STKM",	PIXPACK(0x000000),	0.5f,	0.00f * CFDS,	0.2f,	1.0f,	0.0f,	0.0f,	0.0f,	0.00f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	50,		SC_SPECIAL,		R_TEMP+14.6f+273.15f,	0,		"Stickman. Don't kill him!", ST_NONE, 0, &update_STKM, &graphics_STKM},
	{"SWCH",	PIXPACK(0x103B11),	0.0f,	0.00f * CFDS,	0.90f,  0.00f,  0.0f,	0.0f,	0.00f,  0.000f  * CFDS, 0,	0,		0,	0,	1,	1,	1,	100,	SC_POWERED,		R_TEMP+0.0f	+273.15f,	251,	"Solid. Only conducts when switched on. (PSCN switches on, NSCN switches off)", ST_SOLID, TYPE_SOLID|PROP_POWERED, &update_SWCH, &graphics_SWCH},
	{"SMKE",	PIXPACK(0x222222),	0.9f,	0.04f * CFDS,	0.97f,	0.20f,	0.0f,	-0.1f,	0.00f,	0.001f	* CFDS,	1,	0,		0,	0,	1,	1,	1,	1,		SC_GAS,			R_TEMP+320.0f+273.15f,	88,		"Smoke", ST_SOLID, TYPE_GAS|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC, NULL, &graphics_SMKE},
	{"DESL",	PIXPACK(0x440000),	1.0f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.0f,	0.0f	* CFDS,	2,	2,		0,	0,	5,	1,	1,	15,		SC_LIQUID,		R_TEMP+0.0f	+273.15f,	42,		"Liquid. Explodes under high pressure and temperatures", ST_LIQUID, TYPE_LIQUID, NULL, NULL},
	{"COAL",	PIXPACK(0x222222),	0.0f,   0.00f * CFDS,   0.90f,  0.00f,  0.0f,   0.0f,   0.0f,   0.0f	* CFDS, 0,	0,		0,	0,	20,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	200,	"Solid. Burns slowly.", ST_SOLID, TYPE_SOLID, &update_COAL, &graphics_COAL},
	{"LOXY",	PIXPACK(0x80A0EF),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	5000,  	0,	0,	0,	1,	1,	30,		SC_LIQUID,		80.0f,					70,		"Liquid Oxygen. Very cold. Reacts with fire", ST_LIQUID, TYPE_LIQUID, NULL, NULL},
	{"OXYG",	PIXPACK(0x80A0FF),	2.0f,   0.00f * CFDS,   0.99f,	0.30f,	-0.1f,	0.0f,	3.0f,	0.000f	* CFDS,	0,	0,  	0,	0,	0,	1,	1,	1,		SC_GAS,		 	R_TEMP+0.0f	+273.15f,   70,		"Gas. Ignites easily.", ST_GAS, TYPE_GAS, &update_O2, NULL},
	{"INWR",	PIXPACK(0x544141),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Insulated Wire. Doesn't conduct to metal or semiconductors.", ST_SOLID, TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC, NULL, NULL},
	{"YEST",	PIXPACK(0xEEE0C0),	0.7f,	0.02f * CFDS,	0.96f,	0.80f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	15,		0,	0,	30,	1,	1,	80,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	70,		"Yeast, grows when warm (~37C).", ST_SOLID, TYPE_PART, &update_YEST, NULL},
	{"DYST",	PIXPACK(0xBBB0A0),	0.7f,	0.02f * CFDS,	0.96f,	0.80f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	20,		0,	0,	30,	0,	1,	80,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	70,		"Dead Yeast.", ST_SOLID, TYPE_PART, NULL, NULL},
	{"THRM",	PIXPACK(0xA08090),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.3f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	2,	2,	1,	1,	90,		SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	211,	"Thermite. Burns at extremely high temperature.", ST_SOLID, TYPE_PART, &update_THRM, NULL},
	{"GLOW",	PIXPACK(0x445464),	0.3f,	0.02f * CFDS,	0.98f,	0.80f,	0.0f,	0.15f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	2,	1,	1,	40,		SC_LIQUID,		R_TEMP+20.0f+273.15f,	44,		"Glow, Glows under pressure", ST_LIQUID, TYPE_LIQUID|PROP_LIFE_DEC, &update_GLOW, &graphics_GLOW},
	{"BRCK",	PIXPACK(0x808080),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	251,	"Brick, breakable building material.", ST_SOLID, TYPE_SOLID|PROP_HOT_GLOW, NULL, NULL},
	{"CFLM",	PIXPACK(0x8080FF),	0.9f,	0.04f * CFDS,	0.97f,	0.20f,	0.0f,	-0.1f,	0.00f,	0.0005f	* CFDS,	1,	0,		0,	0,	1,	1,	1,	2,		SC_EXPLOSIVE,	0.0f,					88,		"Sub-zero flame.", ST_LIQUID, TYPE_GAS|PROP_LIFE_DEC|PROP_LIFE_KILL, NULL, &graphics_HFLM},
	{"FIRW",	PIXPACK(0xFFA040),	0.7f,	0.02f * CFDS,	0.96f,	0.80f,	-0.99f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	30,	1,	1,	55,		SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	70,		"Fireworks!", ST_SOLID, TYPE_PART|PROP_LIFE_DEC, &update_FIRW, &graphics_FIRW},
	{"FUSE",	PIXPACK(0x0A5706),	0.0f,   0.00f * CFDS,   0.90f,  0.00f,  0.0f,   0.0f,   0.0f,   0.0f	* CFDS, 0,	0,		0,	0,	20,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	200,	"Solid. Burns slowly. Ignites at somewhat high temperatures and electricity.", ST_SOLID, TYPE_SOLID, &update_FUSE, NULL},
	{"FSEP",	PIXPACK(0x63AD5F),	0.7f,	0.02f * CFDS,	0.96f,	0.80f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	30,	1,	1,	70,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	70,		"Fuse Powder. See FUSE.", ST_SOLID, TYPE_PART, &update_FSEP, NULL},
	{"AMTR",	PIXPACK(0x808080),	0.7f,   0.02f * CFDS,   0.96f,  0.80f,  0.00f,  0.10f,  1.00f,  0.0000f * CFDS, 0,	0,		0,	0,	0,	1,	1,	100,	SC_NUCLEAR,	 	R_TEMP+0.0f +273.15f,	70,		"Anti-Matter, Destroys a majority of particles", ST_NONE, TYPE_PART, &update_AMTR, NULL}, //Maybe TYPE_ENERGY?
	{"BCOL",	PIXPACK(0x333333),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.3f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	5,	2,	1,	1,	90,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	150,	"Broken Coal. Heavy particles. See COAL", ST_SOLID, TYPE_PART, &update_BCOL, NULL},
	{"PCLN",	PIXPACK(0x3B3B0A),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_POWERED,		R_TEMP+0.0f	+273.15f,	251,	"Solid. When activated, duplicates any particles it touches.", ST_NONE, TYPE_SOLID|PROP_CLONE|PROP_POWERED, NULL, &graphics_PCLN},
	{"HSWC",	PIXPACK(0x3B0A0A),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_POWERED,		R_TEMP+0.0f	+273.15f,	251,	"Heat switch. Conducts Heat only when activated", ST_NONE, TYPE_SOLID|PROP_POWERED, NULL, &graphics_HSWC},
	{"IRON",	PIXPACK(0x707070),	0.0f,	0.00f * CFDS,	0.90f,  0.00f,  0.0f,	0.0f,	0.00f,  0.000f	* CFDS, 0,	0,		0,	1,	50,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f +273.15f,	251,	"Rusts with salt, can be used for electrolysis of WATR", ST_SOLID, TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_HOT_GLOW, &update_IRON, NULL},
	{"MORT",	PIXPACK(0xE0E0E0),	0.0f,	0.00f * CFDS,	1.00f,	1.00f,	-0.99f,	0.0f,	0.01f,	0.002f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	-1,		SC_CRACKER,		R_TEMP+4.0f	+273.15f,	60,		"Steam Train.", ST_NONE, TYPE_PART, &update_MORT, NULL},
	{"LIFE",	PIXPACK(0x0CAC00),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	0,	1,	100,	SC_LIFE,		9000.0f,				40,		"Game Of Life! B3/S23", ST_NONE, TYPE_SOLID|PROP_LIFE, NULL, &graphics_LIFE},
	{"DLAY",	PIXPACK(0x753590),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		4.0f+273.15f,			0,		"Conducts with temperature-dependent delay. (use HEAT/COOL).", ST_SOLID, TYPE_SOLID, &update_DLAY, &graphics_DLAY},
	{"CO2",		PIXPACK(0x666666),	2.0f,   0.00f * CFDS,   0.99f,	0.30f,	-0.1f,	0.1f,	1.0f,	0.000f	* CFDS,	1,	0,  	0,	0,	0,	1,	1,	1,		SC_GAS,			R_TEMP+273.15f,			88,		"Carbon Dioxide", ST_GAS, TYPE_GAS, &update_CO2, NULL},
	{"DRIC",	PIXPACK(0xE0E0E0),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	-0.0005f* CFDS,	0,	0,		0,	0,	20,	1,	1,	100,	SC_SOLIDS,		172.65f,				2,		"Dry Ice.", ST_SOLID, TYPE_SOLID, NULL, NULL},
	{"BUBW",	PIXPACK(0x2030D0),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	1,	30,		SC_LIQUID,		R_TEMP-2.0f	+273.15f,	29,		"Carbonated water. Conducts electricity. Freezes. Extinguishes fires.", ST_LIQUID, TYPE_LIQUID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_NEUTPENETRATE, &update_CBNW, &graphics_CBNW},
	{"STOR",	PIXPACK(0x50DFDF),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_POWERED,		R_TEMP+0.0f	+273.15f,	251,	"Solid. Stores a single particle, releases when charged with PSCN, also passes to PIPE", ST_NONE, TYPE_SOLID, &update_STOR, &graphics_STOR},
	{"PVOD",	PIXPACK(0x792020),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_POWERED,		R_TEMP+0.0f	+273.15f,	251,	"Solid. When activated, destroys entering particles", ST_NONE, TYPE_SOLID|PROP_POWERED, NULL, &graphics_PVOD},
	{"CONV",	PIXPACK(0x0AAB0A),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	251,	"Solid. Converts whatever touches it into its ctype.", ST_NONE, TYPE_SOLID, &update_CONV, NULL},
	{"CAUS",	PIXPACK(0x80FFA0),	2.0f,   0.00f * CFDS,   0.99f,	0.30f,	-0.1f,	0.0f,	1.50f,	0.000f	* CFDS,	0,	0,  	0,	0,	0,	1,	1,	1,		SC_GAS,		 	R_TEMP+0.0f	+273.15f,   70,		"Caustic Gas, acts like Acid", ST_GAS, TYPE_GAS|PROP_DEADLY, &update_CAUS, NULL},
	{"LIGH",	PIXPACK(0xFFFFC0),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	0,	    "More realistic lightning. Set pen size to set the size of the lightning.", ST_SOLID, TYPE_SOLID, &update_LIGH, &graphics_LIGH},
    {"TESC",	PIXPACK(0x707040),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Tesla coil!", ST_SOLID, TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_HOT_GLOW, NULL, NULL},
    {"DEST",	PIXPACK(0xFF3311),  -0.05f,	0.00f * CFDS,	0.95f,	0.95f,	-0.1f,	0.4f,	0.00f,	0.000f	* CFDS,	1,	0,		0,  0,	0,	1,	1,	101,	SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	150,	"More destructive Bomb.", ST_SOLID, TYPE_PART|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC, &update_DEST, &graphics_DEST},
	{"SPNG",  	PIXPACK(0xFFBE30),	0.00f, 	0.00f * CFDS,   0.00f,  1.00f,   0.00f, 0.0f,   0.00f,  0.000f  * CFDS, 0, 	20, 	0,  1, 	30,	1, 	1,	100,    SC_SOLIDS,  	R_TEMP+0.0f +273.15f,   251,    "A sponge, absorbs water.", ST_SOLID, TYPE_SOLID, &update_SPNG, &graphics_SPNG},
	{"RIME",  	PIXPACK(0xCCCCCC),	0.00f, 	0.00f * CFDS,   0.00f,  1.00f,   0.00f, 0.0f,   0.00f,  0.000f  * CFDS, 0, 	0, 		0, 	0, 	30,	1,  1,	100,    SC_CRACKER,  	243.15f,				100,    "Not quite Ice", ST_SOLID, TYPE_SOLID, &update_RIME, NULL},
	{"FOG",  	PIXPACK(0xAAAAAA),	0.8f,	0.00f * CFDS,	0.4f,	0.70f,	-0.1f,	0.0f,	0.99f,	0.000f	* CFDS, 0, 	0, 		0,  0,  30, 1,  1,	1,		SC_CRACKER,  	243.15f,				100,    "Not quite Steam", ST_GAS, TYPE_GAS|PROP_LIFE_DEC, &update_FOG, NULL},
	{"BCLN",	PIXPACK(0xFFD040),	0.0f,	0.00f * CFDS,	0.97f,	0.50f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	12,	1,	1,	100,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	251,	"Breakable Clone.", ST_NONE, TYPE_SOLID|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC|PROP_BREAKABLECLONE, NULL, NULL},
	{"LOVE",	PIXPACK(0xFF30FF),	0.0f,	0.00f * CFDS,	0.00f,	0.00f,	0.0f,	0.0f,	0.0f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_CRACKER,		373.0f,					40,		"Love...", ST_GAS, TYPE_SOLID, &update_MISC, NULL},
	{"DEUT",  	PIXPACK(0x00153F),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	1,	31,		SC_NUCLEAR,		R_TEMP-2.0f	+273.15f,	251,	"Deuterium oxide. Volume changes with temp, radioactive with neutrons.", ST_LIQUID, TYPE_LIQUID|PROP_NEUTPENETRATE, &update_DEUT, &graphics_DEUT},
	{"WARP",  	PIXPACK(0x000000),	0.8f,	0.00f * CFDS,	0.9f,	0.70f,	-0.1f,	0.0f,	3.00f,	0.000f	* CFDS, 0, 	0, 		0,  0,  30, 1,  1,	1,		SC_NUCLEAR,  	R_TEMP +273.15f,		100,    "Displaces other elements.", ST_GAS, TYPE_GAS|PROP_LIFE_DEC|PROP_LIFE_KILL, &update_WARP, NULL},
	{"PUMP",	PIXPACK(0x0A0A3B),	0.0f,	0.00f * CFDS,	0.95f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	10,	1,	1,	100,	SC_POWERED,		273.15f,				0,		"Changes pressure to its temp when activated. (use HEAT/COOL).", ST_SOLID, TYPE_SOLID|PROP_POWERED, &update_PUMP, &graphics_PUMP},
	{"FWRK",	PIXPACK(0x666666),	0.4f,	0.01f * CFDS,	0.99f,	0.95f,	0.0f,	0.4f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	1,	1,	1,	97,		SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	100,	"First fireworks made, activated by heat/neutrons.", ST_SOLID, TYPE_PART|PROP_LIFE_DEC, &update_FWRK, NULL},
	{"PIPE",	PIXPACK(0x444444),	0.0f,	0.00f * CFDS,	0.95f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_SOLIDS,		273.15f,				0,		"Moves elements around, read FAQ on website for help.", ST_SOLID, TYPE_SOLID|PROP_LIFE_DEC, &update_PIPE, &graphics_PIPE},
	{"FRZZ",	PIXPACK(0xC0E0FF),	0.7f,	0.01f * CFDS,	0.96f,	0.90f,	-0.1f,	0.05f,	0.01f,	-0.00005f* CFDS,1,	0,		0,	0,	20,	1,	1,	50,		SC_POWDERS,		90.0f,					46,		"FREEZE", ST_SOLID, TYPE_PART, &update_FRZZ, NULL},
	{"FRZW",	PIXPACK(0x1020C0),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	1,	30,		SC_CRACKER,		120.0f,					29,		"FREEZE WATER", ST_LIQUID, TYPE_LIQUID||PROP_LIFE_DEC, &update_FRZW, NULL},
	{"GRAV",	PIXPACK(0xFFE0A0),	0.7f,	0.00f * CFDS,	1.00f,	1.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	1,	10,		0,	0,	30,	1,	1,	85,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	70,		"Very light dust. Changes colour based on velocity.", ST_SOLID, TYPE_PART, &update_MISC, &graphics_GRAV},
	{"BIZR",	PIXPACK(0x00FF77),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	1,	30,		SC_LIQUID,		R_TEMP+0.0f	+273.15f,	29,		"Bizarre... contradicts the normal state changes.", ST_LIQUID, TYPE_LIQUID, &update_BIZR, &graphics_BIZR},
	{"BIZG",	PIXPACK(0x00FFBB),	1.0f,	0.01f * CFDS,	0.99f,	0.30f,	-0.1f,	0.0f,	2.75f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	1,		SC_CRACKER,		R_TEMP-200.0f+273.15f,	42,		"Bizarre gas", ST_GAS, TYPE_GAS, &update_BIZR, &graphics_BIZR},
	{"BIZS",	PIXPACK(0x00E455),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_CRACKER,		R_TEMP+300.0f+273.15f,	251,	"Bizarre solid", ST_SOLID, TYPE_SOLID, &update_BIZR, &graphics_BIZR},
	{"INST",	PIXPACK(0x404039),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Instantly conducts, PSCN to charge, NSCN to take.", ST_SOLID, TYPE_SOLID|PROP_LIFE_DEC, NULL, NULL},
	{"ISOZ",	PIXPACK(0xAA30D0),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	0,	1,	1,	24,		SC_NUCLEAR,		R_TEMP-2.0f	+273.15f,	29,		"Radioactive liquid", ST_LIQUID, TYPE_LIQUID|PROP_NEUTPENETRATE, &update_ISZ, NULL},
	{"ISZS",	PIXPACK(0x662089),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	-0.0007f* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_NUCLEAR,		140.00f,				251,	"Solid form of ISOZ, slowly decays.", ST_SOLID, TYPE_SOLID, &update_ISZ, NULL},
	{"PRTI",	PIXPACK(0xEB5917),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	-0.005f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	0,		"Portal IN.  Things go in here, now with channels (same as WIFI)", ST_SOLID, TYPE_SOLID, &update_PRTI, &graphics_PRTI},
	{"PRTO",	PIXPACK(0x0020EB),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.005f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	0,		"Portal OUT.  Things come out here, now with channels (same as WIFI)", ST_SOLID, TYPE_SOLID, &update_PRTO, &graphics_PRTO},
	{"PSTE",	PIXPACK(0xAA99AA),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	1,	31,		SC_LIQUID,		R_TEMP-2.0f	+273.15f,	29,		"Colloid, Hardens under pressure", ST_LIQUID, TYPE_LIQUID, NULL, NULL},
	{"PSTS",	PIXPACK(0x776677),	0.0f,	0.00f * CFDS,	0.00f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	20,	0,	1,	100,	SC_CRACKER,		R_TEMP-2.0f	+273.15f,	29,		"Solid form of PSTE, temporary", ST_SOLID, TYPE_SOLID, NULL, NULL},
	{"ANAR",	PIXPACK(0xFFFFEE),	-0.7f,	-0.02f* CFDS,	0.96f,	0.80f,	0.1f,	-0.1f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	30,	1,	1,	85,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	70,		"Very light dust. Behaves opposite gravity", ST_SOLID, TYPE_PART, &update_ANAR, NULL},
	{"VINE",	PIXPACK(0x079A00),	0.0f,	0.00f * CFDS,	0.95f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	20,		0,	0,	10,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f +273.15f,	65,		"Vine, grows", ST_SOLID, TYPE_SOLID, &update_VINE, NULL},
	{"INVS",	PIXPACK(0x00CCCC),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	15,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	164,	"Invisible to everything while under pressure.", ST_SOLID, TYPE_SOLID | PROP_NEUTPASS, NULL, &graphics_INVS},
	{"EQVE",	PIXPACK(0xFFE0A0),	0.7f,	0.02f * CFDS,	0.96f,	0.80f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	30,	0,	1,	85,		SC_CRACKER,		R_TEMP+0.0f	+273.15f,	70,		"Shared velocity test", ST_SOLID, TYPE_PART, NULL, NULL},
	{"SPWN2",	PIXPACK(0xAAAAAA),	0.0f,	0.00f * CFDS,	1.00f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	0,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	0,		"STK2 spawn point", ST_SOLID, TYPE_SOLID, &update_SPAWN2, NULL},
	{"SPWN",	PIXPACK(0xAAAAAA),	0.0f,	0.00f * CFDS,	1.00f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	0,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	0,		"STKM spawn point", ST_SOLID, TYPE_SOLID, &update_SPAWN, NULL},
	{"SHLD",	PIXPACK(0xAAAAAA),	0.0f,	0.00f * CFDS,	1.00f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	0,		"Shield, spark it to grow", ST_SOLID, TYPE_SOLID|PROP_LIFE_DEC, &update_SHLD1, NULL},
	{"SHD2",	PIXPACK(0x777777),	0.0f,	0.00f * CFDS,	1.00f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	0,	1,	100,	SC_CRACKER,		R_TEMP+0.0f	+273.15f,	0,		"Shield lvl 2", ST_SOLID, TYPE_SOLID|PROP_LIFE_DEC, &update_SHLD2, NULL},
	{"SHD3",	PIXPACK(0x444444),	0.0f,	0.00f * CFDS,	1.00f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	0,	1,	100,	SC_CRACKER,		R_TEMP+0.0f	+273.15f,	0,		"Shield lvl 3", ST_SOLID, TYPE_SOLID|PROP_LIFE_DEC, &update_SHLD3, NULL},
	{"SHD4",	PIXPACK(0x212121),	0.0f,	0.00f * CFDS,	1.00f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	0,	1,	100,	SC_CRACKER,		R_TEMP+0.0f	+273.15f,	0,		"Shield lvl 4", ST_SOLID, TYPE_SOLID|PROP_LIFE_DEC, &update_SHLD4, NULL},
	{"LOLZ",	PIXPACK(0x569212),	0.0f,	0.00f * CFDS,	0.00f,	0.00f,	0.0f,	0.0f,	0.0f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_CRACKER,		373.0f,					40,		"Lolz", ST_GAS, TYPE_SOLID, &update_MISC, NULL},
	{"WIFI",	PIXPACK(0x40A060),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	2,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	0,		"Wireless transmitter, color coded.", ST_SOLID, TYPE_SOLID, &update_WIFI, &graphics_WIFI},
	{"FILT",	PIXPACK(0x000056),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	251,	"Filter for photons, changes the color.", ST_SOLID, TYPE_SOLID, NULL, &graphics_FILT},
	{"ARAY",	PIXPACK(0xFFBB00),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f +273.15f,	0,		"Ray Emitter. Rays create points when they collide", ST_SOLID, TYPE_SOLID|PROP_LIFE_DEC, &update_ARAY, NULL},
	{"BRAY",	PIXPACK(0xFFFFFF),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	0,	1,	100,	SC_ELEC,		R_TEMP+0.0f +273.15f,	251,	"Ray Point. Rays create points when they collide", ST_SOLID, TYPE_SOLID|PROP_LIFE_DEC|PROP_LIFE_KILL, NULL, &graphics_BRAY},
	{"STK2",	PIXPACK(0x000000),	0.5f,	0.00f * CFDS,	0.2f,	1.0f,	0.0f,	0.0f,	0.0f,	0.00f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	50,		SC_SPECIAL,		R_TEMP+14.6f+273.15f,	0,		"Stickman. Don't kill him!", ST_NONE, 0, &update_STKM2, &graphics_STKM2},
	{"BOMB",	PIXPACK(0xFFF288),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	20,	1,	1,	30,		SC_EXPLOSIVE,	R_TEMP-2.0f	+273.15f,	29,		"Bomb.", ST_NONE, TYPE_PART|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC|PROP_SPARKSETTLE, &update_BOMB, &graphics_BOMB},
	{"C-5",		PIXPACK(0x2050E0),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	88,		"Cold explosive", ST_SOLID, TYPE_SOLID | PROP_NEUTPENETRATE, &update_C5, NULL},
	{"SING",	PIXPACK(0x242424),	0.7f,	0.36f * CFDS,	0.96f,	0.80f,	0.1f,	0.12f,	0.00f,	-0.001f	* CFDS,	1,	0,		0,	0,	0,	1,	1,	86,		SC_NUCLEAR,		R_TEMP+0.0f	+273.15f,	70,		"Singularity", ST_SOLID, TYPE_PART|PROP_LIFE_DEC, &update_SING, NULL},
	{"QRTZ",	PIXPACK(0xAADDDD),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	3,		"Quartz, breakable mineral. Conducts but becomes brittle at lower temperatures.", ST_SOLID, TYPE_SOLID|PROP_HOT_GLOW|PROP_LIFE_DEC, &update_QRTZ, &graphics_QRTZ},
	{"PQRT",	PIXPACK(0x88BBBB),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.27f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	0,	1,	1,	90,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	3,		"Broken quartz.", ST_SOLID, TYPE_PART| PROP_HOT_GLOW, &update_QRTZ, &graphics_QRTZ},
	{"EMP",	    PIXPACK(0x66AAFF),	0.0f,   0.00f * CFDS,   0.90f,  0.00f,  0.0f,   0.0f,   0.0f,   0.0f	* CFDS, 0,	0,		0,	0,	3,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	121,	"Breaks activated electronics.", ST_SOLID, TYPE_SOLID|PROP_LIFE_DEC, &update_EMP, &graphics_EMP},
	{"BREL",	PIXPACK(0x707060),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.18f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	2,	2,	1,	1,	90,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	211,	"Broken electronics", ST_SOLID, TYPE_PART|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_HOT_GLOW, NULL, NULL},
	{"ELEC",	PIXPACK(0xDFEFFF),	0.0f,	0.00f * CFDS,	1.00f,	1.00f,	-0.99f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	-1,		SC_NUCLEAR,		R_TEMP+200.0f+273.15f,	251,	"Electrons", ST_GAS, TYPE_ENERGY|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC, &update_ELEC, &graphics_ELEC},
	{"ACEL",	PIXPACK(0x0099CC),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_FORCE,		R_TEMP+0.0f	+273.15f,	251,	"Accelerator", ST_NONE, TYPE_SOLID, &update_ACEL, &graphics_ACEL},
	{"DCEL",	PIXPACK(0x99CC00),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_FORCE,		R_TEMP+0.0f	+273.15f,	251,	"Decelerator", ST_NONE, TYPE_SOLID, &update_DCEL, &graphics_DCEL},
	{"TNT",		PIXPACK(0xC05050),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	88,		"Explosive.", ST_SOLID, TYPE_SOLID | PROP_NEUTPENETRATE, &update_BANG, NULL},
	{"IGNC",	PIXPACK(0xC0B050),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	88,		"Ignition cord.", ST_SOLID, TYPE_SOLID | PROP_NEUTPENETRATE | PROP_SPARKSETTLE | PROP_LIFE_KILL, &update_IGNT, NULL},
	{"BOYL",	PIXPACK(0x0A3200),	1.0f,	0.01f * CFDS,	0.99f,	0.30f,	-0.1f,	0.0f,	0.18f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	1,		SC_GAS,			R_TEMP+2.0f	+273.15f,	42,		"Boyle, variable pressure gas. Expands when heated.", ST_GAS, TYPE_GAS, &update_BOYL, NULL},
	{"PPTO",	PIXPACK(0x0020EB),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.005f	* CFDS,	0,	0,		0,	0,	0,	0,	0,	100,	SC_SPECIAL,		R_TEMP	+273.15f,   	0,		"Powered Portal OUT.  Things come out here, now with channels (same as WIFI)", ST_SOLID, TYPE_SOLID, &update_PRTO, &graphics_PRTO},
	{"VIRS",	PIXPACK(0xFE11F6),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	100,	0,	0,	20,	0,	0,	31,		SC_LIQUID,		R_TEMP+50.0f +273.15f,	251,	"Virus. Turns everything it touches into virus", ST_LIQUID, TYPE_LIQUID, &update_VIRS, NULL},
	{"VRSS",	PIXPACK(0xD408CD),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	5,		0,	1,	1,	0,	0,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	251,	"Solid Virus. Turns everything it touches into virus", ST_SOLID, TYPE_SOLID, &update_VIRS, NULL},
	{"VRSG",	PIXPACK(0xFE68FE),	1.0f,	0.01f * CFDS,	0.99f,	0.30f,	-0.1f,	0.0f,	0.75f,	0.000f	* CFDS,	0,	500,	0,	0,	0,	0,	0,	1,		SC_GAS,			R_TEMP+500.0f+273.15f,	251,	"Gas Virus. Turns everything it touches into virus", ST_GAS, TYPE_GAS, &update_VIRS, NULL},
	{"CURE",	PIXPACK(0x8BE700),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	1,		0,	0,	20,	0,	0,	32,		SC_LIQUID,		R_TEMP+0.0f +273.15f,	251,	"Cure. Turns virus back into what it was before", ST_LIQUID, TYPE_LIQUID, NULL, NULL},
	{"WIND",	PIXPACK(0x000000),  0.0f,	0.00f * CFDS,	0.90f,  0.00f,  0.0f,	0.0f,	0.00f,	0.000f  * CFDS,	0,  0,		0,  0,  0,  0,  0,	100,	SC_SPECIAL,		0.0f,					40,		"", ST_NONE, ST_NONE, NULL, NULL},
	{"HYGN",	PIXPACK(0x5070FF),	2.0f,	0.00f * CFDS,	0.99f,	0.30f,	-0.10f,	0.00f,	3.00f,	0.000f	* CFDS, 0,  0,		0,	0,	0,	1,	1,	1,		SC_GAS,			R_TEMP+0.0f +273.15f,	251,	"Combines with O2 to make WATR", ST_GAS, TYPE_GAS, &update_H2, NULL},
	{"SOAP",	PIXPACK(0xF5F5DC),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	1,	35,		SC_LIQUID,		R_TEMP-2.0f	+273.15f,	29,		"Soap. Creates bubbles.", ST_LIQUID, TYPE_LIQUID|PROP_NEUTPENETRATE|PROP_LIFE_DEC, &update_SOAP, NULL},
	{"BHOL",	PIXPACK(0x202020),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	186,	"Black hole (Requires newtonian gravity)", ST_SOLID, TYPE_SOLID, &update_NBHL, NULL},
	{"WHOL",	PIXPACK(0xFFFFFF),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	186,	"White hole (Requires newtonian gravity)", ST_SOLID, TYPE_SOLID, &update_NWHL, NULL},
	{"MERC",    PIXPACK(0x736B6D),	0.4f,	0.04f * CFDS,	0.94f,	0.80f,	0.0f,	0.3f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	1,	91,		SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Mercury. Volume changes with temperature, Conductive.", ST_LIQUID, TYPE_LIQUID|PROP_CONDUCTS|PROP_NEUTABSORB|PROP_LIFE_DEC, &update_MERC, NULL},
	{"PBCN",	PIXPACK(0x3B1D0A),	0.0f,	0.00f * CFDS,	0.97f,	0.50f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	12,	1,	1,	100,	SC_POWERED,		R_TEMP+0.0f	+273.15f,	251,	"Powered breakable clone", ST_NONE, TYPE_SOLID|PROP_BREAKABLECLONE|PROP_POWERED, NULL, &graphics_PBCN},
	{"GPMP",	PIXPACK(0x0A3B3B),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_POWERED,		0.0f		+273.15f,	0,		"Changes gravity to its temp when activated. (use HEAT/COOL).", ST_NONE, TYPE_SOLID|PROP_POWERED, &update_GPMP, &graphics_GPMP},
	{"CLST",	PIXPACK(0xE4A4A4),	0.7f,	0.02f * CFDS,	0.94f,	0.95f,	0.0f,	0.2f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	2,	2,	1,	1,	55,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	70,		"Clay dust. Produces paste when mixed with water.", ST_SOLID, TYPE_PART, &update_CLST, &graphics_CLST},	
	{"WIRE",    PIXPACK(0xFFCC00),  0.0f,   0.00f * CFDS,   0.00f,  0.00f,  0.0f,   0.0f,   0.00f,  0.000f  * CFDS, 0,  0,      0,  0,  0,  1,  1,  100,    SC_ELEC,        R_TEMP+0.0f +273.15f,   250,    "WireWorld wires.",ST_SOLID,TYPE_SOLID,&update_WIRE, &graphics_WIRE},
	{"GBMB",	PIXPACK(0x1144BB),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	20,	1,	1,	30,		SC_EXPLOSIVE,	R_TEMP-2.0f	+273.15f,	29,		"Sticks to first object it touches then produces strong gravity push.", ST_NONE, TYPE_PART|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC, &update_GBMB, &graphics_GBMB},
	{"FIGH",	PIXPACK(0x000000),	0.5f,	0.00f * CFDS,	0.2f,	1.0f,	0.0f,	0.0f,	0.0f,	0.00f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	50,		SC_SPECIAL,		R_TEMP+14.6f+273.15f,	0,		"Fighter. Tries to kill stickmen.", ST_NONE, 0, &update_FIGH, &graphics_FIGH},
	{"FRAY",	PIXPACK(0x00BBFF),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_FORCE,		20.0f+0.0f +273.15f,	0,		"Force Emitter. Push or pull objects based on temp value, use like ARAY", ST_SOLID, TYPE_SOLID|PROP_LIFE_DEC, &update_FRAY, NULL},
	{"RPEL",	PIXPACK(0x99CC00),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_FORCE,		20.0f+0.0f	+273.15f,	0,	"Repel or attract particles based on temp value.", ST_NONE, TYPE_SOLID, &update_REPL, NULL},
	//Mod elements past this point
	{"BALL",	PIXPACK(0x0010A0),	0.7f,	0.02f * CFDS,	0.96f,	0.80f,	0.00f,	0.1f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	30,	1,	1,	85,		SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	70,		"Moving solid. Acts like a bouncy ball", ST_NONE, TYPE_PART, &update_MOVS, NULL},
	{"ANIM",	PIXPACK(0x505050),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_POWERED,		R_TEMP+0.0f	+273.15f,	0,		"Animated Liquid Crystal", ST_SOLID, TYPE_SOLID, &update_ANIM, NULL},
	{"INDI",	PIXPACK(0xCCCCCC),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	501,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	0,		"Indestructible Insulator", ST_SOLID, TYPE_SOLID|PROP_INDESTRUCTIBLE, NULL, NULL},
	{"OTWR",	PIXPACK(0x604040),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"One Time Wire", ST_SOLID, TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_HOT_GLOW, NULL, NULL},
	{"PPTI",	PIXPACK(0xEB5917),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	-0.005f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_POWERED,		R_TEMP+0.0f	+273.15f,	0,		"Powered Portal IN.  Things go in here, now with channels (same as WIFI)", ST_SOLID, TYPE_SOLID|PROP_POWERED, &update_PRTI, &graphics_PPTI},
	{"PPTO",	PIXPACK(0x0020EB),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.005f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_POWERED,		R_TEMP	+273.15f,   	0,		"Powered Portal OUT.  Things come out here, now with channels (same as WIFI)", ST_SOLID, TYPE_SOLID|PROP_POWERED, &update_PRTO, &graphics_PPTO},
	{"VIRS",	PIXPACK(0xFE11F6),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	100,	0,	0,	20,	1,	1,	31,		SC_LIQUID,		R_TEMP+50.0f +273.15f,	251,	"Virus. Turns everything it touches into virus", ST_LIQUID, TYPE_LIQUID, &update_VIRS, NULL},
	{"VRSS",	PIXPACK(0xD408CD),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	5,		0,	1,	1,	0,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	251,	"Solid Virus. Turns everything it touches into virus", ST_SOLID, TYPE_SOLID, &update_VIRS, NULL},
	{"VRSG",	PIXPACK(0xFE68FE),	1.0f,	0.01f * CFDS,	0.99f,	0.30f,	-0.1f,	0.0f,	0.75f,	0.000f	* CFDS,	0,	500,	0,	0,	0,	0,	1,	1,		SC_GAS,			R_TEMP+500.0f+273.15f,	251,	"Gas Virus. Turns everything it touches into virus", ST_GAS, TYPE_GAS, &update_VIRS, NULL},
	{"CURE",	PIXPACK(0x8BE700),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	1,		0,	0,	20,	1,	1,	32,		SC_LIQUID,		R_TEMP+0.0f +273.15f,	251,	"Cure. Turns virus back into what it was before", ST_LIQUID, TYPE_LIQUID, NULL, NULL},
	{"ACTV",	PIXPACK(0x005254),	0.0f,	0.00f * CFDS,	0.90f,  0.00f,  0.0f,	0.0f,	0.00f,  0.000f  * CFDS, 0,	0,		0,	0,	0,	1,	1,	100,	SC_POWERED,		R_TEMP+0.0f	+273.15f,	251,	"Activator. Can only be sparked when activated", ST_SOLID, TYPE_SOLID|PROP_POWERED, NULL, &graphics_ACTV},
	{"PINV",	PIXPACK(0x00CCCC),	0.0f,	0.00f * CFDS,	1.00f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	15,	1,	1,	100,	SC_POWERED,		R_TEMP+0.0f	+273.15f,	164,	"Invisible to everything when activated.", ST_SOLID, TYPE_SOLID|PROP_NEUTPASS|PROP_POWERED, NULL, &graphics_PINV},
	{"RAZR",	PIXPACK(0xC0C0C0),	0.7f,	0.07f * CFDS,	0.97f,	0.00f,	0.0f,	1.5f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	0,	1,	1,	500,	SC_POWDERS,		R_TEMP+0.0f	+273.15f,	50,		"Heavy silver particles", ST_SOLID, TYPE_PART, NULL, NULL},
	{"COND",	PIXPACK(0xFE7207),	0.0f,	0.00f * CFDS,	0.85f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	0,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Conducts electricity. Customizable with tmp and tmp2", ST_SOLID, TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC, NULL, NULL},
	{"PWHT",	PIXPACK(0x381D7C),	0.0f,	0.00f * CFDS,	0.97f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	0,	1,	1,	100,	SC_POWERED,		R_TEMP+0.0f	+273.15f,	0,		"Powered Heater. Flood fill heats particles to it's temp. Use only one", ST_NONE, TYPE_SOLID|PROP_POWERED, &update_PWHT, &graphics_PWHT},
	//Name		Colour				Advec	Airdrag			Airloss	Loss	Collid	Grav	Diffus	Hotair			Fal	Burn	Exp	Mel	Hrd M	Use	Weight	Section			H						Ins		Description
};

/*
Name: The name of the element.

Colour: Color in hexadecimal code.

Advec: How much the particle is accelerated by moving air.

Airdrag: How much air the particle generates in the direction of travel.

Airloss: How much the particle slows down moving air (although this won't have as big an effect as a wall). 1 = no effect, 0 = maximum effect.

Loss: How much velocity the particle loses each frame. 1 = no loss, .5 = half loss.

Collid: Velocity is multiplied by this when the particle collides with something.

Grav: How fast the particle falls. A negative number means it floats.

Diffus: How much the particle "wiggles" around (think GAS).

Hotair: How much the particle increases the pressure by.

Fal: How does the particle move? 0 = solid, 1 = powder, 2 = liquid

Burn: Does it burn? 0 = no, higher numbers = higher "burnage".

Exp: Does it explode? 0 = no, 1 = when touching fire, 2 = when touching fire or when pressure > 2.5

Mel: Does it melt? 1 = yes, 0 = no.

Hrd: How much does acid affect it? 0 = no effect, higher numbers = higher effect.

M: Does it show up on the menu? 1 = yes, 0 = no.

Weight: Heavier elements sink beneath lighter ones. 1 = Gas. 2 = Light, 98 = Heavy (liquids 0-49, powder 50-99). 100 = Solid. -1 is Neutrons and Photons.

Section: The section of the menu it is in.

H: What temperature does it have when created? Temperature is in Kelvin (Kelvin = degrees C + 273.15). R_TEMP+273.15f gives room temperature.

Ins: specific heat value (how fast it transfers heat to particles touching it), can be found by using the real life heat value in J/G K (or KJ/KG K) by 96.635/RealLifeValue. 0 - no heat transfer, 250 - maximum heat transfer speed.

Description: A short one sentence description of the element, shown when you mouse over it in-game.

State: What state is this element? Options are ST_NONE, ST_SOLID, ST_LIQUID, ST_GAS.

Properties: Does this element have special properties? The properties can be found at ~214. Separate each property with | inside the property variable.

Function:

Graphics function:

*/

// define abbreviations for impossible p/t values
#define IPL -257.0f
#define IPH 257.0f
#define ITL MIN_TEMP-1
#define ITH MAX_TEMP+1
// no transition (PT_NONE means kill part)
#define NT -1
// special transition - lava ctypes etc need extra code, which is only found and run if ST is given
#define ST PT_NUM
part_transition ptransitions[PT_NUM] =
{	//			if low pressure		if high pressure	if low temperature	if high temperature
	// Name		plv		plt	 		phv		pht			tlv		tlt	 		thv		tht
	/* NONE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* DUST */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* WATR */ {IPL,	NT,			IPH,	NT,			273.15f,PT_ICEI,	373.0f,	PT_WTRV},
	/* OIL  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			333.0f,	PT_GAS},
	/* FIRE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			2773.0f,PT_PLSM},
	/* STNE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			983.0f,	PT_LAVA},
	/* LAVA */ {IPL,	NT,			IPH,	NT,			2573.15f,ST,		ITH,	NT}, // 2573.15f is highest melt pt of possible ctypes
	/* GUN  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			673.0f,	PT_FIRE},
	/* NITR */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			673.0f,	PT_FIRE},
	/* CLNE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* GAS  */ {IPL,	NT,			6.0f,	PT_OIL,		ITL,	NT,			573.0f,	PT_FIRE},
	/* C-4  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			673.0f,	PT_FIRE},
	/* GOO  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* ICE  */ {IPL,	NT,			0.8f,	PT_SNOW,	ITL,	NT,			233.0f,	ST},
	/* METL */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1273.0f,PT_LAVA},
	/* SPRK */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* SNOW */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			273.0f,	PT_WATR},
	/* WOOD */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			873.0f,	PT_FIRE},
	/* NEUT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* PLUT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* PLNT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			573.0f,	PT_FIRE},
	/* ACID */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* VOID */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* WTRV */ {IPL,	NT,			IPH,	NT,			371.0f,	ST,			ITH,	NT},
	/* CNCT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1123.0f,PT_LAVA},
	/* DSTW */ {IPL,	NT,			IPH,	NT,			273.15f,PT_ICEI,	373.0f,	PT_WTRV},
	/* SALT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1173.0f,PT_LAVA},
	/* SLTW */ {IPL,	NT,			IPH,	NT,			233.0f,	PT_ICEI,	483.0f,	ST},
	/* DMND */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* BMTL */ {IPL,	NT,			1.0f,	ST,			ITL,	NT,			1273.0f,PT_LAVA},
	/* BRMT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1273.0f,PT_LAVA},
	/* PHOT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* URAN */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* WAX  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			319.0f,	PT_MWAX},
	/* MWAX */ {IPL,	NT,			IPH,	NT,			318.0f,	PT_WAX,		673.0f,	PT_FIRE},
	/* PSCN */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1687.0f,PT_LAVA},
	/* NSCN */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1687.0f,PT_LAVA},
	/* LN2  */ {IPL,	NT,			IPH,	NT,			63.0f,	PT_NICE,	77.0f,	PT_NONE},
	/* INSL */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* VACU */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* VENT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* RBDM */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			312.0f,	PT_LRBD},
	/* LRBD */ {IPL,	NT,			IPH,	NT,			311.0f,	PT_RBDM,	961.0f,	PT_FIRE},
	/* NTCT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1687.0f,PT_LAVA},
	/* SAND */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1973.0f,PT_LAVA},
	/* GLAS */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1973.0f,PT_LAVA},
	/* PTCT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1414.0f,PT_LAVA},
	/* BGLA */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1973.0f,PT_LAVA},
	/* THDR */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* PLSM */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* ETRD */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* NICE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			63.1f,	PT_LNTG},
	/* NBLE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* BTRY */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			2273.0f,PT_PLSM},
	/* LCRY */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1273.0f,PT_BGLA},
	/* STKM */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			620.0f,	PT_FIRE},
	/* SWCH */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* SMKE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			625.0f,	PT_FIRE},
	/* DESL */ {IPL,	NT,			5.0f,	PT_FIRE,	ITL,	NT,			335.0f,	PT_FIRE},
	/* COAL */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* LO2  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			90.1f,	PT_O2},
	/* O2   */ {IPL,	NT,			IPH,	NT,			90.0f,	PT_LO2,		ITH,	NT},
	/* INWR */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1687.0f,PT_LAVA},
	/* YEST */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			373.0f,	PT_DYST},
	/* DYST */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			473.0f,	PT_DUST},
	/* THRM */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* GLOW */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* BRCK */ {IPL,	NT,			8.8f,	PT_STNE,	ITL,	NT,			1223.0f,PT_LAVA},
	/* CFLM */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* FIRW */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* FUSE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* FSEP */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* AMTR */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* BCOL */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* PCLN */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* HSWC */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* IRON */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1687.0f,PT_LAVA},
	/* MORT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* LIFE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* DLAY */ {IPL,    NT,         IPH,    NT,         ITL,    NT,         ITH,	NT},
	/* CO2  */ {IPL,    NT,         IPH,    NT,         194.65f,PT_DRIC,   ITH,	NT},
	/* DRIC */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			195.65f,PT_CO2},
	/* CBNW */ {IPL,	NT,			IPH,	NT,			273.15f,PT_ICEI,	373.0f,	PT_WTRV},
	/* STOR */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* PVOD */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* CONV */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* CAUS */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* LIGH */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* TESC */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* DEST */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* SPNG */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			2730.0f,PT_FIRE},
	/* RIME */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			273.15f,PT_WATR},
	/* FOG  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			373.15f,PT_WTRV},
	/* BCLN */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* LOVE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* DEUT  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* WARP */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* PUMP */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* FWRK */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* PIPE */ {IPL,	NT,			10.0f,	PT_BRMT,	ITL,	NT,			ITH,	NT},
	/* FRZZ */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* FRZW */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			53.0f,	PT_ICEI},
	/* GRAV */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* BIZR */ {IPL,	NT,			IPH,	NT,			100.0f,	PT_BIZRG,	400.0f,	PT_BIZRS},
	/* BIZRG*/ {IPL,	NT,			IPH,	NT,			ITL,	NT,			100.0f,	PT_BIZR},//,	400.0f,	PT_BIZRS},
	/* BIZRS*/ {IPL,	NT,			IPH,	NT,			400.0f,	PT_BIZR,	ITH,	NT},//	100.0f,	PT_BIZRG},
	/* INST */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* ISOZ */ {IPL,	NT,			IPH,	NT,			160.0f,	PT_ISZS,	ITH,	NT},
	/* ISZS */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			300.0f,	PT_ISOZ},
	/* PRTI */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* PRTO */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* PSTE */ {IPL,	NT,			0.5f,	PT_PSTS,	ITL,	NT,			747.0f,	PT_BRCK},
	/* PSTS */ {0.5f,	PT_PSTE,	IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* ANAR */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* VINE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			573.0f,	PT_FIRE},
	/* INVS */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* EQVE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* SPWN2*/ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* SPAWN*/ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* SHLD1*/ {IPL,	NT,			7.0f,	PT_NONE,	ITL,	NT,			ITH,	NT},
	/* SHLD2*/ {IPL,	NT,			15.0f,	PT_NONE,	ITL,	NT,			ITH,	NT},
	/* SHLD3*/ {IPL,	NT,			25.0f,	PT_NONE,	ITL,	NT,			ITH,	NT},
	/* SHLD4*/ {IPL,	NT,			40.0f,	PT_NONE,	ITL,	NT,			ITH,	NT},
	/* LOlZ */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* WIFI */ {IPL,	NT,			15.0f,	PT_BRMT,	ITL,	NT,			ITH,	NT},
	/* FILT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* ARAY */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* BRAY */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* STKM2*/ {IPL,	NT,			IPH,	NT,			ITL,	NT,			620.0f,	PT_FIRE},
	/* BOMB */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* C-5  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* SING */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* QRTZ */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			2573.15f,PT_LAVA},
	/* PQRT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			2573.15f,PT_LAVA},
	/* EMP  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* BREC */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* ELEC */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* ACEL */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* DCEL */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* TNT  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* IGNP */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			673.0f,	PT_FIRE},
	/* BOYL */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* FREE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* VIRS */ {IPL,	NT,			IPH,	NT,			305.0f,	PT_VRSS,	673.0f,PT_VRSG},
	/* VRSS */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			305.0f,PT_VIRS},
	/* VRSG */ {IPL,	NT,			IPH,	NT,			673.0f,	PT_VRSG,	ITH,	NT},
	/* FREE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* WIND */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* H2   */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* SOAP */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITL,	NT},
	/* NBHL */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* NWHL */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* MERC */ {IPL,    NT,         IPH,    NT,         ITL,    NT,         ITH,	NT},
	/* PBCN */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* GPMP */ {IPL,    NT,         IPH,    NT,         ITL,    NT,         ITH,	NT},
	/* CLST */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1256.0f,PT_LAVA},
	/* WIRE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* GBMB */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* FIGH */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			620.0f,	PT_FIRE},
	/* FRAY */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* REPL */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	// Mod elements past this point
	/* BALL */ {25.0f,	PT_NONE,	25.0f,	PT_NONE,	ITL,	NT,			ITH,	NT},
	/* ANIM */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* OTWR */ {IPL,	NT,			10.0f,	PT_BREC,	ITL,	NT,			1273.0f,PT_LAVA},
	/* INDI */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* PPTI */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* PPTO */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* VIRS */ {IPL,	NT,			IPH,	NT,			305.0f,	PT_VRSS,	673.0f,PT_VRSG},
	/* VRSS */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			305.0f,PT_VIRS},
	/* VRSG */ {IPL,	NT,			IPH,	NT,			673.0f,	PT_VRSG,	ITH,	NT},
	/* CURE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* ACTV */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* PINV */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* RAZR */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* COND */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* PWHT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
};

part_type ptypes2[PT_NUM];
part_transition ptransitions2[PT_NUM];
