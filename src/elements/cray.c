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

unsigned int wavelengthToDecoColour(int wavelength)
{
	int colr = 0, colg = 0, colb = 0, x;
	unsigned int dcolour = 0;
	for (x=0; x<12; x++) {
		colr += (wavelength >> (x+18)) & 1;
		colb += (wavelength >>  x)     & 1;
	}
	for (x=0; x<12; x++)
		colg += (wavelength >> (x+9))  & 1;
	x = 624/(colr+colg+colb+1);
	colr *= x;
	colg *= x;
	colb *= x;

	if(colr > 255) colr = 255;
	else if(colr < 0) colr = 0;
	if(colg > 255) colg = 255;
	else if(colg < 0) colg = 0;
	if(colb > 255) colb = 255;
	else if(colb < 0) colb = 0;

	return (255<<24) | (colr<<16) | (colg<<8) | colb;
}

int update_CRAY(UPDATE_FUNC_ARGS)
{
	int r, nxx, nyy, docontinue, nxi, nyi, rx, ry, nr, ry1, rx1, partsRemaining = 255;
	if (parts[i].tmp)
		partsRemaining = parts[i].tmp;
	if(parts[i].ctype<=0 || parts[i].ctype>=PT_NUM || !ptypes[parts[i].ctype].enabled) {
		int r, rx, ry;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES)
				{
					r = photons[y+ry][x+rx];
					if (!r)
						r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)!=PT_CRAY && (r&0xFF)<PT_NUM)
					{
						parts[i].ctype = r&0xFF;
					}
				}
	} else if (parts[i].life==0) {
		unsigned int colored = 0;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)==PT_SPRK && parts[r>>8].life==3) {
						int destroy = (parts[r>>8].ctype==PT_PSCN)?1:0;
						int nostop = (parts[r>>8].ctype==PT_INST)?1:0;
						for (docontinue = 1, nxx = 0, nyy = 0, nxi = rx*-1, nyi = ry*-1; docontinue; nyy+=nyi, nxx+=nxi) {
							if (!(x+nxi+nxx<XRES && y+nyi+nyy<YRES && x+nxi+nxx >= 0 && y+nyi+nyy >= 0)) {
								break;
							}
							r = pmap[y+nyi+nyy][x+nxi+nxx];
							if (!r) {
								int nr = create_part(-1, x+nxi+nxx, y+nyi+nyy, parts[i].ctype);
								if (nr!=-1) {
									parts[nr].dcolour = colored;
									if(!--partsRemaining)
										docontinue = 0;
								}
							} else if ((r&0xFF)==PT_FILT) {//get color if passed through FILT
								colored = wavelengthToDecoColour(parts[r>>8].ctype);
							} else if ((r&0xFF)==PT_CRAY || nostop) {
								docontinue = 1;
							} else if(destroy && (r&0xFF != PT_DMND)) {
								kill_part(r>>8);
								if(!--partsRemaining)
									docontinue = 0;
							} else {
								docontinue = 0;
							}
						}
					}
				}
	}
	return 0;
}
