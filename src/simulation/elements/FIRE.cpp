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
#include "graphics.h"

int FIRE_update_legacy(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, rt, lpv, t = parts[i].type;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (bmap[(y+ry)/CELL][(x+rx)/CELL] && bmap[(y+ry)/CELL][(x+rx)/CELL]!=WL_STREAM)
					continue;
				rt = r&0xFF;
				lpv = (int)pv[(y+ry)/CELL][(x+rx)/CELL];
				if (lpv < 1) lpv = 1;
				if (ptypes[rt].meltable && ((rt!=PT_RBDM && rt!=PT_LRBD) || t!=PT_SPRK) && ((t!=PT_FIRE&&t!=PT_PLSM) || (rt!=PT_METL && rt!=PT_IRON && rt!=PT_ETRD && rt!=PT_PSCN && rt!=PT_NSCN && rt!=PT_NTCT && rt!=PT_PTCT && rt!=PT_BMTL && rt!=PT_BRMT && rt!=PT_SALT && rt!=PT_INWR)) && ptypes[rt].meltable*lpv>(rand()%1000))
				{
					if (t!=PT_LAVA || parts[i].life>0)
					{
						if (rt==PT_BRMT)
							parts[r>>8].ctype = PT_BMTL;
						else if (rt==PT_SAND)
							parts[r>>8].ctype = PT_GLAS;
						else
							parts[r>>8].ctype = rt;
						part_change_type(r>>8,x+rx,y+ry,PT_LAVA);
						parts[r>>8].life = rand()%120+240;
					}
					else
					{
						parts[i].life = 0;
						parts[i].ctype = PT_NONE;//rt;
						part_change_type(i,x,y,(parts[i].ctype)?parts[i].ctype:PT_STNE);
						return 1;
					}
				}
				if (rt==PT_ICEI || rt==PT_SNOW)
				{
					parts[r>>8].type = PT_WATR;
					if (t==PT_FIRE)
					{
						kill_part(i);
						return 1;
					}
					if (t==PT_LAVA)
					{
						parts[i].life = 0;
						part_change_type(i,x,y,PT_STNE);
					}
				}
				if (rt==PT_WATR || rt==PT_DSTW || rt==PT_SLTW)
				{
					kill_part(r>>8);
					if (t==PT_FIRE)
					{
						kill_part(i);
						return 1;
					}
					if (t==PT_LAVA)
					{
						parts[i].life = 0;
						parts[i].ctype = PT_NONE;
						part_change_type(i,x,y,(parts[i].ctype)?parts[i].ctype:PT_STNE);
					}
				}
			}
	return 0;
}

