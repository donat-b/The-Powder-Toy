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

int VIRS_update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	if (parts[i].pavg[0])
	{
		parts[i].pavg[0] -= rand()%2 < 1 ? 0:1;
		if ((parts[i].pavg[0]) == 0)
		{
			part_change_type(i,x,y,parts[i].tmp2);
			parts[i].tmp2 = 0;
			parts[i].pavg[0] = 0;
			parts[i].pavg[1] = 0;
			return 0;
		}
	}
	if (parts[i].pavg[1] > 0)
	{
		if (rand()%15 < 1)
			parts[i].pavg[1]--;
	}
	else if (!parts[i].pavg[0])
	{
		kill_part(i);
		return 1;
	}
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = ((pmap[y+ry][x+rx]&0xFF)==PT_PINV&&parts[pmap[y+ry][x+rx]>>8].life==10)?0:pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (((r&0xFF) == PT_VIRS || (r&0xFF) == PT_VRSS || (r&0xFF) == PT_VRSG) && parts[r>>8].pavg[0] && !parts[i].pavg[0])
				{
					int newtmp = (int)parts[r>>8].pavg[0] + (rand()%6 < 1 ? 1:2);
					parts[i].pavg[0] = (float)newtmp;
				}
				else if (!(parts[i].pavg[0] || parts[i].pavg[0] > 10) && (r&0xFF) == PT_CURE)
				{
					parts[i].pavg[0] += 10;
					if (rand()%10<1)
						kill_part(r>>8);
				}
				else if (!parts[i].pavg[0] && (r&0xFF) != PT_VIRS && (r&0xFF) != PT_VRSS && (r&0xFF) != PT_VRSG && !(ptypes[r&0xFF].properties&PROP_INDESTRUCTIBLE))
				{
					if (rand()%50<1)
					{
						int newtmp = (int)parts[i].pavg[1] + (rand()%3 < 1 ? 0:1);
						parts[r>>8].tmp2 = (r&0xFF);
						parts[r>>8].pavg[0] = 0;
						parts[r>>8].pavg[1] = (float)newtmp;
						if (parts[r>>8].temp < 305)
							part_change_type(r>>8,x,y,PT_VRSS);
						else if (parts[r>>8].temp > 673)
							part_change_type(r>>8,x,y,PT_VRSG);
						else
							part_change_type(r>>8,x,y,PT_VIRS);
					}
				}
			}
	return 0;
}

void VIRS_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_VIRS";
	elem->Name = "VIRS";
	elem->Colour = COLPACK(0xFE11F6);
	elem->MenuVisible = 1;
	elem->MenuSection = SC_LIQUID;
	elem->Enabled = 1;

	elem->Advection = 0.6f;
	elem->AirDrag = 0.01f * CFDS;
	elem->AirLoss = 0.98f;
	elem->Loss = 0.95f;
	elem->Collision = 0.0f;
	elem->Gravity = 0.1f;
	elem->Diffusion = 0.00f;
	elem->PressureAdd_NoAmbHeat = 0.000f	* CFDS;
	elem->Falldown = 2;

	elem->Flammable = 100;
	elem->Explosive = 0;
	elem->Meltable = 0;
	elem->Hardness = 20;

	elem->Weight = 31;

	elem->DefaultProperties.temp = 72.0f + 273.15f;
	elem->HeatConduct = 251;
	elem->Latent = 0;
	elem->Description = "Virus. Turns everything it touches into virus.";

	elem->State = ST_LIQUID;
	elem->Properties = TYPE_LIQUID;

	elem->LowPressureTransitionThreshold = IPL;
	elem->LowPressureTransitionElement = NT;
	elem->HighPressureTransitionThreshold = IPH;
	elem->HighPressureTransitionElement = NT;
	elem->LowTemperatureTransitionThreshold = 305.0f;
	elem->LowTemperatureTransitionElement = PT_VRSS;
	elem->HighTemperatureTransitionThreshold = 673.0f;
	elem->HighTemperatureTransitionElement = PT_VRSG;

	elem->DefaultProperties.pavg[1] = 100;

	elem->Update = &VIRS_update;
	elem->Graphics = NULL;
}
