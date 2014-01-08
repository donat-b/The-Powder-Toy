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

#include "powder.h"
#include "gravity.h"
#include "misc.h"
#include "simulation/Element.h"
#include "simulation/ElementDataContainer.h"
#include "simulation/Simulation.h"
#include <cmath>

// Declare the element initialisation functions
#define ElementNumbers_Include_Decl
#define DEFINE_ELEMENT(name, id) void name ## _init_element(ELEMENT_INIT_FUNC_ARGS);
#include "simulation/ElementNumbers.h"
#include "simulation/WallNumbers.h"


Simulation *globalSim = NULL; // TODO: remove this global variable

Simulation::Simulation() :
	pfree(-1)
{
	memset(elementData, 0, sizeof(elementData));
	Clear();
}

Simulation::~Simulation()
{
	int t;
	for (t=0; t<PT_NUM; t++)
	{
		if (elementData[t])
		{
			delete elementData[t];
			elementData[t] = NULL;
		}
	}
}

void Simulation::InitElements()
{
	#define DEFINE_ELEMENT(name, id) if (id>=0 && id<PT_NUM) { name ## _init_element(this, &elements[id], id); };
	#define ElementNumbers_Include_Call
	#include "simulation/ElementNumbers.h"

	Simulation_Compat_CopyData(this);
}

void Simulation::Clear()
{
	int t;
	for (t=0; t<PT_NUM; t++)
	{
		if (elementData[t])
		{
			elementData[t]->Simulation_Cleared(this);
		}
	}
	memset(elementCount, 0, sizeof(elementCount));
	pfree = 0;
}

void Simulation::recountElements()
{
	memset(elementCount, 0, sizeof(elementCount));
	for (int i = 0; i < NPART; i++)
		if (parts[i].type)
			elementCount[parts[i].type]++;
}

// the function for creating a particle
// p=-1 for normal creation (checks whether the particle is allowed to be in that location first)
// p=-3 to create without checking whether the particle is allowed to be in that location
// or p = a particle number, to replace a particle
int Simulation::part_create(int p, int x, int y, int t)
{
	// This function is only for actually creating particles.
	// Not for tools, or changing things into spark, or special brush things like setting clone ctype.

	int i, oldType = PT_NONE;
	if (x<0 || y<0 || x>=XRES || y>=YRES || t<=0 || t>=PT_NUM || !elements[t].Enabled)
	{
		return -1;
	}
	// If the element has a Func_Create_Override, use that instead of the rest of this function
	if (elements[t].Func_Create_Override)
	{
		int ret = (*(elements[t].Func_Create_Override))(this, p, x, y, t);

		// returning -4 means continue with part_create as though there was no override function
		// Useful if creation should be blocked in some conditions, but otherwise no changes to this function (e.g. SPWN)
		if (ret!=-4)
			return ret;
	}

	if (elements[t].Func_Create_Allowed)
	{
		if (!(*(elements[t].Func_Create_Allowed))(this, p, x, y, t))
			return -1;
	}

	if (p==-1)
	{
		// Check whether the particle can be created here

		// If there is a particle, only allow creation if the new particle can occupy the same space as the existing particle
		// If there isn't a particle but there is a wall, check whether the new particle is allowed to be in it
		//   (not "!=2" for wall check because eval_move returns 1 for moving into empty space)
		// If there's no particle and no wall, assume creation is allowed
		if (pmap[y][x] ? (eval_move(t, x, y, NULL)!=2) : (bmap[y/CELL][x/CELL] && eval_move(t, x, y, NULL)==0))
		{
			return -1;
		}
		i = part_alloc();
	}
	else if (p==-3) // skip pmap checks, e.g. for sing explosion
	{
		i = part_alloc();
	}
	else if (p>=0) // Replace existing particle
	{
		int oldX = (int)(parts[p].x+0.5f);
		int oldY = (int)(parts[p].y+0.5f);
		oldType = parts[p].type;
		if (elements[oldType].Func_ChangeType)
		{
			(*(elements[oldType].Func_ChangeType))(this, p, oldX, oldY, oldType, t);
		}
		if (oldType) elementCount[oldType]--;
		delete_part_info(p);
		pmap_remove(p, oldX, oldY);
		i = p;
	}
	else // Dunno, act like it was p=-3
	{
		i = part_alloc();
	}

	// Check whether a particle was successfully allocated
	if (i<0)
		return -1;

	// set the pmap/photon maps to the newly created particle
	pmap_add(i, x, y, t);// TODO: ? only set pmap if (t!=PT_STKM && t!=PT_STKM2 && t!=PT_FIGH)

	// Set some properties
	parts[i] = elements[t].DefaultProperties;
	parts[i].type = t;
	parts[i].x = (float)x;
	parts[i].y = (float)y;
#ifdef OGLR
	parts[i].lastX = (float)x;
	parts[i].lastY = (float)y;
#endif
	if (ptypes[t].properties & PROP_POWERED)
	{
		parts[i].flags |= FLAG_INSTACTV;
	}

	// Fancy dust effects for powder types
	if ((elements[t].Properties & TYPE_PART) && pretty_powder)
	{
		int colr, colg, colb;
		colr = COLR(elements[t].Colour)+sandcolor*1.3+(rand()%40)-20+(rand()%30)-15;
		colg = COLG(elements[t].Colour)+sandcolor*1.3+(rand()%40)-20+(rand()%30)-15;
		colb = COLB(elements[t].Colour)+sandcolor*1.3+(rand()%40)-20+(rand()%30)-15;
		colr = colr>255 ? 255 : (colr<0 ? 0 : colr);
		colg = colg>255 ? 255 : (colg<0 ? 0 : colg);
		colb = colb>255 ? 255 : (colb<0 ? 0 : colb);
		parts[i].dcolour = COLARGB(rand()%150, colr, colg, colb);
	}

	// Set non-static properties (such as randomly generated ones)
	if (elements[t].Func_Create)
	{
		(*(elements[t].Func_Create))(this, i, x, y, t);
	}

	//default moving solid values, not part of any ball and random "bounciness"
	if (ptypes[t].properties & PROP_MOVS)
	{
		parts[i].tmp2 = 255;
		parts[i].pavg[0] = (float)(rand()%20);
		parts[i].pavg[1] = (float)(rand()%20);
	}

	pmap_add(i, x, y, t);

	if (elements[t].Func_ChangeType)
	{
		(*(elements[t].Func_ChangeType))(this, i, x, y, oldType, t);
	}

	elementCount[t]++;
	return i;
}