int FIRE_update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, rt, t = parts[i].type;
	switch (t)
	{
	case PT_PLSM:
		if (parts[i].life <= 1)
		{
			if (parts[i].ctype == PT_NBLE)
			{
				t = PT_NBLE;
				part_change_type(i, x, y, t);
				parts[i].life = 0;
			}
			else if ((parts[i].tmp&0x3) == 3)
			{
				part_change_type(i, x, y, PT_DSTW);
				parts[i].life = 0;
				parts[i].ctype = PT_FIRE;
			}
		}
		break;
	case PT_FIRE:
		if (parts[i].life <=1)
		{
			if ((parts[i].tmp&0x3) == 3)
			{
				part_change_type(i, x, y, PT_DSTW);
				parts[i].life = 0;
				parts[i].ctype = PT_FIRE;
			}
			else if (parts[i].temp<625)
			{
				part_change_type(i, x, y, PT_SMKE);
				parts[i].life = rand()%20+250;
			}
		}
		break;
	default:
		break;
	}
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				rt = r&0xFF;
				//THRM burning
				if (rt==PT_THRM && (t==PT_FIRE || t==PT_PLSM || t==PT_LAVA))
				{
					if (!(rand()%500)) {
						part_change_type(r>>8,x+rx,y+ry,PT_LAVA);
						parts[r>>8].ctype = PT_BMTL;
						parts[r>>8].temp = 3500.0f;
						pv[(y+ry)/CELL][(x+rx)/CELL] += 50.0f;
					} else {
						part_change_type(r>>8,x+rx,y+ry,PT_LAVA);
						parts[r>>8].life = 400;
						parts[r>>8].ctype = PT_THRM;
						parts[r>>8].temp = 3500.0f;
						parts[r>>8].tmp = 20;
					}
					continue;
				}

				if ((rt==PT_COAL) || (rt==PT_BCOL))
				{
					if ((t==PT_FIRE || t==PT_PLSM))
					{
						if (parts[r>>8].life>100 && !(rand()%500))
						{
							parts[r>>8].life = 99;
						}
					}
					else if (t==PT_LAVA)
					{
						if (parts[i].ctype == PT_IRON && !(rand()%500))
						{
							parts[i].ctype = PT_METL;
							kill_part(r>>8);
						}
					}
				}
				if ((surround_space || ptypes[rt].explosive) &&
					ptypes[rt].flammable && (ptypes[rt].flammable + (int)(pv[(y+ry)/CELL][(x+rx)/CELL]*10.0f)) > (rand()%1000) &&
					//exceptions, t is the thing causing the flame and rt is what's burning
					(t != PT_SPRK || (rt != PT_RBDM && rt != PT_LRBD && rt != PT_INSL)) &&
					(t != PT_PHOT || rt != PT_INSL) &&
					(rt != PT_SPNG || parts[r>>8].life == 0))
				{
					part_change_type(r>>8, x+rx, y+ry, PT_FIRE);
					parts[r>>8].temp = restrict_flt(ptypes[PT_FIRE].heat + (ptypes[rt].flammable/2), MIN_TEMP, MAX_TEMP);
					parts[r>>8].life = rand()%80+180;
					parts[r>>8].tmp = parts[r>>8].ctype = 0;
					if (ptypes[rt].explosive)
						pv[y/CELL][x/CELL] += 0.25f * CFDS;
				}
			}
	if (legacy_enable && t!=PT_SPRK) // SPRK has no legacy reactions
		FIRE_update_legacy(UPDATE_FUNC_SUBCALL_ARGS);
	return 0;
}

int FIRE_graphics(GRAPHICS_FUNC_ARGS)
{
	int caddress = (int)restrict_flt(restrict_flt((float)cpart->life, 0.0f, 200.0f)*3, 0.0f, (200.0f*3)-3);
	*colr = (unsigned char)flm_data[caddress];
	*colg = (unsigned char)flm_data[caddress+1];
	*colb = (unsigned char)flm_data[caddress+2];
	
	*firea = 255;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;
	
	*pixel_mode = PMODE_NONE; //Clear default, don't draw pixel
	*pixel_mode |= FIRE_ADD;
	//Returning 0 means dynamic, do not cache
	return 0;
}

void FIRE_create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].life = rand()%50+120;
}

void FIRE_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_FIRE";
	elem->Name = "FIRE";
	elem->Colour = COLPACK(0xFF1000);
	elem->MenuVisible = 1;
	elem->MenuSection = SC_EXPLOSIVE;
	elem->Enabled = 1;

	elem->Advection = 0.9f;
	elem->AirDrag = 0.04f * CFDS;
	elem->AirLoss = 0.97f;
	elem->Loss = 0.20f;
	elem->Collision = 0.0f;
	elem->Gravity = -0.1f;
	elem->Diffusion = 0.00f;
	elem->HotAir = 0.001f	* CFDS;
	elem->Falldown = 0;

	elem->Flammable = 0;
	elem->Explosive = 0;
	elem->Meltable = 0;
	elem->Hardness = 1;

	elem->Weight = 2;

	elem->DefaultProperties.temp = R_TEMP+400.0f+273.15f;
	elem->HeatConduct = 88;
	elem->Latent = 0;
	elem->Description = "Ignites flammable materials. Heats air.";

	elem->State = ST_GAS;
	elem->Properties = TYPE_GAS|PROP_LIFE_DEC|PROP_LIFE_KILL;

	elem->LowPressureTransitionThreshold = IPL;
	elem->LowPressureTransitionElement = NT;
	elem->HighPressureTransitionThreshold = IPH;
	elem->HighPressureTransitionElement = NT;
	elem->LowTemperatureTransitionThreshold = ITL;
	elem->LowTemperatureTransitionElement = NT;
	elem->HighTemperatureTransitionThreshold = 2773.0f;
	elem->HighTemperatureTransitionElement = PT_PLSM;

	elem->Update = &FIRE_update;
	elem->Graphics = &FIRE_graphics;
	elem->Func_Create = &FIRE_create;
	elem->Init = &FIRE_init_element;
}
