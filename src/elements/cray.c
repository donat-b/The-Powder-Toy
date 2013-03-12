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
	int r, nxx, nyy, docontinue, nxi, nyi, rx, ry, nr, ry1, rx1;
	// set ctype to things that touch it if it doesn't have one already
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
					if ((r&0xFF)!=PT_CRAY && (r&0xFF)!=PT_PSCN && (r&0xFF)!=PT_INST && (r&0xFF)!=PT_METL && (r&0xFF)!=PT_SPRK && (r&0xFF)<PT_NUM)
					{
						parts[i].ctype = r&0xFF;
						parts[i].temp = parts[r>>8].temp;
					}
				}
	} else if (parts[i].life==0) { // only fire when life is 0, but nothing sets the life right now
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)==PT_SPRK && parts[r>>8].life==3) { //spark found, start creating
						unsigned int colored = 0;
						int destroy = parts[r>>8].ctype==PT_PSCN;
						int nostop = parts[r>>8].ctype==PT_INST;
						int createSpark = (parts[r>>8].ctype==PT_INWR);
						int partsRemaining = 255;
						if (parts[i].tmp) //how far it shoots
							partsRemaining = parts[i].tmp;
						for (docontinue = 1, nxx = 0, nyy = 0, nxi = rx*-1, nyi = ry*-1; docontinue; nyy+=nyi, nxx+=nxi) {
							if (!(x+nxi+nxx<XRES && y+nyi+nyy<YRES && x+nxi+nxx >= 0 && y+nyi+nyy >= 0)) {
								break;
							}
							r = pmap[y+nyi+nyy][x+nxi+nxx];
							if (!IsWallBlocking(x+nxi+nxx, y+nyi+nyy, parts[i].ctype) && (!pmap[y+nyi+nyy][x+nxi+nxx] || createSpark)) { // create, also set color if it has passed through FILT
								int nr;
								if (parts[i].ctype == PT_LIFE)
									nr = create_part(-1, x+nxi+nxx, y+nyi+nyy, parts[i].ctype|(parts[i].tmp2<<8));
								else
									nr = create_part(-1, x+nxi+nxx, y+nyi+nyy, parts[i].ctype);
								if (nr!=-1) {
									parts[nr].dcolour = colored;
									parts[nr].temp = parts[i].temp;
									if(!--partsRemaining)
										docontinue = 0;
								}
							} else if ((r&0xFF)==PT_FILT) { // get color if passed through FILT
								colored = wavelengthToDecoColour(parts[r>>8].ctype);
							} else if ((r&0xFF) == PT_CRAY || nostop) {
								docontinue = 1;
							} else if(destroy && ((r&0xFF) != PT_DMND)) {
								kill_part(r>>8);
								if(!--partsRemaining)
									docontinue = 0;
							}
							else
								docontinue = 0;
							if(!partsRemaining)
								docontinue = 0;
						}
					}
				}
	}
	return 0;
}
