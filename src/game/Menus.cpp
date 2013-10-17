
#include "Menus.h"
#include "simulation/Tool.h"

void MenuSection::ClearTools()
{
	for (std::vector<Tool*>::iterator iter = tools.begin(), end = tools.end(); iter != end; ++iter)
		delete *iter;
	tools.clear();
}

MenuSection* menuSections[SC_TOTAL];

void InitMenusections()
{
	menuSections[0] =  new MenuSection('\xC1', "Walls", true);
	menuSections[1] =  new MenuSection('\xC2', "Electronics", true);
	menuSections[2] =  new MenuSection('\xD6', "Powered Materials", true);
	menuSections[3] =  new MenuSection('\x99', "Sensors", true);
	menuSections[4] =  new MenuSection('\xE3', "Force Creating", true);
	menuSections[5] =  new MenuSection('\xC5', "Explosives", true);
	menuSections[6] =  new MenuSection('\xC3', "Gases", true);
	menuSections[7] =  new MenuSection('\xC4', "Liquids", true);
	menuSections[8] =  new MenuSection('\xD0', "Powders", true);
	menuSections[9] =  new MenuSection('\xD1', "Solids", true);
	menuSections[10] = new MenuSection('\xC6', "Radioactive", true);
	menuSections[11] = new MenuSection('\xCC', "Special", true);
	menuSections[12] = new MenuSection('\xD2', "Game of Life", true);
	menuSections[13] = new MenuSection('\xD7', "Tools", true);
	menuSections[14] = new MenuSection('\xE2', "\brF\bla\bov\bgo\btr\bbi\bpt\bwe", true);
	menuSections[15] = new MenuSection('\xE5', "Deco", true);
	menuSections[16] = new MenuSection('\xC8', "Cracker!", false);
	menuSections[17] = new MenuSection('\xE2', "Favorite2", false);
	menuSections[18] = new MenuSection('\xE2', "HUD", false);
	menuSections[19] = new MenuSection('\xE2', "Other", false);
}

int GetNumMenus()
{
	int total = 0;
	for (int j = 0; j < SC_TOTAL; j++)
		if (menuSections[j]->enabled)
			total++;
	return total;
}
