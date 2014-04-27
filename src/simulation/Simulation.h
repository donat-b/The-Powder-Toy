/*
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

#ifndef Simulation_h
#define Simulation_h

#include "simulation/Element.h"
#include "powder.h"

// Defines for element transitions
#define IPL -257.0f
#define IPH 257.0f
#define ITL MIN_TEMP-1
#define ITH MAX_TEMP+1
// no transition (PT_NONE means kill part)
#define NT -1
// special transition - lava ctypes etc need extra code, which is only found and run if ST is given
#define ST PT_NUM

class ElementDataContainer;
class Brush;

class Simulation
{
public:
	particle parts[NPART];
	int elementCount[PT_NUM];
	Element elements[PT_NUM];
	ElementDataContainer *elementData[PT_NUM];
	int pfree;
	int parts_lastActiveIndex;
	


	Simulation();
	~Simulation();
	void InitElements();
	void InitElement(char* name, int id);
	void Clear();
	void recountElements();
	int part_create(int p, int x, int y, int t, int v = -1);
	void part_kill(int i);
	bool part_change_type(int i, int x, int y, int t);
	void part_change_type_force(int i, int t);
	void delete_part_info(int i);

	void RecalcFreeParticles();
	void Update();
	bool UpdateParticle(int i);

	void spark_all(int i, int x, int y);
	bool spark_all_attempt(int i, int x, int y);
	void spark_conductive(int i, int x, int y);
	bool spark_conductive_attempt(int i, int x, int y);

	int lightning_recreate; //timer for when LIGH can be created again
	int CreateParts(int x, int y, int c, int flags, bool fill, Brush* brush = NULL);
	int CreatePartFlags(int x, int y, int c, int flags);
	void CreateLine(int x1, int y1, int x2, int y2, int c, int flags, Brush* brush = NULL);
	void CreateBox(int x1, int y1, int x2, int y2, int c, int flags);
	int FloodFillPmapCheck(int x, int y, int type);
	int FloodParts(int x, int y, int fullc, int replace, int flags);

	void CreateWall(int x, int y, int wall);
	void CreateWallLine(int x1, int y1, int x2, int y2, int rx, int ry, int wall);
	void CreateWallBox(int x1, int y1, int x2, int y2, int wall);
	int FloodWalls(int x, int y, int wall, int replace);

	int CreateTool(int x, int y, int tool, float strength);
	void CreateToolBrush(int x, int y, int tool, float strength, Brush* brush);
	void CreateToolLine(int x1, int y1, int x2, int y2, int tool, float strength, Brush* brush);
	void CreateToolBox(int x1, int y1, int x2, int y2, int tool, float strength);

	int CreateProp(int x, int y, PropertyType propType, PropertyValue propValue, size_t propOffset);
	void CreatePropBrush(int x, int y, PropertyType propType, PropertyValue propValue, size_t propOffset, Brush* brush);
	void CreatePropLine(int x1, int y1, int x2, int y2, PropertyType propType, PropertyValue propValue, size_t propOffset, Brush* brush);
	void CreatePropBox(int x1, int y1, int x2, int y2, PropertyType propType, PropertyValue propValue, size_t propOffset);
	int FloodPropHelper(int x, int y, int partType, PropertyType propType, PropertyValue propValue, size_t propOffset, char * bitmap);
	int FloodProp(int x, int y, PropertyType propType, PropertyValue propValue, size_t propOffset);

	void CreateDeco(int x, int y, int tool, unsigned int color);
	void CreateDecoBrush(int x, int y, int tool, unsigned int color, Brush* brush);
	void CreateDecoLine(int x1, int y1, int x2, int y2, int tool, unsigned int color, Brush* brush);
	void CreateDecoBox(int x1, int y1, int x2, int y2, int tool, unsigned int color);
	void FloodDeco(int x, int y, unsigned int color, unsigned int replace);

	// Functions defined here should hopefully be inlined
	// Don't put anything that will change often here, since changes cause a lot of recompiling

	bool IsElement(int t) const
	{
		return (t>=0 && t<PT_NUM && elements[t].Enabled);
	}
	bool InBounds(int x, int y)
	{
		return (x>=0 && y>=0 && x<XRES && y<YRES);
	}

	// Most of the time, part_alloc and part_free should not be used directly unless you really know what you're doing. 
	// Use part_create and part_kill instead.
	int part_alloc()
	{
		if (pfree == -1)
			return -1;
		int i = pfree;
		pfree = parts[i].life;
		if (i>parts_lastActiveIndex)
			parts_lastActiveIndex = i;
		return i;
	}
	void part_free(int i)
	{
		parts[i].type = 0;
		parts[i].life = pfree;
		pfree = i;
	}
	void pmap_add(int i, int x, int y, int t)
	{
		// NB: all arguments are assumed to be within bounds
		if (elements[t].Properties & TYPE_ENERGY)
			photons[y][x] = t|(i<<8);
		else if ((!pmap[y][x] || (t!=PT_INVIS && t!= PT_FILT)))// && (pmap[y][x]&0xFF) != PT_PINV)
			pmap[y][x] = t|(i<<8);
	}
	void pmap_remove(int i, int x, int y)
	{
		// NB: all arguments are assumed to be within bounds
		if ((pmap[y][x]>>8)==i)
			pmap[y][x] = 0;
		else if ((photons[y][x]>>8)==i)
			photons[y][x] = 0;
	}
};

void Simulation_Compat_CopyData(Simulation *sim);

extern Simulation *globalSim; // TODO: remove this

#endif
