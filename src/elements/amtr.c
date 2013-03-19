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

int update_AMTR(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = ((pmap[y+ry][x+rx]&0xFF)==PT_PINV&&parts[pmap[y+ry][x+rx]>>8].life==10)?0:pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)!=PT_AMTR && !(ptypes[r&0xFF].properties&PROP_INDESTRUCTIBLE) && !(ptypes[r&0xFF].properties&PROP_CLONE) && (r&0xFF)!=PT_NONE && (r&0xFF)!=PT_PHOT && (r&0xFF)!=PT_VOID && (r&0xFF)!=PT_BHOL && (r&0xFF)!=PT_NBHL && (r&0xFF)!=PT_PRTI && (r&0xFF)!=PT_PRTO && (r&0xFF)!=PT_PPTI && (r&0xFF)!=PT_PPTO)
				{
					if(!parts[i].ctype || (parts[i].ctype==(r&0xFF))!=(parts[i].tmp&1))
					{
						parts[i].life++;
						if (parts[i].life==4)
						{
							kill_part(i);
							return 1;
						}
						if (10>(rand()/(RAND_MAX/100)))
							create_part(r>>8, x+rx, y+ry, PT_PHOT);
						else
							kill_part(r>>8);
						pv[y/CELL][x/CELL] -= 2.0f;
					}
				}
			}
	return 0;
}
