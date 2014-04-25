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

#include <cmath>
#include "powder.h"
#include "gravity.h"
#include "misc.h"
#include "interface.h" //for framenum, try and remove this later
#include "game/Brush.h"

//Simulation stuff
#include "Element.h"
#include "ElementDataContainer.h"
#include "Tool.h"
#include "Simulation.h"
#include "CoordStack.h"

// Declare the element initialisation functions
#define ElementNumbers_Include_Decl
#define DEFINE_ELEMENT(name, id) void name ## _init_element(ELEMENT_INIT_FUNC_ARGS);
#include "ElementNumbers.h"
#include "WallNumbers.h"
#include "ToolNumbers.h"


Simulation *globalSim = NULL; // TODO: remove this global variable

Simulation::Simulation() :
	pfree(-1),
	lightning_recreate(0)
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
int Simulation::part_create(int p, int x, int y, int t, int v)
{
	// This function is only for actually creating particles.
	// Not for tools, or changing things into spark, or special brush things like setting clone ctype.

	int i, oldType = PT_NONE;
	if (x<0 || y<0 || x>=XRES || y>=YRES || t<=0 || t>=PT_NUM || !elements[t].Enabled)
	{
		return -1;
	}

	// Spark Checks here
	if (p == -2 && (pmap[y][x]&0xFF) == PT_BUTN && parts[pmap[y][x]>>8].life == 10)
	{
		spark_conductive(pmap[y][x]>>8, x, y);
		return pmap[y][x]>>8;
	}
	if (t==PT_SPRK)
	{
		int type = pmap[y][x]&0xFF;
		int index = pmap[y][x]>>8;
		if(type == PT_WIRE)
		{
			parts[index].ctype = PT_DUST;
			return index;
		}
		if (p==-2 && ((elements[type].Properties & PROP_DRAWONCTYPE) || type==PT_CRAY))
		{
			parts[index].ctype = PT_SPRK;
			return index;
		}
		if (!(type == PT_INST || (elements[type].Properties&PROP_CONDUCTS)))
			return -1;
		if (parts[index].life!=0)
			return -1;
		if (p == -2 && type == PT_INST)
		{
			INST_flood_spark(this, x, y);
			return index;
		}

		spark_conductive_attempt(index, x, y);
		return index;
	}
	// End Spark checks

	//Brush Creation
	if (p == -2)
	{
		if (pmap[y][x])
		{
			int drawOn = pmap[y][x]&0xFF;
			//If an element has the PROP_DRAWONCTYPE property, and the element being drawn to it does not have PROP_NOCTYPEDRAW (Also some special cases), set the element's ctype
			if (((ptypes[drawOn].properties & PROP_DRAWONCTYPE) ||
				 (drawOn == PT_STOR && !(ptypes[t].properties & TYPE_SOLID)) ||
				 (drawOn == PT_PCLN && t != PT_PSCN && t != PT_NSCN) ||
				 (drawOn == PT_PBCN && t != PT_PSCN && t != PT_NSCN))
				&& (!(ptypes[t].properties & PROP_NOCTYPEDRAW)))
			{
				parts[pmap[y][x]>>8].ctype = t;
				if (t == PT_LIFE && v < NGOL && drawOn != PT_STOR)
					parts[pmap[y][x]>>8].tmp = v;
			}
			else if ((drawOn == PT_DTEC || (drawOn == PT_PSTN && t != PT_FRME)) && drawOn != t)
			{
				parts[pmap[y][x]>>8].ctype = t;
				if (drawOn == PT_DTEC && t==PT_LIFE && v<NGOL)
					parts[pmap[y][x]>>8].tmp = v;
			}
			return -1;
		}
		else if (IsWallBlocking(x, y, t))
			return -1;
		if (photons[y][x] && (ptypes[t].properties & TYPE_ENERGY))
			return -1;
	}
	// End Brush Creation

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

	if (p == -1)
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
	else if (p == -3) // skip pmap checks, e.g. for sing explosion
	{
		i = part_alloc();
	}
	else if (p >= 0) // Replace existing particle
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
		colr = (int)(COLR(elements[t].Colour)+sandcolor*1.3f+(rand()%40)-20+(rand()%30)-15);
		colg = (int)(COLG(elements[t].Colour)+sandcolor*1.3f+(rand()%40)-20+(rand()%30)-15);
		colb = (int)(COLB(elements[t].Colour)+sandcolor*1.3f+(rand()%40)-20+(rand()%30)-15);
		colr = colr>255 ? 255 : (colr<0 ? 0 : colr);
		colg = colg>255 ? 255 : (colg<0 ? 0 : colg);
		colb = colb>255 ? 255 : (colb<0 ? 0 : colb);
		parts[i].dcolour = COLARGB(rand()%150, colr, colg, colb);
	}

	// Set non-static properties (such as randomly generated ones)
	if (elements[t].Func_Create)
	{
		(*(elements[t].Func_Create))(this, i, x, y, t, v);
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
	if (t)
	{
		pmap_add(i, x, y, t);
		elementCount[t]++;
	}
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

//Used by lua to change type and delete any particle specific info, and also keep pmap / elementCount up to date
void Simulation::part_change_type_force(int i, int t)
{
	int x = (int)(parts[i].x), y = (int)(parts[i].y);
	if (t<0 || t>=PT_NUM)
		return;
	/*if (elements[t].Func_Create_Allowed)
	{
		if (!(*(elements[t].Func_Create_Allowed))(this, i, x, y, t))
		{
			part_kill(i);
			return;
		}
	}*/

	int oldType = parts[i].type;
	if (oldType) elementCount[oldType]--;
	delete_part_info(i);
	parts[i].type = t;
	pmap_remove(i, x, y);
	if (t)
	{
		pmap_add(i, x, y, t);
		elementCount[t]++;
	}

	if (elements[oldType].Func_ChangeType)
	{
		(*(elements[oldType].Func_ChangeType))(this, i, x, y, oldType, t);
	}
	if (elements[t].Func_ChangeType)
	{
		(*(elements[t].Func_ChangeType))(this, i, x, y, oldType, t);
	}
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

	if (x>=0 && y>=0 && x<XRES && y<YRES)
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
	//else if (parts[i].type==PT_INST)
	//	INST_flood_spark(this, x, y);
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

/*
 *
 *Functions for creating parts, walls, tools, and deco
 *
 */

int Simulation::CreateParts(int x, int y, int c, int flags, bool fill, Brush* brush)
{
	int f = 0, rx = 0, ry = 0, shape = CIRCLE_BRUSH;
	if (brush)
	{
		rx = brush->GetRadius().X, ry = brush->GetRadius().Y, shape = brush->GetShape();
	}

	if (c == PT_LIGH)
	{
		if (globalSim->lightning_recreate > 0)
			return 0;
		int newlife = rx + ry;
		if (newlife > 55)
			newlife = 55;
		c = c|newlife<<8;
		globalSim->lightning_recreate = newlife/4;
		rx = ry = 0;
	}
	else if (c == PT_STKM || c == PT_STKM2 || c == PT_FIGH)
		rx = ry = 0;
	else if (c == PT_TESC)
	{
		int newtmp = (rx*4+ry*4+7);
		if (newtmp > 300)
			newtmp = 300;
		c = c|newtmp<<8;
	}

	if (rx<=0) //workaround for rx == 0 crashing. todo: find a better fix later.
	{
		for (int j = y - ry; j <= y + ry; j++)
			if (CreatePartFlags(x, j, c, flags))
				f = 1;
	}
	else
	{
		int tempy = y, i, j, jmax, oldy;
		// tempy is the smallest y value that is still inside the brush
		// jmax is the largest y value that is still inside the brush

		//For triangle brush, start at the very bottom
		if (brush->GetShape() == TRI_BRUSH)
			tempy = y + ry;
		for (i = x - rx; i <= x; i++)
		{
			oldy = tempy;
			while (brush->IsInside(i-x,tempy-y))
				tempy = tempy - 1;
			tempy = tempy + 1;
			if (fill)
			{
				//If triangle brush, create parts down to the bottom always; if not go down to the bottom border
				if (brush->GetShape() == TRI_BRUSH)
					jmax = y + ry;
				else
					jmax = 2*y - tempy;

				for (j = tempy; j <= jmax; j++)
				{
					if (CreatePartFlags(i, j, c, flags))
						f = 1;
					//don't create twice in the vertical center line
					if (i!=x && CreatePartFlags(2*x-i, j, c, flags))
						f = 1;
				}
			}
			else
			{
				if ((oldy != tempy && brush->GetShape() != SQUARE_BRUSH) || i == x-rx)
					oldy--;
				for (j = tempy; j <= oldy+1; j++)
				{
					int i2 = 2*x-i, j2 = 2*y-j;
					if (brush->GetShape() == TRI_BRUSH)
						j2 = y+ry;
					if (CreatePartFlags(i, j, c, flags))
						f = 1;
					if (i2 != i && CreatePartFlags(i2, j, c, flags))
						f = 1;
					if (j2 != j && CreatePartFlags(i, j2, c, flags))
						f = 1;
					if (i2 != i && j2 != j && CreatePartFlags(i2, j2, c, flags))
						f = 1;
				}
			}
		}
	}
	return !f;
}

int Simulation::CreatePartFlags(int x, int y, int c, int flags)
{
	//delete
	if (c == 0 && !(flags&BRUSH_REPLACEMODE))
		delete_part(x, y, flags);
	//specific delete
	else if ((flags&BRUSH_SPECIFIC_DELETE) && !(flags&BRUSH_REPLACEMODE))
	{
		if (((ElementTool*)activeTools[2])->GetID() > 0 || (pmap[y][x]&0xFF) == ((ElementTool*)activeTools[2])->GetID() || (photons[y][x]&0xFF) == ((ElementTool*)activeTools[2])->GetID())
			delete_part(x, y, flags);
	}
	//replace mode
	else if (flags&BRUSH_REPLACEMODE)
	{
		if (x<0 || y<0 || x>=XRES || y>=YRES)
			return 0;
		if (((ElementTool*)activeTools[2])->GetID() > 0 && (pmap[y][x]&0xFF) != ((ElementTool*)activeTools[2])->GetID() && (photons[y][x]&0xFF) != ((ElementTool*)activeTools[2])->GetID())
			return 0;
		if (pmap[y][x])
		{
			delete_part(x, y, flags);
			if (c!=0)
				create_part(-2, x, y, c);
		}
	}
	//normal draw
	else
		if (create_part(-2, x, y, c) == -1)
			return 1;
	return 0;
}

void Simulation::CreateLine(int x1, int y1, int x2, int y2, int c, int flags, Brush* brush)
{
	int x, y, dx, dy, sy;
	bool reverseXY = abs(y2-y1) > abs(x2-x1), fill = true;
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
			CreateParts(y, x, c, flags, fill, brush);
		else
			CreateParts(x, y, c, flags, fill, brush);
		e += de;
		fill = false;
		if (e >= 0.5f)
		{
			y += sy;
			if (!(brush->GetRadius().X+brush->GetRadius().Y) && ((y1<y2) ? (y<=y2) : (y>=y2)))
			{
				if (reverseXY)
					CreateParts(y, x, c, flags, fill, brush);
				else
					CreateParts(x, y, c, flags, fill, brush);
			}
			e -= 1.0f;
		}
	}
}

void Simulation::CreateBox(int x1, int y1, int x2, int y2, int c, int flags)
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
	for (int j = y1; j <= y2; j++)
		for (int i = x1; i <= x2; i++)
			CreateParts(i, j, c, flags, false);
}

