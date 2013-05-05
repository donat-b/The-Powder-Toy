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

int update_GOLD(UPDATE_FUNC_ARGS)
{
	int rx, ry, r, blocking = 0, j;
	static int checkCoordsX[] = { -4, 4, 0, 0 };
	static int checkCoordsY[] = { 0, 0, -4, 4 };
	//Find nearby rusted iron (BMTL with tmp 1+)
	for(j = 0; j < 8; j++){
		rx = (rand()%9)-4;
		ry = (rand()%9)-4;
		if ((!rx != !ry) && BOUNDS_CHECK) {
			r = pmap[y+ry][x+rx];
			if(!r) continue;
			if((r&0xFF)==PT_BMTL && parts[r>>8].tmp)
			{
				parts[r>>8].tmp = 0;
				part_change_type(r>>8, x+rx, y+ry, PT_IRON);
			}
		}
	}
	//Find sparks
	if(!parts[i].life)
	{
		for(j = 0; j < 4; j++){
			rx = checkCoordsX[j];
			ry = checkCoordsY[j];
			if ((!rx != !ry) && BOUNDS_CHECK) {
				r = pmap[y+ry][x+rx];
				if(!r) continue;
				if((r&0xFF)==PT_SPRK && parts[r>>8].life && parts[r>>8].life<4)
				{
					parts[i].life = 4;
					parts[i].type = PT_SPRK;
					parts[i].ctype = PT_GOLD;
				}
			}
		}
	}
	if ((photons[y][x]&0xFF) == PT_NEUT)
	{
		if (!(rand()%7))
		{
			kill_part(photons[y][x]>>8);
		}
	}
	return 0;
}

int graphics_GOLD(GRAPHICS_FUNC_ARGS)
{
	*colr += rand()%10-5;
	*colg += rand()%10-5;
	*colb += rand()%10-5;
	return 0;
}
