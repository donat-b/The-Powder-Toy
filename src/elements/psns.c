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

int update_PSNS(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, rt;
	if (pv[y/CELL][x/CELL] > parts[i].temp-273.15f)
	{
		parts[i].life = 0;
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					rt = r&0xFF;
					if (parts_avg(i,r>>8,PT_INSL) != PT_INSL)
					{
						if ((ptypes[rt].properties&PROP_CONDUCTS) && !(rt==PT_WATR||rt==PT_SLTW||rt==PT_NTCT||rt==PT_PTCT||rt==PT_INWR) && parts[r>>8].life==0)
						{
							parts[r>>8].life = 4;
							parts[r>>8].ctype = rt;
							part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
						}
					}
				}
	}
	return 0;
}
