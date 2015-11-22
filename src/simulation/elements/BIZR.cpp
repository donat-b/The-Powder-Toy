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

#define BLEND 0.95f

int BIZR_update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, nr, ng, nb, na;
	float tr, tg, tb, ta, mr, mg, mb, ma;
	if (parts[i].dcolour)
	{
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)!=PT_BIZR && (r&0xFF)!=PT_BIZRG  && (r&0xFF)!=PT_BIZRS)
					{
						ta = (float)COLA(parts[r>>8].dcolour);
						tr = (float)COLR(parts[r>>8].dcolour);
						tg = (float)COLG(parts[r>>8].dcolour);
						tb = (float)COLB(parts[r>>8].dcolour);

						ma = (float)COLA(parts[i].dcolour);
						mr = (float)COLR(parts[i].dcolour);
						mg = (float)COLG(parts[i].dcolour);
						mb = (float)COLB(parts[i].dcolour);
						
						nr = (int)((tr*BLEND) + (mr*(1 - BLEND)));
						ng = (int)((tg*BLEND) + (mg*(1 - BLEND)));
						nb = (int)((tb*BLEND) + (mb*(1 - BLEND)));
						na = (int)((ta*BLEND) + (ma*(1 - BLEND)));
						
						parts[r>>8].dcolour = COLARGB(na, nr, ng, nb);
					}
				}
	}
	return 0;
}

int BIZR_graphics(GRAPHICS_FUNC_ARGS)
{
	int x = 0;
	float brightness = fabs(cpart->vx) + fabs(cpart->vy);
	if (cpart->ctype&0x3FFFFFFF)
	{
		*colg = 0;
		*colb = 0;
		*colr = 0;
		for (x=0; x<12; x++) {
			*colr += (cpart->ctype >> (x+18)) & 1;
			*colb += (cpart->ctype >>  x)     & 1;
		}
		for (x=0; x<12; x++)
			*colg += (cpart->ctype >> (x+9))  & 1;
		x = 624 / (*colr + *colg + *colb + 1);
		*colr *= x;
		*colg *= x;
		*colb *= x;
	}
	if (brightness > 0)
	{
		brightness /= 5;
		*firea = 255;
		*firer = (int)(*colr * brightness);
		*fireg = (int)(*colg * brightness);
		*fireb = (int)(*colb * brightness);
		*pixel_mode |= FIRE_ADD;
	}
	*pixel_mode |= PMODE_BLUR;
	return 0;
}

void BIZR_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_BIZR";
	elem->Name = "BIZR";
	elem->Colour = COLPACK(0x00FF77);
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
	elem->HotAir = 0.000f	* CFDS;
	elem->Falldown = 2;

	elem->Flammable = 0;
	elem->Explosive = 0;
	elem->Meltable = 0;
	elem->Hardness = 20;

	elem->Weight = 30;

	elem->DefaultProperties.temp = R_TEMP+0.0f	+273.15f;
	elem->HeatConduct = 29;
	elem->Latent = 0;
	elem->Description = "Bizarre... contradicts the normal state changes. Paints other elements with its deco color.";

	elem->State = ST_LIQUID;
	elem->Properties = TYPE_LIQUID;

	elem->LowPressureTransitionThreshold = IPL;
	elem->LowPressureTransitionElement = NT;
	elem->HighPressureTransitionThreshold = IPH;
	elem->HighPressureTransitionElement = NT;
	elem->LowTemperatureTransitionThreshold = 100.0f;
	elem->LowTemperatureTransitionElement = PT_BIZRG;
	elem->HighTemperatureTransitionThreshold = 400.0f;
	elem->HighTemperatureTransitionElement = PT_BIZRS;

	elem->DefaultProperties.ctype = 0x47FFFF;

	elem->Update = &BIZR_update;
	elem->Graphics = &BIZR_graphics;
	elem->Init = &BIZR_init_element;
}
