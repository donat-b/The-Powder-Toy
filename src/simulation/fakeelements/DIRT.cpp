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

int DIRT_update(UPDATE_FUNC_ARGS)
{
	if (!pmap[y+1][x-2] || !pmap[y+1][x-1] || !pmap[y+1][x] || !pmap[y+1][x+1] || !pmap[y+1][x+2])
		parts[i].vx = rand()%3-1;
	return 0;
}

void DIRT_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_DIRT";
	elem->Name = "DIRT";
	elem->Colour = COLPACK(0xB2751A);
	elem->MenuVisible = 1;
	elem->MenuSection = SC_POWDERS;
	elem->Enabled = 1;

	elem->Advection = 0.4f;
	elem->AirDrag = 0.02f * CFDS;
	elem->AirLoss = 0.96f;
	elem->Loss = 0.97f;
	elem->Collision = 0.0f;
	elem->Gravity = 0.2f;
	elem->Diffusion = 0.00f;
	elem->PressureAdd_NoAmbHeat = 0.000f	* CFDS;
	elem->Falldown = 1;

	elem->Flammable = 10;
	elem->Explosive = 0;
	elem->Meltable = 0;
	elem->Hardness = 30;
	elem->PhotonReflectWavelengths = 0x3FFFFFC0;

	elem->Weight = 85;

	elem->DefaultProperties.temp = R_TEMP+0.0f	+273.15f;
	elem->HeatConduct = 70;
	elem->Latent = 0;
	elem->Description = "Dirt. Can be used to simulate dirt.";

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

	elem->Update = &DIRT_update;
	elem->Graphics = NULL;
	elem->Init = &DIRT_init_element;
}