//used for element and prop tool floodfills
int Simulation::FloodFillPmapCheck(int x, int y, int type)
{
	if (type == 0)
		return !pmap[y][x] && !photons[y][x];
	if (ptypes[type].properties&TYPE_ENERGY)
		return (photons[y][x] & 0xFF) == type;
	else
		return (pmap[y][x] & 0xFF) == type;
}

int Simulation::FloodParts(int x, int y, int fullc, int replace, int flags)
{
	int c = fullc&0xFF;
	int x1, x2;
	int created_something = 0;

	if (replace == -1)
	{
		//if initial flood point is out of bounds, do nothing
		if (c != 0 && (x < CELL || x >= XRES-CELL || y < CELL || y >= YRES-CELL))
			return 1;
		if (c==0)
		{
			replace = pmap[y][x]&0xFF;
			if(!replace && !(replace = photons[y][x]&0xFF))
			{
				if (bmap[y/CELL][x/CELL])
					return globalSim->FloodWalls(x/CELL, y/CELL, WL_ERASE, -1);
				else
					return 0;
				if ((flags&BRUSH_REPLACEMODE) && ((ElementTool*)activeTools[2])->GetID() != replace)
					return 0;
			}
		}
		else
			replace = 0;
	}
	if (c != 0 && IsWallBlocking(x, y, c))
		return 0;

	if (!FloodFillPmapCheck(x, y, replace) || ((flags&BRUSH_SPECIFIC_DELETE) && ((ElementTool*)activeTools[2])->GetID() != replace))
		return 0;

	try
	{
		CoordStack cs;
		cs.push(x, y);

		do
		{
			cs.pop(x, y);
			x1 = x2 = x;
			// go left as far as possible
			while (c?x1>CELL:x1>0)
			{
				if (!FloodFillPmapCheck(x1-1, y, replace) || (c != 0 && IsWallBlocking(x1-1, y, c)))
				{
					break;
				}
				x1--;
			}
			// go right as far as possible
			while (c?x2<XRES-CELL-1:x2<XRES-1)
			{
				if (!FloodFillPmapCheck(x2+1, y, replace) || (c != 0 && IsWallBlocking(x2+1, y, c)))
				{
					break;
				}
				x2++;
			}
			// fill span
			for (x=x1; x<=x2; x++)
			{
				if (CreateParts(x, y, fullc, flags, true));
					created_something = 1;
			}

			if (c ? y>CELL : y>0)
				for (x=x1; x<=x2; x++)
					if (FloodFillPmapCheck(x, y-1, replace) && (c == 0 || !IsWallBlocking(x, y-1, c)))
					{
						cs.push(x, y-1);
					}

			if (c ? y<=YRES-CELL : y<=YRES)
				for (x=x1; x<=x2; x++)
					if (FloodFillPmapCheck(x, y+1, replace) && (c == 0 || !IsWallBlocking(x, y+1, c)))
					{
						cs.push(x, y+1);
					}
		}
		while (cs.getSize() > 0);
	}
	catch (std::exception& e)
	{
		return -1;
	}
	return created_something;
}

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
	//streamlines can't be drawn next to each other
	else if (wall == WL_STREAM)
	{
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
			CreateWallBox(y-ry, x-rx, y+rx, x+ry, wall);
		else
			CreateWallBox(x-rx, y-ry, x+rx, y+ry, wall);
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			if ((y1<y2) ? (y<=y2) : (y>=y2))
			{
				if (reverseXY)
					CreateWallBox(y-ry, x-rx, y+rx, x+ry, wall);
				else
					CreateWallBox(x-rx, y-ry, x+rx, y+ry, wall);
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
	for (int j = y1; j <= y2; j++)
		for (int i = x1; i <= x2; i++)
			CreateWall(i, j, wall);
}

int Simulation::FloodWalls(int x, int y, int wall, int replace)
{
	int x1, x2;
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

int Simulation::CreateTool(int x, int y, int tool, float strength)
{
	if (!InBounds(x, y))
		return -2;
	if (tool == TOOL_HEAT || tool == TOOL_COOL)
	{
		int r = pmap[y][x];
		if (!(r&0xFF))
			r = photons[y][x];
		if (r&0xFF)
		{
			float heatchange;
			if ((r&0xFF) == PT_PUMP || (r&0xFF) == PT_GPMP)
				heatchange = strength*.1f;
			else if ((r&0xFF) == PT_ANIM)
				heatchange = strength;
			else
				heatchange = strength*2.0f;

			if (tool == TOOL_HEAT)
			{
				parts[r>>8].temp = restrict_flt(parts[r>>8].temp + heatchange, MIN_TEMP, MAX_TEMP);
			}
			else if (tool == TOOL_COOL)
			{
				parts[r>>8].temp = restrict_flt(parts[r>>8].temp - heatchange, MIN_TEMP, MAX_TEMP);
			}
			return r>>8;
		}
		else
		{
			return -1;
		}
	}
	else if (tool == TOOL_AIR)
	{
		pv[y/CELL][x/CELL] += strength*.05f;
		return -1;
	}
	else if (tool == TOOL_VAC)
	{
		pv[y/CELL][x/CELL] -= strength*.05f;
		return -1;
	}
	else if (tool == TOOL_PGRV)
	{
		gravmap[(y/CELL)*(XRES/CELL)+(x/CELL)] = strength*5.0f;
		return -1;
	}
	else if (tool == TOOL_NGRV)
	{
		gravmap[(y/CELL)*(XRES/CELL)+(x/CELL)] = strength*-5.0f;
		return -1;
	}
	return -1;
}

void Simulation::CreateToolBrush(int x, int y, int tool, float strength, Brush* brush)
{
	int rx = brush->GetRadius().X, ry = brush->GetRadius().Y;
	if (rx <= 0) //workaround for rx == 0 crashing. todo: find a better fix later.
	{
		for (int j = y - ry; j <= y + ry; j++)
			CreateTool(x, j, tool, strength);
	}
	else
	{
		int tempy = y, i, j, jmax, oldy;
		// tempy is the smallest y value that is still inside the brush
		// jmax is the largest y value that is still inside the brush (bottom border of brush)

		//For triangle brush, start at the very bottom
		if (brush->GetShape() == TRI_BRUSH)
			tempy = y + ry;
		for (i = x - rx; i <= x; i++)
		{
			oldy = tempy;
			//loop up until it finds a point not in the brush
			while (brush->IsInside(i-x,tempy-y))
				tempy = tempy - 1;
			tempy = tempy + 1;

			//If triangle brush, create parts down to the bottom always; if not go down to the bottom border
			if (brush->GetShape() == TRI_BRUSH)
				jmax = y + ry;
			else
				jmax = 2*y - tempy;
			for (j = tempy; j <= jmax; j++)
			{
				CreateTool(i, j, tool, strength);
				//don't create twice in the vertical center line
				if (i != x)
					CreateTool(2*x-i, j, tool, strength);
			}
		}
	}
}

void Simulation::CreateToolLine(int x1, int y1, int x2, int y2, int tool, float strength, Brush* brush)
{
	if (tool == TOOL_WIND)
	{
		int rx = brush->GetRadius().X, ry = brush->GetRadius().Y;
		for (int j = -ry; j <= ry; j++)
			for (int i = -rx; i <= rx; i++)
				if (x2+i>0 && y2+j>0 && x2+i<XRES && y2+j<YRES && brush->IsInside(i, j))
				{
					vx[(y2+j)/CELL][(x2+i)/CELL] += (x2-x1)*0.01f;
					vy[(y2+j)/CELL][(x2+i)/CELL] += (y2-y1)*0.01f;
				}
		return;
	}
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
			CreateToolBrush(y, x, tool, strength, brush);
		else
			CreateToolBrush(x, y, tool, strength, brush);
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			if (!(brush->GetRadius().X+brush->GetRadius().Y) && ((y1<y2) ? (y<=y2) : (y>=y2)))
			{
				if (reverseXY)
					CreateToolBrush(y, x, tool, strength, brush);
				else
					CreateToolBrush(x, y, tool, strength, brush);
			}
			e -= 1.0f;
		}
	}
}

