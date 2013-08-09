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

int H2_update(UPDATE_FUNC_ARGS)
{
	int r,rx,ry,rt;
	if (parts[i].temp > 2273.15 && pv[y/CELL][x/CELL] > 50.0f)
		parts[i].tmp = 1;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				rt = (r&0xFF);
				if (!r)
					continue;
				if (pv[y/CELL][x/CELL] > 8.0f && rt == PT_DESL) // This will not work. DESL turns to fire above 5.0 pressure
				{
					part_change_type(r>>8,x+rx,y+ry,PT_WATR);
					part_change_type(i,x,y,PT_OIL);
				}
				if (parts[r>>8].temp > 2273.15 && pv[y/CELL][x/CELL] > 45.0f)
					continue;
				if (pv[x/CELL][y/CELL] < 45.0f)
				{
					if (rt==PT_FIRE)
					{
						parts[r>>8].temp=2473.15f;
						if(parts[r>>8].tmp&0x02)
							parts[r>>8].temp=3473;
						parts[r>>8].tmp |= 1;
					}
					if (rt==PT_FIRE || (rt==PT_PLSM && !(parts[r>>8].tmp&1)) || (rt==PT_LAVA && parts[r>>8].ctype != PT_BMTL))
					{
						sim->part_create(i,x,y,PT_FIRE);
						parts[i].temp+=(rand()%250);
						parts[i].tmp |= 1;
						pv[y/CELL][x/CELL] += 0.1f;
					}
				}
			}
	if (parts[i].temp > 2273.15f && pv[y/CELL][x/CELL] > 50.0f)
	{
		if (rand()%5 < 1)
		{
			int j;
			float temp = parts[i].temp;
			sim->part_create(i,x,y,PT_NBLE);

			j = sim->part_create(-3,x+rand()%3-1,y+rand()%3-1,PT_NEUT);
			if (j != -1)
				parts[j].temp = temp;
			if (!(rand()%10))
			{
				j = sim->part_create(-3,x+rand()%3-1,y+rand()%3-1,PT_ELEC);
				if (j != -1)
					parts[j].temp = temp;
			}
			j = sim->part_create(-3,x+rand()%3-1,y+rand()%3-1,PT_PHOT);
			if (j != -1)
			{
				parts[j].ctype = 0x7C0000;
				parts[j].temp = temp;
			}
			j = sim->part_create(-3,x+rand()%3-1,y+rand()%3-1,PT_PLSM);
			if (j != -1)
			{
				parts[j].temp = temp;
				parts[j].tmp |= 1;
			}

			parts[i].temp = temp+750+rand()%500;
			pv[y/CELL][x/CELL] += 30;
		}
	}
	return 0;
}

void H2_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_H2";
	elem->Name = "HYGN";
	elem->Colour = COLPACK(0x5070FF);
	elem->MenuVisible = 1;
	elem->MenuSection = SC_GAS;
	elem->Enabled = 1;

	elem->Advection = 2.0f;
	elem->AirDrag = 0.00f * CFDS;
	elem->AirLoss = 0.99f;
	elem->Loss = 0.30f;
	elem->Collision = -0.10f;
	elem->Gravity = 0.00f;
	elem->Diffusion = 3.00f;
	elem->PressureAdd_NoAmbHeat = 0.000f	* CFDS;
	elem->Falldown = 0;

	elem->Flammable = 0;
	elem->Explosive = 0;
	elem->Meltable = 0;
	elem->Hardness = 0;

	elem->Weight = 1;

	elem->DefaultProperties.temp = R_TEMP+0.0f +273.15f;
	elem->HeatConduct = 251;
	elem->Latent = 0;
	elem->Description = "Hydrogen. Combusts with OXYG to make WATR. Undergoes fusion at high temperature and pressure.";

	elem->State = ST_GAS;
	elem->Properties = TYPE_GAS;

	elem->LowPressureTransitionThreshold = IPL;
	elem->LowPressureTransitionElement = NT;
	elem->HighPressureTransitionThreshold = IPH;
	elem->HighPressureTransitionElement = NT;
	elem->LowTemperatureTransitionThreshold = ITL;
	elem->LowTemperatureTransitionElement = NT;
	elem->HighTemperatureTransitionThreshold = ITH;
	elem->HighTemperatureTransitionElement = NT;

	elem->Update = &H2_update;
	elem->Graphics = NULL;
	elem->Init = &H2_init_element;
}
