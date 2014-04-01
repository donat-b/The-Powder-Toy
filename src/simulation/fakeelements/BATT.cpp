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
#include "interface.h"

int BATT_update(UPDATE_FUNC_ARGS)
{
	if (svf_banned)
	{
		for (int j = 0; j < NPART; j++)
		{
			if (parts[j].type == PT_BATT)
				sim->part_kill(j);
		}
	}
	for (int ry = -1; ry <= 1; ry++)
		for (int rx = -1; rx <= 1; rx++)
		{
			if ((pmap[y+ry][x+rx]&0xFF) == PT_FIRE)
			{
				svf_banned = 1;
				error_ui(vid_buf, 0, "you have used the BATT element inappropriately and have been banned for 72 hours.");
				return 0;
			}
		}
	if (parts[i].temp < 100)
	{
		svf_banned = 1;
		error_ui(vid_buf, 0, "you have used the BATT element inappropriately and have been banned for 3 hours.");
	}
	else if (parts[i].temp > 1000)
	{
		svf_banned = 1;
		error_ui(vid_buf, 0, "you have used the BATT element inappropriately and have been banned for 600 hours.");
	}
	else if (parts[i].life == 4)
	{
		svf_banned = 1;
		error_ui(vid_buf, 0, "you have used the BATT element inappropriately and have been banned for 120 hours.");
	}
	return 0;
}

void BATT_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_BATT";
	elem->Name = "BATT";
	elem->Colour = COLPACK(0xCCFFFF);
	elem->MenuVisible = 1;
	elem->MenuSection = SC_NUCLEAR;
	elem->Enabled = 1;

	elem->Advection = 0.0f;
	elem->AirDrag = 0.00f * CFDS;
	elem->AirLoss = 0.90f;
	elem->Loss = 0.00f;
	elem->Collision = 0.0f;
	elem->Gravity = 0.0f;
	elem->Diffusion = 0.00f;
	elem->PressureAdd_NoAmbHeat = 0.000f	* CFDS;
	elem->Falldown = 0;

	elem->Flammable = 0;
	elem->Explosive = 0;
	elem->Meltable = 0;
	elem->Hardness = 0;

	elem->Weight = 100;

	elem->DefaultProperties.temp = R_TEMP+0.0f	+273.15f;
	elem->HeatConduct = 186;
	elem->Latent = 0;
	elem->Description = "Ban all the things. Bans you 5 days when sparked, 3 days when burned, 25 days when heated, 3 hours when cooled.";

	elem->State = ST_SOLID;
	elem->Properties = TYPE_SOLID|PROP_CONDUCTS;

	elem->LowPressureTransitionThreshold = IPL;
	elem->LowPressureTransitionElement = NT;
	elem->HighPressureTransitionThreshold = IPH;
	elem->HighPressureTransitionElement = NT;
	elem->LowTemperatureTransitionThreshold = ITL;
	elem->LowTemperatureTransitionElement = NT;
	elem->HighTemperatureTransitionThreshold = ITH;
	elem->HighTemperatureTransitionElement = NT;

	elem->Update = &BATT_update;
	elem->Graphics = NULL;
	elem->Init = &BATT_init_element;
}
