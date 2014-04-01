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

int DIMT_update(UPDATE_FUNC_ARGS)
{
	if (parts[i].life)
		parts[i].tmp2 = -1;
	else if (parts[i].tmp2 > 0)
	{
		if (parts[i].tmp2 > 1000 && rand()%2)
			sim->part_change_type(i, x, y, PT_DEST);
		else
			parts[i].tmp2--;
		if (parts[i].tmp2 == 0 || parts[i].tmp2 < -1)
			sim->part_kill(i);
	}
	return 0;
}

void DIMT_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_DIMT";
	elem->Name = "DIMT";
	elem->Colour = COLPACK(0x404060);
	elem->MenuVisible = 1;
	elem->MenuSection = SC_ELEC;
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
	elem->Meltable = 1;
	elem->Hardness = 1;

	elem->Weight = 100;

	elem->DefaultProperties.temp = R_TEMP+0.0f	+273.15f;
	elem->DefaultProperties.tmp2 = 1000;
	elem->HeatConduct = 251;
	elem->Latent = 919;
	elem->Description = "Disappearing metal. Disappears if not sparked for 20 seconds.";

	elem->State = ST_SOLID;
	elem->Properties = TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_HOT_GLOW;

	elem->LowPressureTransitionThreshold = IPL;
	elem->LowPressureTransitionElement = NT;
	elem->HighPressureTransitionThreshold = IPH;
	elem->HighPressureTransitionElement = NT;
	elem->LowTemperatureTransitionThreshold = ITL;
	elem->LowTemperatureTransitionElement = NT;
	elem->HighTemperatureTransitionThreshold = 1273.0f;
	elem->HighTemperatureTransitionElement = PT_LAVA;

	elem->Update = &DIMT_update;
	elem->Graphics = NULL;
	elem->Init = &DIMT_init_element;
}