bool Simulation::part_change_type(int i, int x, int y, int t)//changes the type of particle number i, to t.  This also changes pmap at the same time.
{
	if (x<0 || y<0 || x>=XRES || y>=YRES || i>=NPART || t<0 || t>=PT_NUM)
		return false;

	if (t==parts[i].type)
		return true;
	if (ptypes[parts[i].type].properties&PROP_INDESTRUCTIBLE)
		return false;
	if (elements[t].Func_Create_Allowed)
	{
		if (!(*(elements[t].Func_Create_Allowed))(this, i, x, y, t))
			return false;
	}


	int oldType = parts[i].type;
	if (oldType) elementCount[oldType]--;

	if (!ptypes[t].enabled)
		t = PT_NONE;
	delete_part_info(i);

	parts[i].type = t;
	pmap_remove(i, x, y);
	pmap_add(i, x, y, t);
	if (t) elementCount[t]++;
	if (elements[oldType].Func_ChangeType)
	{
		(*(elements[oldType].Func_ChangeType))(this, i, x, y, oldType, t);
	}
	if (elements[t].Func_ChangeType)
	{
		(*(elements[t].Func_ChangeType))(this, i, x, y, oldType, t);
	}
	return true;
}

void Simulation::part_kill(int i)//kills particle number i
{
	int x, y;
	int t = parts[i].type;

	x = (int)(parts[i].x+0.5f);
	y = (int)(parts[i].y+0.5f);

	if (t && elements[t].Func_ChangeType)
	{
		(*(elements[t].Func_ChangeType))(this, i, x, y, t, PT_NONE);
	}
	delete_part_info(i);

	pmap_remove(i, x, y);
	if (t == PT_NONE) // TODO: remove this? (//This shouldn't happen anymore, but it's here just in case)
		return;
	if (t) elementCount[t]--;
	part_free(i);
}

//delete some extra info that isn't specific to just one particle (TODO: maybe remove tpt.moving_solid()?)
void Simulation::delete_part_info(int i)
{
	if (ptypes[parts[i].type].properties&PROP_MOVS)
	{
		int bn = parts[i].tmp2;
		if (bn >= 0 && bn < 256)
		{
			msnum[bn]--;
			if (msindex[bn]-1 == i)
				msindex[bn] = 0;
		}
	}
}

/* spark_conductive turns a particle into SPRK and sets ctype, life, and temperature.
 * spark_all does something similar, but behaves correctly for WIRE and INST
 *
 * spark_conductive_attempt and spark_all_attempt do the same thing, except they check whether the particle can actually be sparked (is conductive, has life of zero) first. Remember to check for INSL though. 
 * They return true if the particle was successfully sparked.
*/

