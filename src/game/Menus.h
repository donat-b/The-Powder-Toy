#ifndef MENUS_H
#define MENUS_H

#define SC_WALL 0
#define SC_ELEC 1
#define SC_POWERED 2
#define SC_SENSOR 3
#define SC_FORCE 4
#define SC_EXPLOSIVE 5
#define SC_GAS 6
#define SC_LIQUID 7
#define SC_POWDERS 8
#define SC_SOLIDS 9
#define SC_NUCLEAR 10
#define SC_SPECIAL 11
#define SC_LIFE 12
#define SC_TOOL 13
#define SC_FAV 14
#define SC_DECO 15
#define SC_CRACKER 16
#define SC_FAV2 17
#define SC_HUD 18
#define SC_OTHER 19
#define SC_TOTAL 20

#include <vector>

class Tool;
class MenuSection
{
public:
	unsigned char icon;
	std::string name;
	std::vector<Tool*> tools;
	bool enabled;

	MenuSection(char icon_, std::string name_, bool enabled_):
		icon(icon_),
		name(name_),
		tools(std::vector<Tool*>()),
		enabled(enabled_)
	{
	}

	void AddTool(Tool* tool)
	{
		tools.push_back(tool);
	}

	void ClearTools();
};

extern MenuSection* menuSections[SC_TOTAL];

void InitMenusections();
void ClearMenusections();
int GetNumMenus();

#endif