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

int PCLN_update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, rt;
	if (parts[i].ctype<=0 || parts[i].ctype>=PT_NUM || !ptypes[parts[i].ctype].enabled || (parts[i].ctype==PT_LIFE && (parts[i].tmp<0 || parts[i].tmp>=NGOL)))
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK)
				{
					r = photons[y+ry][x+rx];
					if (!r)
						r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					rt = r&0xFF;
					if (!(ptypes[rt].properties&PROP_CLONE) &&
						!(ptypes[rt].properties&PROP_BREAKABLECLONE) &&
				        rt != PT_SPRK && rt != PT_NSCN && 
						rt != PT_PSCN && rt != PT_STKM && 
						rt != PT_STKM2)
					{
						parts[i].ctype = r&0xFF;
						if (rt==PT_LIFE || rt==PT_LAVA)
							parts[i].tmp = parts[r>>8].ctype;
					}
				}
	if (parts[i].ctype>0 && parts[i].ctype<PT_NUM && ptypes[parts[i].ctype].enabled && parts[i].life==10)
	{
		//create photons a different way
		if (parts[i].ctype==PT_PHOT)
		{
			for (rx=-1; rx<2; rx++)
				for (ry=-1; ry<2; ry++)
					if (rx || ry)
					{
						int r = sim->part_create(-1, x+rx, y+ry, parts[i].ctype);
						if (r != -1)
						{
							parts[r].vx = rx*3.0f;
							parts[r].vy = ry*3.0f;
							if (r>i)
							{
								// Make sure movement doesn't happen until next frame, to avoid gaps in the beams of photons produced
								parts[r].flags |= FLAG_SKIPMOVE;
							}
						}
					}
		}
		//create life a different way
		else if (parts[i].ctype==PT_LIFE)
		{
			for (rx=-1; rx<2; rx++)
				for (ry=-1; ry<2; ry++)
				{
					// TODO: change this create_part
					create_part(-1, x+rx, y+ry, parts[i].ctype|(parts[i].tmp<<8));
				}
		}
		else if (parts[i].ctype!=PT_LIGH || !(rand()%30))
		{
			int np = sim->part_create(-1, x+rand()%3-1, y+rand()%3-1, parts[i].ctype);
			if (np>=0)
			{
				if (parts[i].ctype==PT_LAVA && parts[i].tmp>0 && parts[i].tmp<PT_NUM && ptransitions[parts[i].tmp].tht==PT_LAVA)
					parts[np].ctype = parts[i].tmp;
			}
		}
	}
	return 0;
}

int PCLN_graphics(GRAPHICS_FUNC_ARGS)
{
	int lifemod = ((cpart->life>10?10:cpart->life)*10);
	*colr += lifemod;
	*colg += lifemod;
	return 0;
}

void PCLN_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_PCLN";
	elem->Name = "PCLN";
	elem->Colour = COLPACK(0x3B3B0A);
	elem->MenuVisible = 1;
	elem->MenuSection = SC_POWERED;
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
	elem->Description = "Powered clone. When activated, duplicates any particles it touches.";

	elem->State = ST_NONE;
	elem->Properties = TYPE_SOLID|PROP_CLONE|PROP_POWERED|PROP_NOCTYPEDRAW;

	elem->LowPressureTransitionThreshold = IPL;
	elem->LowPressureTransitionElement = NT;
	elem->HighPressureTransitionThreshold = IPH;
	elem->HighPressureTransitionElement = NT;
	elem->LowTemperatureTransitionThreshold = ITL;
	elem->LowTemperatureTransitionElement = NT;
	elem->HighTemperatureTransitionThreshold = ITH;
	elem->HighTemperatureTransitionElement = NT;

	elem->Update = NULL;
	elem->Graphics = &PCLN_graphics;
	elem->Init = &PCLN_init_element;
}
