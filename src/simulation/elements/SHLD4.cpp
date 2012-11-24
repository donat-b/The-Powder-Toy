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

int SHLD4_update(UPDATE_FUNC_ARGS)
{
	int r, nnx, nny, rx, ry, np;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
				{
					if (1>rand()%5500)
					{
						np = create_part(-1,x+rx,y+ry,PT_SHLD1);
						if (np<0) continue;
						parts[np].life=7;
						part_change_type(i,x,y,PT_SHLD2);
					}
					else
						continue;

				}
				if ((r&0xFF)==PT_SHLD2 && parts[i].life>3)
				{
					part_change_type(r>>8,x+rx,y+ry,PT_SHLD3);
					parts[r>>8].life = 7;
				}
				else if ((r&0xFF)==PT_SPRK&&parts[i].life==0)
					for ( nnx=-1; nnx<2; nnx++)
						for ( nny=-1; nny<2; nny++)
						{
							if (!pmap[y+ry+nny][x+rx+nnx])
							{
								np = create_part(-1,x+rx+nnx,y+ry+nny,PT_SHLD1);
								if (np<0) continue;
								parts[np].life=7;
							}
						}
			}
	return 0;
}

void SHLD4_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_SHLD4";
	elem->Name = "SHD4";
	elem->Colour = COLPACK(0x212121);
	elem->MenuVisible = 0;
	elem->MenuSection = SC_CRACKER;
	elem->Enabled = 1;

	elem->Advection = 0.0f;
	elem->AirDrag = 0.00f * CFDS;
	elem->AirLoss = 1.00f;
	elem->Loss = 0.00f;
	elem->Collision = 0.0f;
	elem->Gravity = 0.0f;
	elem->Diffusion = 0.00f;
	elem->PressureAdd_NoAmbHeat = 0.000f	* CFDS;
	elem->Falldown = 0;

	elem->Flammable = 0;
	elem->Explosive = 0;
	elem->Meltable = 0;
	elem->Hardness = 1;

	elem->Weight = 100;

	elem->DefaultProperties.temp = R_TEMP+0.0f	+273.15f;
	elem->HeatConduct = 0;
	elem->Latent = 0;
	elem->Description = "Shield lvl 4.";

	elem->State = ST_SOLID;
	elem->Properties = TYPE_SOLID|PROP_LIFE_DEC;

	elem->LowPressureTransitionThreshold = IPL;
	elem->LowPressureTransitionElement = NT;
	elem->HighPressureTransitionThreshold = 40.0f;
	elem->HighPressureTransitionElement = PT_NONE;
	elem->LowTemperatureTransitionThreshold = ITL;
	elem->LowTemperatureTransitionElement = NT;
	elem->HighTemperatureTransitionThreshold = ITH;
	elem->HighTemperatureTransitionElement = NT;

	elem->Update = &SHLD4_update;
	elem->Graphics = NULL;
}