void Simulation::CreateToolBox(int x1, int y1, int x2, int y2, int tool, float strength)
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
	for (int j = y1; j <= y2; j++)
		for (int i = x1; i <= x2; i++)
			CreateTool(i, j, tool, strength);
}

int Simulation::CreateProp(int x, int y, PropertyType propType, PropertyValue propValue, size_t propOffset)
{
	if (!InBounds(x, y))
		return -2;

	int i = pmap[y][x];
	if (!i)
		i = photons[y][x];

	if (i & 0xFF)
	{
		if (propType == Integer)
			*((int*)(((char*)&parts[i>>8]) + propOffset)) = propValue.Integer;
		else if (propType == UInteger)
			*((unsigned int*)(((char*)&parts[i>>8]) + propOffset)) = propValue.UInteger;
		else if (propType == Float)
			*((float*)(((char*)&parts[i>>8]) + propOffset)) = propValue.Float;
		return i >> 8;
	}
	return -1;
}

void Simulation::CreatePropBrush(int x, int y, PropertyType propType, PropertyValue propValue, size_t propOffset, Brush* brush)
{
	int rx = brush->GetRadius().X, ry = brush->GetRadius().Y;
	if (rx <= 0) //workaround for rx == 0 crashing. todo: find a better fix later.
	{
		for (int j = y - ry; j <= y + ry; j++)
			CreateProp(x, j, propType, propValue, propOffset);
	}
	else
	{
		int tempy = y, i, j, jmax, oldy;
		// tempy is the smallest y value that is still inside the brush
		// jmax is the largest y value that is still inside the brush (bottom border of brush)

		//For triangle brush, start at the very bottom
		if (brush->GetShape() == TRI_BRUSH)
			tempy = y + ry;
		for (i = x - rx; i <= x; i++)
		{
			oldy = tempy;
			//loop up until it finds a point not in the brush
			while (brush->IsInside(i - x, tempy - y))
				tempy = tempy - 1;
			tempy = tempy + 1;

			//If triangle brush, create parts down to the bottom always; if not go down to the bottom border
			if (brush->GetShape() == TRI_BRUSH)
				jmax = y + ry;
			else
				jmax = 2 * y - tempy;
			for (j = tempy; j <= jmax; j++)
			{
				CreateProp(i, j, propType, propValue, propOffset);
				//don't create twice in the vertical center line
				if (i != x)
					CreateProp(2 * x - i, j, propType, propValue, propOffset);
			}
			//TODO: should use fill argument like creating elements does (but all the repetitiveness here needs to be removed first)
		}
	}
}

