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

int VIRS_update(UPDATE_FUNC_ARGS);

void VRSG_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_VRSG";
	elem->Name = "VRSG";
	elem->Colour = COLPACK(0xFE68FE);
	elem->MenuVisible = 0;
	elem->MenuSection = SC_GAS;
	elem->Enabled = 1;

	elem->Advection = 1.0f;
	elem->AirDrag = 0.01f * CFDS;
	elem->AirLoss = 0.99f;
	elem->Loss = 0.30f;
	elem->Collision = -0.1f;
	elem->Gravity = 0.0f;
	elem->Diffusion = 0.75f;
	elem->PressureAdd_NoAmbHeat = 0.000f	* CFDS;
	elem->Falldown = 0;

	elem->Flammable = 500;
	elem->Explosive = 0;
	elem->Meltable = 0;
	elem->Hardness = 0;

	elem->Weight = 1;

	elem->CreationTemperature = R_TEMP+500.0f+273.15f;
	elem->HeatConduct = 251;
	elem->Latent = 0;
	elem->Description = "Gas Virus. Turns everything it touches into virus.";

	elem->State = ST_GAS;
	elem->Properties = TYPE_GAS;

	elem->LowPressureTransitionThreshold = IPL;
	elem->LowPressureTransitionElement = NT;
	elem->HighPressureTransitionThreshold = IPH;
	elem->HighPressureTransitionElement = NT;
	elem->LowTemperatureTransitionThreshold = 673.0f;
	elem->LowTemperatureTransitionElement = PT_VRSG;
	elem->HighTemperatureTransitionThreshold = ITH;
	elem->HighTemperatureTransitionElement = NT;

	elem->DefaultProperties.pavg[1] = 100;

	elem->Update = &VIRS_update;
	elem->Graphics = NULL;
}
