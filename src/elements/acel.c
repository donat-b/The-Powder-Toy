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

int update_ACEL(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	float multiplier;
	if (parts[i].life!=0)
	{
		float change = (float)(parts[i].life > 1000 ? 1000 : (parts[i].life < 0 ? 0 : parts[i].life));
		multiplier = 1.0f+(change/100.0f);
	}
	else
	{
		multiplier = 1.1f;
	}
	parts[i].tmp = 0;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry) && !(rx && ry))
			{
				r = pmap[y+ry][x+rx];
				if(!r)
					r = photons[y+ry][x+rx];
				if ((r>>8)>=NPART || !r)
					continue;
				if(ptypes[r&0xFF].properties & (TYPE_PART | TYPE_LIQUID | TYPE_GAS | TYPE_ENERGY))
				{
					parts[r>>8].vx *= multiplier;
					parts[r>>8].vy *= multiplier;
					parts[i].tmp = 1;
				}
			}
	return 0;
}

int graphics_ACEL(GRAPHICS_FUNC_ARGS)
{
	if(cpart->tmp)
		*pixel_mode |= PMODE_GLOW;
	return 0;
}
int update_DCEL(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	float multiplier;
	if (parts[i].life!=0)
	{
		float change = (float)(parts[i].life > 100 ? 100 : (parts[i].life < 0 ? 0 : parts[i].life));
		multiplier = 1.0f-(change/100.0f);
	}
	else
	{
		multiplier = 1.0f/1.1f;
	}
	parts[i].tmp = 0;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry) && !(rx && ry))
			{
				r = pmap[y+ry][x+rx];
				if(!r)
					r = photons[y+ry][x+rx];
				if ((r>>8)>=NPART || !r)
					continue;
				if(ptypes[r&0xFF].properties & (TYPE_PART | TYPE_LIQUID | TYPE_GAS | TYPE_ENERGY))
				{
					parts[r>>8].vx *= multiplier;
					parts[r>>8].vy *= multiplier;
					parts[i].tmp = 1;
				}
			}
	return 0;
}

int graphics_DCEL(GRAPHICS_FUNC_ARGS)
{
	if(cpart->tmp)
		*pixel_mode |= PMODE_GLOW;
	return 0;
}
