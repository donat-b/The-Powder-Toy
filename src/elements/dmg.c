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

int update_DMG(UPDATE_FUNC_ARGS)
{
	int r, rr, rx, ry, nb, nxi, nxj, t, dist;
	int rad = 25;
	float angle, fx, fy;

	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)!=PT_DMG && (r&0xFF)!=PT_EMBR && (r&0xFF)!=PT_DMND && (r&0xFF)!=PT_CLNE && (r&0xFF)!=PT_PCLN && (r&0xFF)!=PT_BCLN)
				{
					kill_part(i);
					for (nxj=-rad; nxj<=rad; nxj++)
						for (nxi=-rad; nxi<=rad; nxi++)
							if (x+nxi>=0 && y+nxj>=0 && x+nxi<XRES && y+nxj<YRES && (nxi || nxj))
							{
								dist = (int)(sqrt(pow(nxi, 2.0f)+pow(nxj, 2.0f)));//;(pow((float)nxi,2))/(pow((float)rad,2))+(pow((float)nxj,2))/(pow((float)rad,2));
								if (!dist || (dist <= rad))
								{
									rr = pmap[y+nxj][x+nxi];
									if (rr)
									{
										angle = atan2((float)nxj, nxi);
										fx = cos(angle) * 7.0f;
										fy = sin(angle) * 7.0f;

										parts[rr>>8].vx += fx;
										parts[rr>>8].vy += fy;

										vx[(y+nxj)/CELL][(x+nxi)/CELL] += fx;
										vy[(y+nxj)/CELL][(x+nxi)/CELL] += fy;

										pv[(y+nxj)/CELL][(x+nxi)/CELL] += 1.0f;

										t = parts[rr>>8].type;
										if(t && ptransitions[t].pht>-1 && ptransitions[t].pht<PT_NUM)
											part_change_type(rr>>8, x+nxi, y+nxj, ptransitions[t].pht);
										else if(t == PT_BMTL)
											part_change_type(rr>>8, x+nxi, y+nxj, PT_BRMT);
										else if(t == PT_GLAS)
											part_change_type(rr>>8, x+nxi, y+nxj, PT_BGLA);
										else if(t == PT_COAL)
											part_change_type(rr>>8, x+nxi, y+nxj, PT_BCOL);
										else if(t == PT_QRTZ)
											part_change_type(rr>>8, x+nxi, y+nxj, PT_PQRT);
										else if(t == PT_TUNG)
											part_change_type(rr>>8, x+nxi, y+nxj, PT_BRMT); 
									}
								}
							}
							return 1;
				}
			}
			return 0;
}

int graphics_DMG(GRAPHICS_FUNC_ARGS)
{
	*pixel_mode |= PMODE_FLARE;
	return 1;
}
