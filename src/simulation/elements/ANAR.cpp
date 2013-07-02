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

int ANAR_update(UPDATE_FUNC_ARGS)
{
        int r, rx, ry;
       
        //if (parts[i].temp >= 0.23)
               // parts[i].temp --;
		for (rx=-1; rx<2; rx++)
				for (ry=-1; ry<2; ry++)
                        if (BOUNDS_CHECK && (rx || ry))
                        {
                                r = pmap[y+ry][x+rx];
                                if (!r)
                                        continue;
                                if ((r&0xFF)==PT_HFLM)
                                {
										if (!(rand()%4))
                                        {
                                                part_change_type(i,x,y,PT_HFLM);
                                                parts[i].life = rand()%150+50;
                                                parts[r>>8].temp = parts[i].temp = 0;
                                                pv[y/CELL][x/CELL] -= 0.5;
                                        }
                                }
                        }
        return 0;
}

void ANAR_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_ANAR";
	elem->Name = "ANAR";
	elem->Colour = COLPACK(0xFFFFEE);
	elem->MenuVisible = 1;
	elem->MenuSection = SC_POWDERS;
	elem->Enabled = 1;

	elem->Advection = -0.7f;
	elem->AirDrag = -0.02f* CFDS;
	elem->AirLoss = 0.96f;
	elem->Loss = 0.80f;
	elem->Collision = 0.1f;
	elem->Gravity = -0.1f;
	elem->Diffusion = 0.00f;
	elem->PressureAdd_NoAmbHeat = 0.000f	* CFDS;
	elem->Falldown = 1;

	elem->Flammable = 0;
	elem->Explosive = 0;
	elem->Meltable = 0;
	elem->Hardness = 30;

	elem->Weight = 85;

	elem->CreationTemperature = R_TEMP+0.0f	+273.15f;
	elem->HeatConduct = 70;
	elem->Latent = 0;
	elem->Description = "Anti-air. Very light dust, which behaves opposite gravity.";

	elem->State = ST_SOLID;
	elem->Properties = TYPE_PART;

	elem->LowPressureTransitionThreshold = IPL;
	elem->LowPressureTransitionElement = NT;
	elem->HighPressureTransitionThreshold = IPH;
	elem->HighPressureTransitionElement = NT;
	elem->LowTemperatureTransitionThreshold = ITL;
	elem->LowTemperatureTransitionElement = NT;
	elem->HighTemperatureTransitionThreshold = ITH;
	elem->HighTemperatureTransitionElement = NT;

	elem->Update = &ANAR_update;
	elem->Graphics = NULL;
}
