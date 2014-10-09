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

#include "simulation/ElementsCommon.h"
#include "MOVS.h"

void rotate(float *x, float *y, float angle)
{
	float cos = cosf(angle), sin = sinf(angle);
	float newx = cos**x-sin**y, newy = sin**x+cos**y;
	*x = newx, *y = newy;
}

int MOVS_update(UPDATE_FUNC_ARGS)
{
	int bn = parts[i].tmp2, type, bounce = 2;
	float tmp = parts[i].pavg[0], tmp2 = parts[i].pavg[1];

	MovingSolid *movingSolid = ((MOVS_ElementDataContainer*)sim->elementData[PT_MOVS])->GetMovingSolid(bn);
	if (!movingSolid || (parts[i].flags&FLAG_DISAPPEAR))
		return 0;
	//center control particle was killed, ball slowly falls apart
	if (!movingSolid->index)
	{
		if (rand()%500<1)
		{
			kill_part(i);
			return 1;
		}
	}
	//determine rotated x and y coordinates relative to center (if rotation is on)
	else
	{
		tmp = parts[i].pavg[0];
		tmp2 = parts[i].pavg[1];
		if (sim->msRotation)
			rotate(&tmp, &tmp2, movingSolid->rotationOld);
	}
	//kill moving solid control particle with a lot of pressure (other ones disappear at 30 pressure)
	if (!tmp && !tmp2 && pv[y/CELL][x/CELL] > 10 || pv[y/CELL][x/CELL] < -10)
	{
		kill_part(i);
		return 1;
	}
	type = pmap[y+1][x]&0xFF;
	//bottom side collision
	if (tmp2 > 0 && type && y+1 < YRES && ((type != PT_MOVS && !eval_move(PT_MOVS, x, y+1, NULL)) || (type == PT_MOVS && parts[pmap[y+1][x]>>8].tmp2 != bn) || IsWallBlocking(x, y+1, PT_MOVS)))
	{
		parts[i].vy -= tmp2*bounce;
		movingSolid->rotation -= tmp/50000;
	}
	type = pmap[y-1][x]&0xFF;
	//top side collision
	if (tmp2 < 0 && type && y-1 >= 0 && ((type != PT_MOVS && !eval_move(PT_MOVS, x, y-1, NULL)) || (type == PT_MOVS && parts[pmap[y-1][x]>>8].tmp2 != bn) || IsWallBlocking(x, y-1, PT_MOVS)))
	{
		parts[i].vy -= tmp2*bounce;
		movingSolid->rotation -= tmp/50000;
	}
	type = pmap[y][x+1]&0xFF;
	//right side collision
	if (tmp > 0 && type && x+1 < XRES && ((type != PT_MOVS && !eval_move(PT_MOVS, x+1, y, NULL)) || (type == PT_MOVS && parts[pmap[y][x+1]>>8].tmp2 != bn) || IsWallBlocking(x+1, y, PT_MOVS)))
	{
		parts[i].vx -= tmp*bounce;
		movingSolid->rotation -= tmp/50000;
	}
	type = pmap[y][x-1]&0xFF;
	//left side collision
	if (tmp < 0 && type && x-1 >= 0 && ((type != PT_MOVS && !eval_move(PT_MOVS, x-1, y, NULL)) || (type == PT_MOVS && parts[pmap[y][x-1]>>8].tmp2 != bn) || IsWallBlocking(x-1, y, PT_MOVS)))
	{
		parts[i].vx -= tmp*bounce;
		movingSolid->rotation -= tmp/50000;
	}
	return 0;
}

bool MOVS_create_allowed(ELEMENT_CREATE_ALLOWED_FUNC_ARGS)
{
	if (((MOVS_ElementDataContainer*)sim->elementData[PT_MOVS])->GetNumBalls() >= 255 || (pmap[y][x]&0xFF))
		return false;
	return true;
}

void MOVS_create(ELEMENT_CREATE_FUNC_ARGS)
{
	if (v == 2 || ((MOVS_ElementDataContainer*)sim->elementData[PT_MOVS])->IsCreatingSolid())
	{
		((MOVS_ElementDataContainer*)sim->elementData[PT_MOVS])->CreateMovingSolid(i, x, y);
	}
	else if (v == 1)
	{
		((MOVS_ElementDataContainer*)sim->elementData[PT_MOVS])->CreateMovingSolidCenter(i);
	}
	else
	{
		parts[i].tmp2 = 255;
		parts[i].pavg[0] = rand()%20-10.0f;
		parts[i].pavg[1] = rand()%20-10.0f;
	}
}

void MOVS_ChangeType(ELEMENT_CHANGETYPE_FUNC_ARGS)
{
	if (to != PT_MOVS)
	{
		MovingSolid *movingSolid = ((MOVS_ElementDataContainer*)sim->elementData[PT_MOVS])->GetMovingSolid(parts[i].tmp2);
		if (movingSolid && !(parts[i].flags&FLAG_DISAPPEAR))
		{
			movingSolid->particleCount--;
			if (movingSolid->index-1 == i)
				movingSolid->index = 0;
		}
	}
}

void MOVS_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_MOVS";
	elem->Name = "BALL";
	elem->Colour = COLPACK(0x0010A0);
	elem->MenuVisible = 1;
	elem->MenuSection = SC_SPECIAL;
	elem->Enabled = 1;

	elem->Advection = 0.4f;
	elem->AirDrag = 0.004f * CFDS;
	elem->AirLoss = 0.92f;
	elem->Loss = 0.80f;
	elem->Collision = 0.00f;
	elem->Gravity = 0.1f;
	elem->Diffusion = 0.00f;
	elem->HotAir = 0.000f	* CFDS;
	elem->Falldown = 0;

	elem->Flammable = 0;
	elem->Explosive = 0;
	elem->Meltable = 0;
	elem->Hardness = 30;

	elem->Weight = 85;

	elem->DefaultProperties.temp = R_TEMP + 273.15f;
	elem->HeatConduct = 70;
	elem->Latent = 0;
	elem->Description = "Moving solid. Acts like a bouncy ball.";

	elem->State = ST_NONE;
	elem->Properties = TYPE_PART;

	elem->LowPressureTransitionThreshold = -25.0f;
	elem->LowPressureTransitionElement = PT_NONE;
	elem->HighPressureTransitionThreshold = 25.0f;
	elem->HighPressureTransitionElement = PT_NONE;
	elem->LowTemperatureTransitionThreshold = ITL;
	elem->LowTemperatureTransitionElement = NT;
	elem->HighTemperatureTransitionThreshold = ITH;
	elem->HighTemperatureTransitionElement = NT;

	elem->Update = &MOVS_update;
	elem->Graphics = NULL;
	elem->Func_Create_Allowed = &MOVS_create_allowed;
	elem->Func_Create = &MOVS_create;
	elem->Func_ChangeType = &MOVS_ChangeType;
	elem->Init = &MOVS_init_element;

	if (sim->elementData[t])
	{
		delete sim->elementData[t];
	}
	sim->elementData[t] = new MOVS_ElementDataContainer;
}
