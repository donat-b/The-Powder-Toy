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

int update_PUMP(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	if (parts[i].life==10)
	{
		if (parts[i].temp>=256.0+273.15)
			parts[i].temp=256.0f+273.15f;
		if (parts[i].temp<= -256.0+273.15)
			parts[i].temp = -256.0f+273.15f;

		if (pv[y/CELL][x/CELL]<(parts[i].temp-273.15))
			pv[y/CELL][x/CELL] += 0.1f*((parts[i].temp-273.15)-pv[y/CELL][x/CELL]);
		if (y+CELL<YRES && pv[y/CELL+1][x/CELL]<(parts[i].temp-273.15))
			pv[y/CELL+1][x/CELL] += 0.1f*((parts[i].temp-273.15)-pv[y/CELL+1][x/CELL]);
		if (x+CELL<XRES)
		{
			pv[y/CELL][x/CELL+1] += 0.1f*((parts[i].temp-273.15)-pv[y/CELL][x/CELL+1]);
			if (y+CELL<YRES)
				pv[y/CELL+1][x/CELL+1] += 0.1f*((parts[i].temp-273.15)-pv[y/CELL+1][x/CELL+1]);
		}
	}
	return 0;
}