void Simulation::CreatePropLine(int x1, int y1, int x2, int y2, PropertyType propType, PropertyValue propValue, size_t propOffset, Brush* brush)
{
	int x, y, dx, dy, sy;
	bool reverseXY = abs(y2 - y1) > abs(x2 - x1);
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
		de = dy / (float)dx;
	else
		de = 0.0f;
	y = y1;
	sy = (y1<y2) ? 1 : -1;
	for (x = x1; x <= x2; x++)
	{
		if (reverseXY)
			CreatePropBrush(y, x, propType, propValue, propOffset, brush);
		else
			CreatePropBrush(x, y, propType, propValue, propOffset, brush);
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			if (!(brush->GetRadius().X +  brush->GetRadius().Y) && ((y1<y2) ? (y <= y2) : (y >= y2)))
			{
				if (reverseXY)
					CreatePropBrush(y, x, propType, propValue, propOffset, brush);
				else
					CreatePropBrush(x, y, propType, propValue, propOffset, brush);
			}
			e -= 1.0f;
		}
	}
}

void Simulation::CreatePropBox(int x1, int y1, int x2, int y2, PropertyType propType, PropertyValue propValue, size_t propOffset)
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
	for (int j = y1; j <= y2; j++)
		for (int i = x1; i <= x2; i++)
			CreateProp(i, j, propType, propValue, propOffset);
}

