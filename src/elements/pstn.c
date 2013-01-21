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

int tempParts[128];

int MoveStack(int stackX, int stackY, int directionX, int directionY, int size, int amount, int retract)
{
	int foundEnd = 0, foundParts = 0;
	int posX, posY, r, spaces = 0, currentPos = 0;
	for(posX = stackX, posY = stackY; currentPos < size; posX += directionX, posY += directionY)
	{
		if (!(posX < XRES && posY < YRES && posX >= 0 && posY >= 0)) {
			break;
		}
		r = pmap[posY][posX];
		if(!r) {
			spaces++;
			foundEnd = 1;
			if(spaces >= amount)
				break;
		} else {
			foundParts = 1;
			tempParts[currentPos++] = r>>8;
		}
	}
	if(amount > spaces)
		amount = spaces;
	if(foundParts && foundEnd) {
		//Move particles
		int j;
		for(j = 0; j < currentPos; j++) {
			int jP = tempParts[j], nPx, nPy;
			parts[jP].x += (float)(directionX*amount);
			parts[jP].y += (float)(directionY*amount);
			nPx = (int)(parts[jP].x + 0.5f);
			nPy = (int)(parts[jP].y + 0.5f);
			pmap[nPy][nPx] = parts[jP].type|(jP<<8);
		}
		return amount;
	}
	if(!foundParts && foundEnd)
		return amount;
	return 0;
}

int update_PSTN(UPDATE_FUNC_ARGS)
{
	int maxSize = parts[i].tmp ? parts[i].tmp : 15;
 	int currentSize = parts[i].tmp2&0xFF;
 	int currentDirection = (parts[i].tmp2>>8)&0xFF;
 	int state = parts[i].ctype;
	int r, nxx, nyy, docontinue, nxi, nyi, rx, ry, nr, ry1, rx1;
	if (parts[i].life==0 && !state) {
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry) && (!rx || !ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)==PT_SPRK && parts[r>>8].life==3) {
						int foundEnd = 0;
						int pistonEndX, pistonEndY;
						int pistonCount = 1;
						int newSpace = 0;
						for (nxx = 0, nyy = 0, nxi = rx*-1, nyi = ry*-1; pistonCount < maxSize; nyy+=nyi, nxx+=nxi) {
							if (!(x+nxi+nxx<XRES && y+nyi+nyy<YRES && x+nxi+nxx >= 0 && y+nyi+nyy >= 0)) {
								break;
							}
							r = pmap[y+nyi+nyy][x+nxi+nxx];
							if((r&0xFF)==PT_PSTN) {
								if(!parts[r>>8].ctype)
									pistonCount++;
							} else {
								pistonEndX = x+nxi+nxx;
								pistonEndY = y+nyi+nyy;
								foundEnd = 1;
								break;
							}
						}
						if(foundEnd) {
							newSpace = MoveStack(pistonEndX, pistonEndY, nxi, nyi, maxSize, pistonCount, 0);
							if(newSpace) {
								//Create new piston section
								int j;
								for(j = 0; j < newSpace; j++) {
									int nr = create_part(-3, pistonEndX+(nxi*j), pistonEndY+(nyi*j), PT_PSTN);
									if (nr!=-1) {
										parts[nr].ctype = 1;
									}
								}
							}
						}

					}
				}
	}
	return 0;
}

int graphics_PSTN(GRAPHICS_FUNC_ARGS)
{
	if(cpart->ctype)
	{
		*colr -= 60;
		*colg -= 60;
	}
	return 0;
}
