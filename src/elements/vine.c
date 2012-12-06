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

int update_VINE(UPDATE_FUNC_ARGS) {
	int r, np, rx =(rand()%3)-1, ry=(rand()%3)-1;
	if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
	{
		r = pmap[y+ry][x+rx];
		if (1>rand()%15)
			part_change_type(i,x,y,PT_PLNT);
		else if (!r)
		{
			np = create_part(-1,x+rx,y+ry,PT_VINE);
			if (np<0) return 0;
			parts[np].temp = parts[i].temp;
			parts[i].tmp = 1;
			part_change_type(i,x,y,PT_PLNT);
		}
	}
	if (parts[i].temp > 350 && parts[i].temp > parts[i].tmp2)
		parts[i].tmp2 = (int)parts[i].temp;
	return 0;
}

int graphics_VINE(GRAPHICS_FUNC_ARGS)
{
	float maxtemp = fmax(cpart->tmp2, cpart->temp);
	if (maxtemp > 300)
	{
		*colr += (int)restrict_flt((maxtemp-300)/5,0,58);
		*colg -= (int)restrict_flt((maxtemp-300)/2,0,102);
		*colb += (int)restrict_flt((maxtemp-300)/5,0,70);
	}
	if (maxtemp < 273)
	{
		*colg += (int)restrict_flt((273-maxtemp)/4,0,255);
		*colb += (int)restrict_flt((273-maxtemp)/1.5,0,255);
	}
	return 0;
}