int Simulation::FloodPropHelper(int x, int y, int partType, PropertyType propType, PropertyValue propValue, size_t propOffset, char * bitmap)
{
	int x1, x2, dy = 1;
	x1 = x2 = x;
	while (x1 >= CELL)
	{
		if (!FloodFillPmapCheck(x1 - 1, y, partType) || bitmap[(y*XRES) + x1 - 1])
		{
			break;
		}
		x1--;
	}
	while (x2<XRES - CELL)
	{
		if (!FloodFillPmapCheck(x2 + 1, y, partType) || bitmap[(y*XRES) + x2 + 1])
		{
			break;
		}
		x2++;
	}
	for (x = x1; x <= x2; x++)
	{
		CreateProp(x, y, propType, propValue, propOffset);
		bitmap[(y*XRES) + x] = 1;
	}
	if (y >= CELL + dy)
		for (x = x1; x <= x2; x++)
			if (FloodFillPmapCheck(x, y - dy, partType) && !bitmap[((y - dy)*XRES) + x])
				if (!FloodPropHelper(x, y - dy, partType, propType, propValue, propOffset, bitmap))
					return 0;
	if (y<YRES - CELL - dy)
		for (x = x1; x <= x2; x++)
			if (FloodFillPmapCheck(x, y + dy, partType) && !bitmap[((y + dy)*XRES) + x])
				if (!FloodPropHelper(x, y + dy, partType, propType, propValue, propOffset, bitmap))
					return 0;
	return 1;
}

