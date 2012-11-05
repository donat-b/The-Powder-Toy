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

int isRedBRAY(UPDATE_FUNC_ARGS, int xc, int yc)
{
	return (pmap[yc][xc]&0xFF) == PT_BRAY && parts[pmap[yc][xc]>>8].tmp == 2;
}

int update_SWCH(UPDATE_FUNC_ARGS)
{
	//turn SWCH on/off from two red BRAYS. There must be one either above or below, and one either left or right to work, and it can't come from the side, it must be a diagonal beam
	if (!(pmap[y-1][x-1]&0xFF) && !(pmap[y-1][x+1]&0xFF) && (isRedBRAY(UPDATE_FUNC_SUBCALL_ARGS, x, y-1) || isRedBRAY(UPDATE_FUNC_SUBCALL_ARGS, x, y+1)) && (isRedBRAY(UPDATE_FUNC_SUBCALL_ARGS, x+1, y) || isRedBRAY(UPDATE_FUNC_SUBCALL_ARGS, x-1, y)))
	{
		if (parts[i].life == 10)
			parts[i].life = 9;
		else if (parts[i].life <= 5)
			parts[i].life = 14;
	}
	return 0;
}
