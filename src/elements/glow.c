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

int update_GLOW(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_WATR&&5>(rand()%2000))
				{
					kill_part(i);
					part_change_type(r>>8,x+rx,y+ry,PT_DEUT);
					parts[r>>8].life = 10;
				}
				/*else if (((r&0xFF)==PT_TTAN || ((r&0xFF)==PT_LAVA && parts[r>>8].ctype == PT_TTAN)) && pv[y/CELL][x/CELL] < -200) //not final
				{
					int index;
					if (rand()%5)
						kill_part(r>>8);
					index = create_part(i, x, y, PT_BVBR);
					if (index != -1)
						parts[index].ctype = 1;
					return 1;
				}*/
			}
	parts[i].ctype = (int)pv[y/CELL][x/CELL]*16;

	parts[i].tmp = abs((int)((vx[y/CELL][x/CELL]+vy[y/CELL][x/CELL])*16.0f)) + abs((int)((parts[i].vx+parts[i].vy)*64.0f));
	//printf("%f %f\n", parts[i].vx, parts[i].vy);
	if (parts[i].type==PT_NONE) {
		kill_part(i);
		return 1;
	}
	return 0;
}
