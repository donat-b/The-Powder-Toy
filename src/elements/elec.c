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

#include <element.h>

int update_ELEC(UPDATE_FUNC_ARGS) {
	int r, rt, rx, ry, nb, rrx, rry;
	float rr, rrr;
	parts[i].pavg[0] = (float)x;
	parts[i].pavg[1] = (float)y;
	if(pmap[y][x]==PT_GLOW)
	{
		part_change_type(i, x, y, PT_PHOT);
	}
	for (rx=-2; rx<=2; rx++)
		for (ry=-2; ry<=2; ry++)
			if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES) {
				r = pmap[y+ry][x+rx];
				if (!r)
					r = photons[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_GLAS)
				{
					for (rrx=-1; rrx<=1; rrx++)
					{
						for (rry=-1; rry<=1; rry++)
						{
							if (x+rx+rrx>=0 && y+ry+rry>=0 && x+rx+rrx<XRES && y+ry+rry<YRES) {
								nb = create_part(-1, x+rx+rrx, y+ry+rry, PT_EMBR);
								if (nb!=-1) {
									parts[nb].tmp = 0;
									parts[nb].life = 50;
									parts[nb].temp = parts[i].temp*0.8f;
									parts[nb].vx = (float)(rand()%20-10);
									parts[nb].vy = (float)(rand()%20-10);
								}
							}
						}
					}
					fire_r[y/CELL][x/CELL] += rand()%200;   //D: Doesn't work with OpenGL, also shouldn't be here
					fire_g[y/CELL][x/CELL] += rand()%200;
					fire_b[y/CELL][x/CELL] += rand()%200;
					/* possible alternative, but doesn't work well at the moment because FIRE_ADD divides firea by 8, so the glow isn't strong enough
					create_part(i, x, y, PT_EMBR);
					parts[i].tmp = 2;
					parts[i].life = 2;
					parts[i].ctype = ((rand()%200)<<16) | ((rand()%200)<<8) | (rand()%200);
					*/
					kill_part(i);
					return 1;
				}
				if ((r&0xFF)==PT_LCRY)
				{
					parts[r>>8].tmp2 = 5+rand()%5;
				}
				if ((r&0xFF)==PT_WATR || (r&0xFF)==PT_DSTW || (r&0xFF)==PT_SLTW || (r&0xFF)==PT_CBNW)
				{
					if(rand()<RAND_MAX/3)
					{
						create_part(r>>8, x+rx, y+ry, PT_O2);
					}
					else
					{
						create_part(r>>8, x+rx, y+ry, PT_H2);
					}
					kill_part(i);
					return 1;
				}
				if ((r&0xFF)==PT_NEUT && !pmap[y+ry][x+rx])
				{
					part_change_type(r>>8, x+rx, y+ry, PT_H2);
					parts[r>>8].life = 0;
					parts[r>>8].ctype = 0;
				}
				if ((r&0xFF)==PT_DEUT)
				{
					if(parts[r>>8].life < 6000)
						parts[r>>8].life += 1;
					parts[r>>8].temp = 0;
					parts[i].temp = 0;
					kill_part(i);
					return 1;
				}
				if ((r&0xFF)==PT_EXOT)
				{
					parts[r>>8].tmp2 += 5;
					parts[r>>8].life = 1000;
				}
				if (ptypes[r&0xFF].properties & PROP_CONDUCTS && ((r&0xFF)!=PT_NBLE||parts[i].temp<2273.15))
				{
					create_part(-1, x+rx, y+ry, PT_SPRK);
					kill_part(i);
					return 1;
				}
			}
	return 0;
}

int graphics_ELEC(GRAPHICS_FUNC_ARGS)
{
	*firea = 70;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;

	*pixel_mode |= FIRE_ADD;
	return 0;
}
