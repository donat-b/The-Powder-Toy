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

int ELEC_update(UPDATE_FUNC_ARGS)
{
	int r, rt, rx, ry, nb, rrx, rry;
	for (rx=-2; rx<=2; rx++)
		for (ry=-2; ry<=2; ry++)
			if (BOUNDS_CHECK) {
				r = pmap[y+ry][x+rx];
				if (!r)
					r = photons[y+ry][x+rx];
				if (!r)
					continue;
				rt = r&0xFF;
				switch (r&0xFF)
				{
				case PT_GLAS:
					for (rrx=-1; rrx<=1; rrx++)
						for (rry=-1; rry<=1; rry++)
							if (x+rx+rrx>=0 && y+ry+rry>=0 && x+rx+rrx<XRES && y+ry+rry<YRES)
							{
								nb = sim->part_create(-1, x+rx+rrx, y+ry+rry, PT_EMBR);
								if (nb!=-1)
								{
									parts[nb].tmp = 0;
									parts[nb].life = 50;
									parts[nb].temp = parts[i].temp*0.8f;
									parts[nb].vx = (float)(rand()%20-10);
									parts[nb].vy = (float)(rand()%20-10);
								}
							}
					sim->part_kill(i);
					return 1;
				case PT_LCRY:
					parts[r>>8].tmp2 = 5+rand()%5;
					break;
				case PT_WATR:
				case PT_DSTW:
				case PT_SLTW:
				case PT_CBNW:
					if (!(rand()%3))
					{
						sim->part_create(r>>8, x+rx, y+ry, PT_O2);
					}
					else
					{
						sim->part_create(r>>8, x+rx, y+ry, PT_H2);
					}
					sim->part_kill(i);
					return 1;
				case PT_PROT:
					if (parts[r>>8].tmp2&0x1)
						continue;
				case PT_NEUT:
					sim->part_change_type(r>>8, x+rx, y+ry, PT_H2);
					parts[r>>8].life = 0;
					parts[r>>8].ctype = 0;
					sim->part_kill(i);
					break;
				case PT_DEUT:
					if (parts[r>>8].life < 6000)
						parts[r>>8].life += 1;
					parts[r>>8].temp = 0;
					sim->part_kill(i);
					return 1;
				case PT_EXOT:
					parts[r>>8].tmp2 += 5;
					parts[r>>8].life = 1000;
					break;
				case PT_NONE: //seems to speed up ELEC even if it isn't used
					break;
				default:
					if (sim->elements[rt].Properties & PROP_CONDUCTS && (rt!=PT_NBLE || parts[i].temp<2273.15))
					{
						sim->spark_conductive_attempt(r>>8, x+rx, y+ry);
						sim->part_kill(i);
						return 1;
					}
					break;
				}
			}
	return 0;
}

int ELEC_graphics(GRAPHICS_FUNC_ARGS)
{
	*firea = 70;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;

	*pixel_mode |= FIRE_ADD;
	return 0;
}

void ELEC_create(ELEMENT_CREATE_FUNC_ARGS)
{
	float a = (rand()%360)*3.14159f/180.0f;
	sim->parts[i].life = 680;
	sim->parts[i].vx = 2.0f*cosf(a);
	sim->parts[i].vy = 2.0f*sinf(a);
}

void ELEC_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_ELEC";
	elem->Name = "ELEC";
	elem->Colour = COLPACK(0xDFEFFF);
	elem->MenuVisible = 1;
	elem->MenuSection = SC_NUCLEAR;
	elem->Enabled = 1;

	elem->Advection = 0.0f;
	elem->AirDrag = 0.00f * CFDS;
	elem->AirLoss = 1.00f;
	elem->Loss = 1.00f;
	elem->Collision = -0.99f;
	elem->Gravity = 0.0f;
	elem->Diffusion = 0.00f;
	elem->HotAir = 0.000f	* CFDS;
	elem->Falldown = 0;

	elem->Flammable = 0;
	elem->Explosive = 0;
	elem->Meltable = 0;
	elem->Hardness = 0;

	elem->Weight = -1;

	elem->DefaultProperties.temp = R_TEMP+200.0f+273.15f;
	elem->HeatConduct = 251;
	elem->Latent = 0;
	elem->Description = "Electrons. Sparks electronics, reacts with NEUT and WATR.";

	elem->State = ST_GAS;
	elem->Properties = TYPE_ENERGY|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC;

	elem->LowPressureTransitionThreshold = IPL;
	elem->LowPressureTransitionElement = NT;
	elem->HighPressureTransitionThreshold = IPH;
	elem->HighPressureTransitionElement = NT;
	elem->LowTemperatureTransitionThreshold = ITL;
	elem->LowTemperatureTransitionElement = NT;
	elem->HighTemperatureTransitionThreshold = ITH;
	elem->HighTemperatureTransitionElement = NT;

	elem->Update = &ELEC_update;
	elem->Graphics = &ELEC_graphics;
	elem->Func_Create = &ELEC_create;
	elem->Init = &ELEC_init_element;
}
