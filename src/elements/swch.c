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

int update_SWCH(UPDATE_FUNC_ARGS) {
	//turn off SWCH from two red BRAYS
	if (parts[i].life==10 && (!(pmap[y-1][x-1]&0xFF) && ((pmap[y-1][x]&0xFF)==PT_BRAY&&parts[pmap[y-1][x]>>8].tmp==2) && !(pmap[y-1][x+1]&0xFF) && ((pmap[y][x+1]&0xFF)==PT_BRAY&&parts[pmap[y][x+1]>>8].tmp==2)))
	{
		parts[i].life = 9;
	}
	//turn on SWCH from two red BRAYS
	else if (parts[i].life<=5 && (!(pmap[y-1][x-1]&0xFF) && (((pmap[y-1][x]&0xFF)==PT_BRAY&&parts[pmap[y-1][x]>>8].tmp==2) || ((pmap[y+1][x]&0xFF)==PT_BRAY&&parts[pmap[y+1][x]>>8].tmp==2)) && !(pmap[y-1][x+1]&0xFF) && (((pmap[y][x+1]&0xFF)==PT_BRAY&&parts[pmap[y][x+1]>>8].tmp==2) || ((pmap[y][x-1]&0xFF)==PT_BRAY&&parts[pmap[y][x-1]>>8].tmp==2))))
	{
		parts[i].life = 14;
	}
	return 0;
}
