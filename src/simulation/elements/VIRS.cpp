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
	//pavg[0] measures how many frames until it is cured (0 if still actively spreading and not being cured)
	//pavg[1] measures how many frames until it dies 
	int r, rx, ry, rndstore = rand();
	if (parts[i].pavg[0])
	{
		parts[i].pavg[0] -= (rndstore&0x1) ? 0:1;
		//has been cured, so change back into the original element
		if (!parts[i].pavg[0])
		{
			part_change_type(i,x,y,parts[i].tmp2);
			parts[i].tmp2 = 0;
			parts[i].pavg[0] = 0;
			parts[i].pavg[1] = 0;
			return 0;
		}
	}
	//decrease pavg[1] so it slowly dies
	if (parts[i].pavg[1] > 0)
	{
		if (((rndstore>>1)&0xD) < 1)
		{
			parts[i].pavg[1]--;
			//if pavg[1] is now 0 and it's not in the process of being cured, kill it
			if (!parts[i].pavg[1] && !parts[i].pavg[0])
			{
				kill_part(i);
				return 1;
			}
		}
	}

	//none of the things in the below loop happen while virus is being cured
	if (parts[i].pavg[0])
		return 0;

	for (rx=-1; rx<2; rx++)
	{
		//reset rndstore, one random can last through 3 locations and reduce rand() calling by up to 6x as much
		rndstore = rand();
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = ((pmap[y+ry][x+rx]&0xFF)==PT_PINV&&parts[pmap[y+ry][x+rx]>>8].life==10)?0:pmap[y+ry][x+rx];
				if (!r)
					continue;

				//spread "being cured" state
				if (((r&0xFF) == PT_VIRS || (r&0xFF) == PT_VRSS || (r&0xFF) == PT_VRSG) && parts[r>>8].pavg[0])
				{
					parts[i].pavg[0] = parts[r>>8].pavg[0] + (((rndstore&0x7)>>1) ? 2:1);
					return 0;
				}
				//soap cures virus
				else if ((r&0xFF) == PT_SOAP)
				{
					parts[i].pavg[0] += 10;
					if (!((rndstore&0x7)>>1))
						kill_part(r>>8);
					return 0;
				}
				else if ((r&0xFF) == PT_PLSM)
				{
					if (surround_space && 10 + (int)(pv[(y+ry)/CELL][(x+rx)/CELL]) > (rand()%100))
					{
						globalSim->part_create(i, x, y, PT_PLSM);
						return 1;
					}
				}
				else if ((r&0xFF) != PT_VIRS && (r&0xFF) != PT_VRSS && (r&0xFF) != PT_VRSG && !(ptypes[r&0xFF].properties&PROP_INDESTRUCTIBLE))
				{
					if (!((rndstore&0xF)>>1))
					{
						parts[r>>8].tmp2 = (r&0xFF);
						parts[r>>8].pavg[0] = 0;
						if (parts[i].pavg[1])
							parts[r>>8].pavg[1] = parts[i].pavg[1] + ((rndstore>>4) ? 1:0);
						else
							parts[r>>8].pavg[1] = 0;
						if (parts[r>>8].temp < 305.0f)
							part_change_type(r>>8,x,y,PT_VRSS);
						else if (parts[r>>8].temp > 673.0f)
							part_change_type(r>>8,x,y,PT_VRSG);
						else
							part_change_type(r>>8,x,y,PT_VIRS);
					}
					rndstore = rndstore >> 5;
				}
				//protons make VIRS last forever
				else if ((photons[y+ry][x+rx]&0xFF) == PT_PROT)
				{
					parts[i].pavg[1] = 0;
				}
			}
	}
	return 0;
}

int VIRS_graphics(GRAPHICS_FUNC_ARGS)
{
	*pixel_mode |= PMODE_BLUR;
	*pixel_mode |= NO_DECO;
	return 1;
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

	elem->Flammable = 0;
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

	elem->DefaultProperties.pavg[1] = 250;

	elem->Update = &VIRS_update;
	elem->Graphics = &VIRS_graphics;
	elem->Init = &VIRS_init_element;
}