int Simulation::FloodProp(int x, int y, PropertyType propType, PropertyValue propValue, size_t propOffset)
{
	int r = 0;
	char * bitmap = (char*)malloc(XRES*YRES); //Bitmap for checking
	if (bitmap == 0)
		return 0;
	memset(bitmap, 0, XRES*YRES);
	r = pmap[y][x];
	if (!r)
		r = photons[y][x];
	if (!r)
		return 1;
	FloodPropHelper(x, y, r & 0xFF, propType, propValue, propOffset, bitmap);
	free(bitmap);
	return 0;
}

void Simulation::CreateDeco(int x, int y, int tool, unsigned int color)
{
	int rp, tr = 0, tg = 0, tb = 0;
	float strength = 0.01f, colr, colg, colb, cola;
	if (!InBounds(x, y))
		return;

	rp = pmap[y][x];
	if (!rp)
		rp = photons[y][x];
	if (!rp)
		return;

	switch (tool)
	{
	case DECO_DRAW:
		parts[rp>>8].dcolour = color;
		break;
	case DECO_CLEAR:
		parts[rp>>8].dcolour = 0;
		break;
	case DECO_ADD:
	case DECO_SUBTRACT:
	case DECO_MULTIPLY:
	case DECO_DIVIDE:
		if (!parts[rp>>8].dcolour)
			return;
		cola = COLA(color)/255.0f;
		colr = (float)((parts[rp>>8].dcolour>>16)&0xFF);
		colg = (float)((parts[rp>>8].dcolour>>8)&0xFF);
		colb = (float)((parts[rp>>8].dcolour)&0xFF);

		if (tool == DECO_ADD)
		{
			colr += (COLR(color)*strength)*cola;
			colg += (COLG(color)*strength)*cola;
			colb += (COLB(color)*strength)*cola;
		}
		else if (tool == DECO_SUBTRACT)
		{
			colr -= (COLR(color)*strength)*cola;
			colg -= (COLG(color)*strength)*cola;
			colb -= (COLB(color)*strength)*cola;
		}
		else if (tool == DECO_MULTIPLY)
		{
			colr *= 1.0f+(COLR(color)/255.0f*strength)*cola;
			colg *= 1.0f+(COLG(color)/255.0f*strength)*cola;
			colb *= 1.0f+(COLB(color)/255.0f*strength)*cola;
		}
		else if (tool == DECO_DIVIDE)
		{
			colr /= 1.0f+(COLR(color)/255.0f*strength)*cola;
			colg /= 1.0f+(COLG(color)/255.0f*strength)*cola;
			colb /= 1.0f+(COLB(color)/255.0f*strength)*cola;
		}

		tr = int(colr+.5f); tg = int(colg+.5f); tb = int(colb+.5f);
		if (tr > 255) tr = 255;
		else if (tr < 0) tr = 0;
		if (tg > 255) tg = 255;
		else if (tg < 0) tg = 0;
		if (tb > 255) tb = 255;
		else if (tb < 0) tb = 0;

		parts[rp>>8].dcolour = COLRGB(tr, tg, tb);
		break;
	case DECO_LIGHTEN:
		if (!parts[rp>>8].dcolour)
			return;
		tr = (parts[rp>>8].dcolour>>16)&0xFF;
		tg = (parts[rp>>8].dcolour>>8)&0xFF;
		tb = (parts[rp>>8].dcolour)&0xFF;
		parts[rp>>8].dcolour = ((parts[rp>>8].dcolour&0xFF000000)|(clamp_flt(tr+(255-tr)*0.02+1, 0,255)<<16)|(clamp_flt(tg+(255-tg)*0.02+1, 0,255)<<8)|clamp_flt(tb+(255-tb)*0.02+1, 0,255));
		break;
	case DECO_DARKEN:
		if (!parts[rp>>8].dcolour)
			return;
		tr = (parts[rp>>8].dcolour>>16)&0xFF;
		tg = (parts[rp>>8].dcolour>>8)&0xFF;
		tb = (parts[rp>>8].dcolour)&0xFF;
		parts[rp>>8].dcolour = ((parts[rp>>8].dcolour&0xFF000000)|(clamp_flt(tr-(tr)*0.02, 0,255)<<16)|(clamp_flt(tg-(tg)*0.02, 0,255)<<8)|clamp_flt(tb-(tb)*0.02, 0,255));
		break;
	case DECO_SMUDGE:
		if (x >= CELL && x < XRES-CELL && y >= CELL && y < YRES-CELL)
		{
			int rx, ry, num = 0, ta = 0;
			for (rx=-2; rx<3; rx++)
				for (ry=-2; ry<3; ry++)
				{
					if (abs(rx)+abs(ry) > 2 && (pmap[y+ry][x+rx]&0xFF) && parts[pmap[y+ry][x+rx]>>8].dcolour)
					{
						num++;
						ta += (parts[pmap[y+ry][x+rx]>>8].dcolour>>24)&0xFF;
						tr += (parts[pmap[y+ry][x+rx]>>8].dcolour>>16)&0xFF;
						tg += (parts[pmap[y+ry][x+rx]>>8].dcolour>>8)&0xFF;
						tb += (parts[pmap[y+ry][x+rx]>>8].dcolour)&0xFF;
					}
				}
			if (num == 0)
				return;
			ta = std::min(255,(int)((float)ta/num+.5));
			tr = std::min(255,(int)((float)tr/num+.5));
			tg = std::min(255,(int)((float)tg/num+.5));
			tb = std::min(255,(int)((float)tb/num+.5));
			if (!parts[rp>>8].dcolour)
				ta = std::max(0,ta-3);
			parts[rp>>8].dcolour = ((ta<<24)|(tr<<16)|(tg<<8)|tb);
		}
		break;
	}

	if (parts[rp>>8].type == PT_ANIM)
	{
		parts[rp>>8].animations[framenum] = parts[rp>>8].dcolour;
	}
}

