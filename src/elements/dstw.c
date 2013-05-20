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

int update_DSTW(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_SALT && 1>(rand()%50))
				{
					part_change_type(i,x,y,PT_SLTW);
					// on average, convert 3 DSTW to SLTW before SALT turns into SLTW
					if (rand()%3==0)
						part_change_type(r>>8,x+rx,y+ry,PT_SLTW);
				}
				if (((r&0xFF)==PT_WATR||(r&0xFF)==PT_SLTW) && 1>(rand()%100))
				{
					part_change_type(i,x,y,PT_WATR);
				}
				if ((r&0xFF)==PT_SLTW && !(rand()%2000))
				{
					part_change_type(i,x,y,PT_SLTW);
				}
				if (((r&0xFF)==PT_RBDM||(r&0xFF)==PT_LRBD) && (legacy_enable||parts[i].temp>12.0f) && !(rand()%100))
				{
					part_change_type(i,x,y,PT_FIRE);
					parts[i].life = 4;
				}
				if ((r&0xFF)==PT_FIRE){
					kill_part(r>>8);
						if(!(rand()%30)){
							kill_part(i);
							return 1;
						}
				}
			}
	return 0;
}
