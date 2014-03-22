
#include <sstream>
#include "Menus.h"
#include "simulation/Simulation.h"
#include "simulation/Tool.h"
#include "simulation/WallNumbers.h"
#include "simulation/ToolNumbers.h"
#include "simulation/GolNumbers.h"
#include "hud.h"

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
	menuSections[5] =  new MenuSection('\xC3', "Explosives", true);
	menuSections[6] =  new MenuSection('\xC5', "Gases", true);
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
	menuSections[19] = new MenuSection('\xE2', "Other", false); //list of elements that are hidden or disabled, not in any menu
}

void ClearMenusections()
{
	for (int i = 0; i < SC_TOTAL; i++)
	{
		menuSections[i]->ClearTools();
		delete menuSections[i];
	}
}

int GetNumMenus()
{
	int total = 0;
	for (int j = 0; j < SC_TOTAL; j++)
		if (menuSections[j]->enabled)
			total++;
	return total;
}

//fills all the menus with Tool*s
void FillMenus()
{
	std::string tempActiveTools[3], decoActiveTools[3];
	//active tools might not have been initialized at the start
	if (activeTools[0])
	{
		for (int i = 0; i < 3; i++)
			tempActiveTools[i] = activeTools[i]->GetIdentifier();
		for (int i = 0; i < 3; i++)
			decoActiveTools[i] = decoTools[i]->GetIdentifier();
	}
	//Clear all menusections
	for (int i = 0; i < SC_TOTAL; i++)
	{
		menuSections[i]->ClearTools();
	}

	//Add all generic elements to menus
	for (int i = 0; i < PT_NUM; i++)
	{
		if (globalSim->elements[i].Enabled && i != PT_LIFE)
		{
			if (globalSim->elements[i].MenuVisible || secret_els)
			{
				menuSections[globalSim->elements[i].MenuSection]->AddTool(new ElementTool(i));
			}
			else
				menuSections[SC_OTHER]->AddTool(new ElementTool(i));
		}
	}

	//Fill up LIFE menu
	for (int i = 0; i < NGOL; i++)
	{
		menuSections[SC_LIFE]->AddTool(new GolTool(i));
	}

	//Fill up wall menu
	for (int i = 0; i < WALLCOUNT; i++)
	{
		if (i == WL_STREAM)
			menuSections[SC_WALL]->AddTool(new StreamlineTool());
		else
			menuSections[SC_WALL]->AddTool(new WallTool(i));
	}

	//Fill up tools menu
	for (int i = 0; i < TOOLCOUNT; i++)
	{
		if (i == TOOL_PROP)
			menuSections[SC_TOOL]->AddTool(new PropTool);
		else
			menuSections[SC_TOOL]->AddTool(new ToolTool(i));
	}

	//Fill up deco menu
	for (int i = 0; i < DECOCOUNT; i++)
	{
		menuSections[SC_DECO]->AddTool(new DecoTool(i));
	}

	//Fill up fav. related menus somehow ...
	menuSections[SC_FAV]->AddTool(new Tool(INVALID_TOOL, FAV_MORE, "DEFAULT_FAV_MORE"));
	for (int i = 0; i < 18; i++)
	{
		menuSections[SC_FAV]->AddTool(new Tool(INVALID_TOOL, FAV_MORE-1, "DEFAULT_FAV_FAKE"));
	}
	for (int i = FAV_START+1; i < FAV_END; i++)
	{
		menuSections[SC_FAV2]->AddTool(new Tool(INVALID_TOOL, i, "DEFAULT_FAV_" + std::string(fav[i-FAV_START].name)));
	}
	for (int i = HUD_START; i < HUD_START+HUD_NUM; i++)
	{
		menuSections[SC_HUD]->AddTool(new Tool(INVALID_TOOL, i, "DEFAULT_FAV_" + std::string(hud_menu[i-HUD_START].name)));
	}

	//restore active tools
	if (activeTools[0])
	{
		for (int i = 0; i < 3; i++)
		{
			Tool* temp = GetToolFromIdentifier(tempActiveTools[i]);
			if (!temp)
				temp = GetToolFromIdentifier("DEFAULT_PT_NONE");
			activeTools[i] = temp;
		}
		for (int i = 0; i < 3; i++)
		{
			Tool* temp = GetToolFromIdentifier(decoActiveTools[i]);
			if (!temp)
				temp = GetToolFromIdentifier("DEFAULT_PT_NONE");
			decoTools[i] = temp;
		}
	}
}
