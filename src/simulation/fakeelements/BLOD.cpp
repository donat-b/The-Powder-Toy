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

int BLOD_update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, nr, ng, nb, na;
	float tr, tg, tb, ta, mr, mg, mb, ma;
	float blend;
	unsigned int color = 0xFFFF0000;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)!=PT_BLOD)
				{
					blend = 0.95f;
					tr = (float)((color>>16)&0xFF);
					tg = (float)((color>>8)&0xFF);
					tb = (float)(color&0xFF);
					ta = (float)((color>>24)&0xFF);

					mr = (float)((color>>16)&0xFF);
					mg = (float)((color>>8)&0xFF);
					mb = (float)(color&0xFF);
					ma = (float)((color>>24)&0xFF);

					nr = (int)((tr*blend) + (mr*(1-blend)));
					ng = (int)((tg*blend) + (mg*(1-blend)));
					nb = (int)((tb*blend) + (mb*(1-blend)));
					na = (int)((ta*blend) + (ma*(1-blend)));

					parts[r>>8].dcolour = nr<<16 | ng<<8 | nb | na<<24;
				}
			}
	return 0;
}

void BLOD_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_BLOD";
	elem->Name = "BLOD";
	elem->Colour = COLPACK(0xFE3D05);
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
	elem->Hardness = 1;

	elem->Weight = 30;

	elem->DefaultProperties.temp = R_TEMP+273.15f;
	elem->HeatConduct = 13;
	elem->Latent = 0;
	elem->Description = "Blood. Released from stickmen upon death.";

	elem->State = ST_LIQUID;
	elem->Properties = TYPE_LIQUID;

	elem->LowPressureTransitionThreshold = IPL;
	elem->LowPressureTransitionElement = NT;
	elem->HighPressureTransitionThreshold = IPH;
	elem->HighPressureTransitionElement = NT;
	elem->LowTemperatureTransitionThreshold = ITL;
	elem->LowTemperatureTransitionElement = NT;
	elem->HighTemperatureTransitionThreshold = ITH;
	elem->HighTemperatureTransitionElement = NT;

	elem->Update = &BLOD_update;
	elem->Graphics = NULL;
	elem->Init = &BLOD_init_element;
}