void Simulation::spark_all(int i, int x, int y)
{
	if (parts[i].type==PT_WIRE)
		parts[i].ctype = PT_DUST;
	else if (parts[i].type==PT_INST)
		INST_flood_spark(this, x, y);
	else
		spark_conductive(i, x, y);
}
void Simulation::spark_conductive(int i, int x, int y)
{
	int type = parts[i].type;
	part_change_type(i, x, y, PT_SPRK);
	parts[i].ctype = type;
	if (type==PT_WATR)
		parts[i].life = 6;
	else if (type==PT_SLTW)
		parts[i].life = 5;
	else
		parts[i].life = 4;
	if (parts[i].temp < 673.0f && !legacy_enable && (type==PT_METL || type == PT_BMTL || type == PT_BRMT || type == PT_PSCN || type == PT_NSCN || type == PT_ETRD || type == PT_NBLE || type == PT_IRON))
	{
		parts[i].temp = parts[i].temp+10.0f;
		if (parts[i].temp > 673.0f)
			parts[i].temp = 673.0f;
	}
}
bool Simulation::spark_all_attempt(int i, int x, int y)
{
	if ((parts[i].type==PT_WIRE && parts[i].ctype<=0) || (parts[i].type==PT_INST && parts[i].life<=0))
	{
		spark_all(i, x, y);
		return true;
	}
	else if (!parts[i].life && (elements[parts[i].type].Properties & PROP_CONDUCTS))
	{
		spark_conductive(i, x, y);
		return true;
	}
	return false;
}
bool Simulation::spark_conductive_attempt(int i, int x, int y)
{
	if (!parts[i].life && (elements[parts[i].type].Properties & PROP_CONDUCTS))
	{
		spark_conductive(i, x, y);
		return true;
	}
	return false;
}

//Functions for creating parts, walls, tools, and deco
void Simulation::CreateWall(int x, int y, int wall)
{
	if (x < 0 || y < 0 || x >= XRES/CELL || y >= YRES/CELL)
		return;

	//reset fan velocity, as if new fan walls had been created
	if (wall == WL_FAN)
	{
		fvx[y][x] = 0.0f;
		fvy[y][x] = 0.0f;
	}
	else if (wall == WL_STREAM)
	{
		//streamlines can't be drawn next to each other
		for (int tempY = y-1; tempY <= y+1; tempY++)
			for (int tempX = x-1; tempX <= x+1; tempX++)
			{
				if (tempX >= 0 && tempX < XRES/CELL && tempY >= 0 && tempY < YRES/CELL && bmap[tempY][tempX] == WL_STREAM)
					return;
			}
	}
	else if (wall == WL_ERASEALL)
	{
		for (int i = 0; i < CELL; i++)
			for (int j = 0; j < CELL; j++)
			{
				delete_part(x*CELL+i, y*CELL+j, 0);
			}
		for (int i = 0; i < MAXSIGNS; i++)
			if (signs[i].text[0])
			{
				if (signs[i].x >= x*CELL && signs[i].y >= y*CELL && signs[i].x <= (x+1)*CELL && signs[i].y <= (y+1)*CELL)
					signs[i].text[0] = 0;
			}
		wall = 0;
	}
	if (wall == WL_GRAV || bmap[y][x] == WL_GRAV)
		gravwl_timeout = 60;
	bmap[y][x] = wall;
}

void Simulation::CreateWallLine(int x1, int y1, int x2, int y2, int rx, int ry, int wall)
{
	int x, y, dx, dy, sy;
	bool reverseXY = abs(y2-y1) > abs(x2-x1);
	float e = 0.0f, de;
	if (reverseXY)
	{
		y = x1;
		x1 = y1;
		y1 = y;
		y = x2;
		x2 = y2;
		y2 = y;
	}
	if (x1 > x2)
	{
		y = x1;
		x1 = x2;
		x2 = y;
		y = y1;
		y1 = y2;
		y2 = y;
	}
	dx = x2 - x1;
	dy = abs(y2 - y1);
	if (dx)
		de = dy/(float)dx;
	else
		de = 0.0f;
	y = y1;
	sy = (y1<y2) ? 1 : -1;
	for (x=x1; x<=x2; x++)
	{
		if (reverseXY)
			CreateWallBox(y, x, y+ry, x+rx, wall);
		else
			CreateWallBox(x, y, x+rx, y+ry, wall);
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			if (!(rx+ry) && ((y1<y2) ? (y<=y2) : (y>=y2)))
			{
				if (reverseXY)
					CreateWallBox(y, x, y+ry, x+rx, wall);
				else
					CreateWallBox(x, y, x+rx, y+ry, wall);
			}
			e -= 1.0f;
		}
	}
}