void Simulation::CreateDecoBrush(int x, int y, int tool, unsigned int color, Brush* brush)
{
	int rx = brush->GetRadius().X, ry = brush->GetRadius().Y;
	if (rx <= 0) //workaround for rx == 0 crashing. todo: find a better fix later.
	{
		for (int j = y - ry; j <= y + ry; j++)
			CreateDeco(x, j, tool, color);
	}
	else
	{
		int tempy = y, i, j, jmax, oldy;
		// tempy is the smallest y value that is still inside the brush
		// jmax is the largest y value that is still inside the brush (bottom border of brush)

		//For triangle brush, start at the very bottom
		if (brush->GetShape() == TRI_BRUSH)
			tempy = y + ry;
		for (i = x - rx; i <= x; i++)
		{
			oldy = tempy;
			//loop up until it finds a point not in the brush
			while (brush->IsInside(i-x,tempy-y))
				tempy = tempy - 1;
			tempy = tempy + 1;

			//If triangle brush, create parts down to the bottom always; if not go down to the bottom border
			if (brush->GetShape() == TRI_BRUSH)
				jmax = y + ry;
			else
				jmax = 2*y - tempy;

			for (j = tempy; j <= jmax; j++)
			{
				CreateDeco(i, j, tool, color);
				//don't create twice in the vertical center line
				if (i != x)
					CreateDeco(2*x-i, j, tool, color);
			}
		}
	}
}

void Simulation::CreateDecoLine(int x1, int y1, int x2, int y2, int tool, unsigned int color, Brush* brush)
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
			CreateDecoBrush(y, x, tool, color, brush);
		else
			CreateDecoBrush(x, y, tool, color, brush);
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			if (!(brush->GetRadius().X+brush->GetRadius().Y) && ((y1<y2) ? (y<=y2) : (y>=y2)))
			{
				if (reverseXY)
					CreateDecoBrush(y, x, tool, color, brush);
				else
					CreateDecoBrush(x, y, tool, color, brush);
			}
			e -= 1.0f;
		}
	}
}

void Simulation::CreateDecoBox(int x1, int y1, int x2, int y2, int tool, unsigned int color)
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
	for (int j = y1; j <= y2; j++)
		for (int i = x1; i <= x2; i++)
			CreateDeco(i, j, tool, color);
}

void Simulation::FloodDeco(int x, int y, unsigned int color, unsigned int replace)
{
	//TODO: implement
}

/*
 *
 *End of tool creation section
 *
 */

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
		ptypes[t].hotair = sim->elements[t].HotAir;
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
