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
int tempPartAmount[128];

#define PISTON_INACTIVE	0x00
#define PISTON_RETRACT	0x01
#define PISTON_EXTEND	0x02
#define MAX_FRAME		0x0F
#define DEFAULT_LIMIT	0x1F
#define DEFAULT_ARM_LIMIT	0xFF

int MoveStack(int stackX, int stackY, int directionX, int directionY, int size, int amount, int retract, int callDepth)
{
	int foundParts = 0;
	int posX, posY, r, spaces = 0, currentPos = 0;
	int c;
	r = pmap[stackY][stackX];
	if(!callDepth && (r&0xFF) == PT_FRME) {
		int biggestMove = amount;
		int newY = !!directionX, newX = !!directionY;
		//If the piston is pushing frame, iterate out from the centre to the edge and push everything resting on frame
		int c;
		for(c = 0; c < MAX_FRAME; c++) {
			posY = stackY + (c*newY);
			posX = stackX + (c*newX);
			if (posX < XRES && posY < YRES && posX >= 0 && posY >= 0) {
				r = pmap[posY][posX];
				if((r&0xFF) == PT_FRME) {
					int val = MoveStack(posX, posY, directionX, directionY, size, amount, retract, 1);
					if(val < biggestMove)
						biggestMove = val;
				} else 
					break;
			}
		}
		for(c = 1; c < MAX_FRAME; c++) {
			posY = stackY - (c*newY);
			posX = stackX - (c*newX);
			if (posX < XRES && posY < YRES && posX >= 0 && posY >= 0) {
				r = pmap[posY][posX];
				if((r&0xFF) == PT_FRME) {
					int val = MoveStack(posX, posY, directionX, directionY, size, amount, retract, 1);
					if(val < biggestMove)
						biggestMove = val;
				} else 	
					break;
			}
		}
		return biggestMove;
	}
	if(retract){
		int foundEnd = 0;
		//Warning: retraction does not scan to see if it has space
		for(posX = stackX, posY = stackY; currentPos < size; posX += directionX, posY += directionY) {
			if (!(posX < XRES && posY < YRES && posX >= 0 && posY >= 0)) {
				break;
			}
			r = pmap[posY][posX];
			if(!r) {
				break;
			} else {
				foundParts = 1;
				tempParts[currentPos++] = r>>8;
			}
		}
		if(foundParts) {
			//Move particles
			int j;
			for(j = 0; j < currentPos; j++) {
				int jP = tempParts[j];
				pmap[(int)(parts[jP].y + 0.5f)][(int)(parts[jP].x + 0.5f)] = 0;
				parts[jP].x += (float)((-directionX)*amount);
				parts[jP].y += (float)((-directionY)*amount);
				pmap[(int)(parts[jP].y + 0.5f)][(int)(parts[jP].x + 0.5f)] = parts[jP].type|(jP<<8);
			}
			return amount;
		}
		if(!foundParts && foundEnd)
			return amount;		
	} else {
		for(posX = stackX, posY = stackY; currentPos < size + amount; posX += directionX, posY += directionY) {
			if (!(posX < XRES && posY < YRES && posX >= 0 && posY >= 0)) {
				break;
			}
			r = pmap[posY][posX];
			if(!r) {
				spaces++;
				tempParts[currentPos++] = -1;
				if(spaces >= amount)
					break;
			} else {
				foundParts = 1;
				if(currentPos < size)
					tempParts[currentPos++] = r>>8;
				else 
					break;
			}
		}
		if(foundParts && spaces){
			//Move particles
			int possibleMovement = 0;
			int j;
			for(j = currentPos-1; j >= 0; j--) {
				int jP = tempParts[j];
				if(jP == -1) {
					possibleMovement++;
					continue;
				}
				if(!possibleMovement)
					continue;
				pmap[(int)(parts[jP].y + 0.5f)][(int)(parts[jP].x + 0.5f)] = 0;
				parts[jP].x += (float)(directionX*possibleMovement);
				parts[jP].y += (float)(directionY*possibleMovement);
				pmap[(int)(parts[jP].y + 0.5f)][(int)(parts[jP].x + 0.5f)] = parts[jP].type|(jP<<8);
			}
			return possibleMovement;
		}
		if(!foundParts && spaces)
			return spaces;
	}
	return 0;
}

int update_PSTN(UPDATE_FUNC_ARGS)
{
 	int maxSize = parts[i].tmp ? parts[i].tmp : DEFAULT_LIMIT;
 	int armLimit = parts[i].tmp2 ? parts[i].tmp2 : DEFAULT_ARM_LIMIT;
 	int state = 0;
	int r, nxx, nyy, nxi, nyi, rx, ry;
	int directionX = 0, directionY = 0;
	if(parts[i].ctype)
 		return 0;
	if (parts[i].life==0 && state == PISTON_INACTIVE) {
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry) && (!rx || !ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)==PT_SPRK && parts[r>>8].life==3) {
						if(parts[r>>8].ctype == PT_PSCN)
							state = PISTON_EXTEND;
						else
							state = PISTON_RETRACT;
					}
				}
	}
	if(state == PISTON_EXTEND || state == PISTON_RETRACT) {
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry) && (!rx || !ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF) == PT_PSTN) {
						directionX = rx;
						directionY = ry;
						{
							int foundEnd = 0;
							int pistonEndX, pistonEndY;
							int pistonCount = 0;
							int newSpace = 0;
							int armCount = 0;
							for (nxx = 0, nyy = 0, nxi = directionX, nyi = directionY; pistonCount < maxSize; nyy += nyi, nxx += nxi) {
								if (!(x+nxi+nxx<XRES && y+nyi+nyy<YRES && x+nxi+nxx >= 0 && y+nyi+nyy >= 0)) {
									break;
								}
								r = pmap[y+nyi+nyy][x+nxi+nxx];
								if((r&0xFF)==PT_PSTN) {
									if(parts[r>>8].ctype)
										armCount++;
									else
										pistonCount++;
								} else {
									pistonEndX = x+nxi+nxx;
									pistonEndY = y+nyi+nyy;
									foundEnd = 1;
									break;
								}
							}
							if(foundEnd) {
								if(state == PISTON_EXTEND) {
									if(armCount+pistonCount > armLimit)
										pistonCount = armLimit-armCount;
									if(pistonCount > 0) {
										newSpace = MoveStack(pistonEndX, pistonEndY, directionX, directionY, maxSize, pistonCount, 0, 0);
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
								} else if(state == PISTON_RETRACT) {
									if(pistonCount > armCount)
										pistonCount = armCount;
									if(armCount) {
										//Remove arm section
										int lastPistonX = pistonEndX - nxi;	//Go back to the very last piston arm particle
										int lastPistonY = pistonEndY - nyi;
										int j;
										for(j = 0; j < pistonCount; j++) {
											delete_part(lastPistonX+(nxi*-j), lastPistonY+(nyi*-j), 0);
										}
										MoveStack(pistonEndX, pistonEndY, directionX, directionY, maxSize, pistonCount, 1, 0);
										//newSpace = MoveStack(pistonEndX, pistonEndY, directionX, directionY, maxSize, pistonCount, 1, 0);
									}
								}
							}
						}

						break;
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