void Simulation::CreateWallBox(int x1, int y1, int x2, int y2, int wall)
{
	if (x1 > x2)
	{
		int temp = x2;
		x2 = x1;
		x1 = temp;
	}
	if (y1 > y2)
	{
		int temp = y2;
		y2 = y1;
		y1 = temp;
	}
	for (int i = x1; i <= x2; i++)
		for (int j = y1; j <= y2; j++)
			CreateWall(i, j, wall);
}

int Simulation::FloodWalls(int x, int y, int wall, int replace)
{
	int x1, x2;
	if (wall == SPC_PROP)
		return flood_prop(x*CELL, y*CELL, prop_offset, prop_value, prop_format);
	if (replace == -1)
	{
		if (wall==WL_ERASE || wall==WL_ERASEALL)
		{
			replace = bmap[y][x];
			if (!replace)
				return 0;
		}
		else
			replace = 0;
	}

	if (bmap[y][x] != replace)
		return 1;

	// go left as far as possible
	x1 = x2 = x;
	while (x1 >= 1)
	{
		if (bmap[y][x1-1] != replace)
		{
			break;
		}
		x1--;
	}
	while (x2 < XRES/CELL-1)
	{
		if (bmap[y][x2+1] != replace)
		{
			break;
		}
		x2++;
	}

	// fill span
	for (x=x1; x<=x2; x++)
	{
		CreateWall(x, y, wall);
	}
	// fill children
	if (y >= 1)
		for (x=x1; x<=x2; x++)
			if (bmap[y-1][x] == replace)
				if (!FloodWalls(x, y-1, wall, replace))
					return 0;
	if (y < YRES/CELL-1)
		for (x=x1; x<=x2; x++)
			if (bmap[y+1][x] == replace)
				if (!FloodWalls(x, y+1, wall, replace))
					return 0;
	return 1;
}

//This is not part of the simulation class
void Simulation_Compat_CopyData(Simulation* sim)
{
	// TODO: this can be removed once all the code uses Simulation instead of global variables
	parts = sim->parts;


	for (int t=0; t<PT_NUM; t++)
	{
		ptypes[t].name = mystrdup(sim->elements[t].Name.c_str());
		ptypes[t].pcolors = PIXRGB(COLR(sim->elements[t].Colour), COLG(sim->elements[t].Colour), COLB(sim->elements[t].Colour));
		ptypes[t].menu = sim->elements[t].MenuVisible;
		ptypes[t].menusection = sim->elements[t].MenuSection;
		ptypes[t].enabled = sim->elements[t].Enabled;
		ptypes[t].advection = sim->elements[t].Advection;
		ptypes[t].airdrag = sim->elements[t].AirDrag;
		ptypes[t].airloss = sim->elements[t].AirLoss;
		ptypes[t].loss = sim->elements[t].Loss;
		ptypes[t].collision = sim->elements[t].Collision;
		ptypes[t].gravity = sim->elements[t].Gravity;
		ptypes[t].diffusion = sim->elements[t].Diffusion;
		ptypes[t].hotair = sim->elements[t].PressureAdd_NoAmbHeat;
		ptypes[t].falldown = sim->elements[t].Falldown;
		ptypes[t].flammable = sim->elements[t].Flammable;
		ptypes[t].explosive = sim->elements[t].Explosive;
		ptypes[t].meltable = sim->elements[t].Meltable;
		ptypes[t].hardness = sim->elements[t].Hardness;
		ptypes[t].weight = sim->elements[t].Weight;
		ptypes[t].heat = sim->elements[t].DefaultProperties.temp;
		ptypes[t].hconduct = sim->elements[t].HeatConduct;
		ptypes[t].descs = mystrdup(sim->elements[t].Description.c_str());
		ptypes[t].state = sim->elements[t].State;
		ptypes[t].properties = sim->elements[t].Properties;
		ptypes[t].update_func = sim->elements[t].Update;
		ptypes[t].graphics_func = sim->elements[t].Graphics;

		ptransitions[t].plt = sim->elements[t].LowPressureTransitionElement;
		ptransitions[t].plv = sim->elements[t].LowPressureTransitionThreshold;
		ptransitions[t].pht = sim->elements[t].HighPressureTransitionElement;
		ptransitions[t].phv = sim->elements[t].HighPressureTransitionThreshold;
		ptransitions[t].tlt = sim->elements[t].LowTemperatureTransitionElement;
		ptransitions[t].tlv = sim->elements[t].LowTemperatureTransitionThreshold;
		ptransitions[t].tht = sim->elements[t].HighTemperatureTransitionElement;
		ptransitions[t].thv = sim->elements[t].HighTemperatureTransitionThreshold;

		platent[t] = sim->elements[t].Latent;
	}
}
