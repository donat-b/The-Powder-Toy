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

int DeutExplosion(Simulation *sim, int n, int x, int y, float temp, int t)
{
	int i, c;
	n = (n/50);
	if (n<1) {
		n = 1;
	}
	if (n>340) {
		n = 340;
	}
	for (c=0; c<n; c++)
	{
		i = sim->part_create(-3, x, y, t);
		if (i > -1)
			parts[i].temp = temp;
		pv[y/CELL][x/CELL] += 6.0f * CFDS;
	}
	return 0;
}

int FIRE_update(UPDATE_FUNC_ARGS);

int NEUT_update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	int pressureFactor = 3 + (int)pv[y/CELL][x/CELL];
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK)
			{
				r = pmap[y+ry][x+rx];
				switch (r&0xFF)
				{
				case PT_WATR:
					if (3>(rand()%20))
						part_change_type(r>>8, x+rx, y+ry, PT_DSTW);
					//no break
				case PT_ICEI:
				case PT_SNOW:
					parts[i].vx *= 0.995f;
					parts[i].vy *= 0.995f;
					break;
				case PT_PLUT:
					if (pressureFactor>(rand()%1000))
					{
						if (!(rand()%3))
						{
							sim->part_create(r>>8, x+rx, y+ry, rand()%3 ? PT_LAVA : PT_URAN);
							parts[r>>8].temp = MAX_TEMP;
							if (parts[r>>8].type==PT_LAVA) {
								parts[r>>8].tmp = 100;
								parts[r>>8].ctype = PT_PLUT;
							}
						}
						else
						{
							sim->part_create(r>>8, x+rx, y+ry, PT_NEUT);
							parts[r>>8].vx = 0.25f*parts[r>>8].vx + parts[i].vx;
							parts[r>>8].vy = 0.25f*parts[r>>8].vy + parts[i].vy;
						}
						pv[y/CELL][x/CELL] += 10.0f * CFDS; //Used to be 2, some people said nukes weren't powerful enough
						FIRE_update(UPDATE_FUNC_SUBCALL_ARGS);
					}
					break;
#ifdef SDEUT
				case PT_DEUT:
					if (pressureFactor+1+(parts[r>>8].life/100) > rand()%1000)
					{
						DeutExplosion(sim, parts[r>>8].life, x+rx, y+ry, restrict_flt(parts[r>>8].temp + parts[r>>8].life*500.0f, MIN_TEMP, MAX_TEMP), PT_NEUT);
						kill_part(r>>8);
					}
					break;
#else
				case PT_DEUT:
					if (pressureFactor+1 > rand()%1000)
					{
						sim->part_create(r>>8, x+rx, y+ry, PT_NEUT);
						parts[r>>8].vx = 0.25f*parts[r>>8].vx + parts[i].vx;
						parts[r>>8].vy = 0.25f*parts[r>>8].vy + parts[i].vy;
						if (parts[r>>8].life>0)
						{
							parts[r>>8].life --;
							parts[r>>8].temp = restrict_flt(parts[r>>8].temp + parts[r>>8].life*17.0f, MIN_TEMP, MAX_TEMP);
							pv[y/CELL][x/CELL] += 6.0f * CFDS;
						}
						else
							kill_part(r>>8);
					}
					break;
#endif
				case PT_GUNP:
					if (3>(rand()%200))
						part_change_type(r>>8, x+rx, y+ry, PT_DUST);
					break;
				case PT_DYST:
					if (3>(rand()%200))
						part_change_type(r>>8, x+rx, y+ry, PT_YEST);
					break;
				case PT_YEST:
					part_change_type(r>>8, x+rx, y+ry, PT_DYST);
					break;
				case PT_PLEX:
					if (3>(rand()%200))
						part_change_type(r>>8, x+rx, y+ry, PT_GOO);
					break;
				case PT_NITR:
					if (3>(rand()%200))
						part_change_type(r>>8, x+rx, y+ry, PT_DESL);
					break;
				case PT_PLNT:
					if (!(rand()%20))
						sim->part_create(r>>8, x+rx, y+ry, PT_WOOD);
					break;
				case PT_DESL:
				case PT_OIL:
					if (3>(rand()%200))
						part_change_type(r>>8, x+rx, y+ry, PT_GAS);
					break;
				case PT_COAL:
					if (!(rand()%20))
						sim->part_create(r>>8, x+rx, y+ry, PT_WOOD);
					break;
				case PT_DUST:
					if (!(rand()%20))
						part_change_type(r>>8, x+rx, y+ry, PT_FWRK);
					break;
				case PT_EMBR:
					if (parts[i].tmp == 1 && !(rand()%20))
						part_change_type(r>>8, x+rx, y+ry, PT_FWRK);
					break;
				case PT_FWRK:
					if (!(rand()%20))
						parts[r>>8].ctype = PT_DUST;
					break;
				case PT_ACID:
					if (!(rand()%20))
						sim->part_create(r>>8, x+rx, y+ry, PT_ISOZ);
					break;
				case PT_TTAN:
					if (!(rand()%20))
					{
						kill_part(i);
						return 1;
					}
					break;
				case PT_EXOT:
					if (5>(rand()%100))
						parts[r>>8].life = 1500;
					break;
				default:
					break;
				}
			}
	return 0;
}

int NEUT_graphics(GRAPHICS_FUNC_ARGS)
{
	*firea = 120;
	*firer = 10;
	*fireg = 80;
	*fireb = 120;

	*pixel_mode &= ~PMODE_FLAT;
	*pixel_mode |= FIRE_ADD | PMODE_ADD;
	return 1;
}

void NEUT_create(ELEMENT_CREATE_FUNC_ARGS)
{
	float r = (rand()%128+128)/127.0f;
	float a = (rand()%360)*3.14159f/180.0f;
	sim->parts[i].life = rand()%480+480;
	sim->parts[i].vx = r*cosf(a);
	sim->parts[i].vy = r*sinf(a);
}

void NEUT_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_NEUT";
	elem->Name = "NEUT";
	elem->Colour = COLPACK(0x20E0FF);
	elem->MenuVisible = 1;
	elem->MenuSection = SC_NUCLEAR;
	elem->Enabled = 1;

	elem->Advection = 0.0f;
	elem->AirDrag = 0.00f * CFDS;
	elem->AirLoss = 1.00f;
	elem->Loss = 1.00f;
	elem->Collision = -0.99f;
	elem->Gravity = 0.0f;
	elem->Diffusion = 0.01f;
	elem->HotAir = 0.002f	* CFDS;
	elem->Falldown = 0;

	elem->Flammable = 0;
	elem->Explosive = 0;
	elem->Meltable = 0;
	elem->Hardness = 0;

	elem->Weight = -1;

	elem->DefaultProperties.temp = R_TEMP+4.0f	+273.15f;
	elem->HeatConduct = 60;
	elem->Latent = 0;
	elem->Description = "Neutrons. Interact with matter in odd ways.";

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

	elem->Update = &NEUT_update;
	elem->Graphics = &NEUT_graphics;
	elem->Func_Create = &NEUT_create;
	elem->Init = &NEUT_init_element;
}
