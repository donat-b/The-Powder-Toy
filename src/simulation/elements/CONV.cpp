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

int CONV_update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	int ctype = parts[i].ctype&0xFF, ctypeExtra = parts[i].ctype>>8;
	if (ctype<=0 || ctype>=PT_NUM || !ptypes[ctype].enabled || ctype==PT_CONV || (ctype==PT_LIFE && (ctypeExtra<0 || ctypeExtra>=NGOL)))
	{
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK)
				{
					r = photons[y+ry][x+rx];
					if (!r)
						r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (!(ptypes[r&0xFF].properties&PROP_CLONE) &&
						!(ptypes[r&0xFF].properties&PROP_BREAKABLECLONE) &&
				        (r&0xFF)!=PT_STKM && (r&0xFF)!=PT_STKM2 && 
						(r&0xFF)!=PT_CONV && (r&0xFF)<PT_NUM)
					{
						parts[i].ctype = r&0xFF;
						if ((r&0xFF)==PT_LIFE)
							parts[i].ctype |= (parts[r>>8].ctype << 8);
					}
				}
	}
	else
	{
		int restrictElement = sim->IsElement(parts[i].tmp) ? parts[i].tmp : 0;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK)
				{
					r = photons[y+ry][x+rx];
					if (!r || (restrictElement && (r&0xFF)!=restrictElement))
						r = pmap[y+ry][x+rx];
					if (!r || (restrictElement && (r&0xFF)!=restrictElement))
						continue;
					if((r&0xFF)!=PT_CONV && !(ptypes[r&0xFF].properties&PROP_INDESTRUCTIBLE) && (r&0xFF)!=ctype)
					{
						sim->part_create(r>>8, x+rx, y+ry, ctype, ctypeExtra);
					}
				}
	}
	return 0;
}

void CONV_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_CONV";
	elem->Name = "CONV";
	elem->Colour = COLPACK(0x0AAB0A);
	elem->MenuVisible = 1;
	elem->MenuSection = SC_SPECIAL;
	elem->Enabled = 1;

	elem->Advection = 0.0f;
	elem->AirDrag = 0.00f * CFDS;
	elem->AirLoss = 0.90f;
	elem->Loss = 0.00f;
	elem->Collision = 0.0f;
	elem->Gravity = 0.0f;
	elem->Diffusion = 0.00f;
	elem->HotAir = 0.000f	* CFDS;
	elem->Falldown = 0;

	elem->Flammable = 0;
	elem->Explosive = 0;
	elem->Meltable = 0;
	elem->Hardness = 1;

	elem->Weight = 100;

	elem->DefaultProperties.temp = R_TEMP+0.0f	+273.15f;
	elem->HeatConduct = 251;
	elem->Latent = 0;
	elem->Description = "Solid. Converts everything into whatever it first touches.";

	elem->State = ST_NONE;
	elem->Properties = TYPE_SOLID|PROP_DRAWONCTYPE|PROP_NOCTYPEDRAW;

	elem->LowPressureTransitionThreshold = IPL;
	elem->LowPressureTransitionElement = NT;
	elem->HighPressureTransitionThreshold = IPH;
	elem->HighPressureTransitionElement = NT;
	elem->LowTemperatureTransitionThreshold = ITL;
	elem->LowTemperatureTransitionElement = NT;
	elem->HighTemperatureTransitionThreshold = ITH;
	elem->HighTemperatureTransitionElement = NT;

	elem->Update = &CONV_update;
	elem->Graphics = NULL;
	elem->Init = &CONV_init_element;
}
