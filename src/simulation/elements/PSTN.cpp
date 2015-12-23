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

#include "simulation/ElementsCommon.h"

int tempParts[XRES];

#define PISTON_INACTIVE	0x00
#define PISTON_RETRACT	0x01
#define PISTON_EXTEND	0x02
#define MAX_FRAME		0x0F
#define DEFAULT_LIMIT	0x1F
#define DEFAULT_ARM_LIMIT	0xFF

int CanMoveStack(Simulation *sim, int stackX, int stackY, int directionX, int directionY, int maxSize, int amount, int retract, int block)
{
	int posX, posY, r, spaces = 0, currentPos = 0;
	if (amount <= 0)
		return 0;
	for (posX = stackX, posY = stackY; currentPos < maxSize + amount && currentPos < XRES-1; posX += directionX, posY += directionY)
	{
		if (!(posX < XRES && posY < YRES && posX >= 0 && posY >= 0))
		{
			break;
		}
		r = pmap[posY][posX];
		if (sim->IsWallBlocking(posX, posY, 0) || (block && (r&0xFF) == block))
			return spaces;
		if (!r)
		{
			spaces++;
			tempParts[currentPos++] = -1;
			if (spaces >= amount)
				break;
		}
		else
		{
			if (spaces < maxSize && currentPos < maxSize && (!retract || (((r&0xFF) == PT_FRME) && posX == stackX && posY == stackY)))
				tempParts[currentPos++] = r>>8;
			else
				return spaces;
		}
	}
	if (spaces)
		return currentPos;
	else
		return 0;
}

int MoveStack(Simulation *sim, int stackX, int stackY, int directionX, int directionY, int maxSize, int amount, int retract, int block, int sticky, int callDepth)
{
	int foundParts = 0;
	int posX, posY, r, spaces = 0, currentPos = 0;
	int c, j;
	r = pmap[stackY][stackX];
	if (!callDepth && (r&0xFF) == PT_FRME)
	{
		int newY = !!directionX, newX = !!directionY;
		int realDirectionX = retract?-directionX:directionX;
		int realDirectionY = retract?-directionY:directionY;
		int maxRight = MAX_FRAME, maxLeft = MAX_FRAME;

		//check if we can push all the FRME
		for (c = retract; c < MAX_FRAME; c++)
		{
			posY = stackY + (c*newY);
			posX = stackX + (c*newX);
			if (posX < XRES && posY < YRES && posX >= 0 && posY >= 0 && (pmap[posY][posX]&0xFF) == PT_FRME)
			{
				int val = CanMoveStack(sim, posX, posY, realDirectionX, realDirectionY, maxSize, amount, retract, block);
				if (val < amount)
					amount = val;
			}
			else
			{
				maxRight = c;
				break;
			}
		}
		for (c = 1; c < MAX_FRAME; c++)
		{
			posY = stackY - (c*newY);
			posX = stackX - (c*newX);
			if (posX < XRES && posY < YRES && posX >= 0 && posY >= 0 && (pmap[posY][posX]&0xFF) == PT_FRME)
			{
				int val = CanMoveStack(sim, posX, posY, realDirectionX, realDirectionY, maxSize, amount, retract, block);
				if (val < amount)
					amount = val;
			}
			else
			{
				maxLeft = c;
				break;
			}
		}

		//If the piston is pushing frame, iterate out from the centre to the edge and push everything resting on frame
		for (c = 1; c < maxRight; c++)
		{
			posY = stackY + (c*newY);
			posX = stackX + (c*newX);
			MoveStack(sim, posX, posY, directionX, directionY, maxSize, amount, retract, block, !parts[pmap[posY][posX]>>8].tmp, 1);
		}
		for (c = 1; c < maxLeft; c++)
		{
			posY = stackY - (c*newY);
			posX = stackX - (c*newX);
			MoveStack(sim, posX, posY, directionX, directionY, maxSize, amount, retract, block, !parts[pmap[posY][posX]>>8].tmp, 1);
		}

		//Remove arm section if retracting with FRME
		if (retract)
			for (j = 1; j <= amount; j++)
				sim->part_kill(pmap[stackY+(directionY*-j)][stackX+(directionX*-j)]>>8);
		return MoveStack(sim, stackX, stackY, directionX, directionY, maxSize, amount, retract, block, !parts[pmap[stackY][stackX]>>8].tmp, 1);
	}
	if (retract)
	{
		int foundEnd = 0;
		//Remove arm section if retracting without FRME
		if (!callDepth)
			for (j = 1; j <= amount; j++)
				sim->part_kill(pmap[stackY+(directionY*-j)][stackX+(directionX*-j)]>>8);
		for (posX = stackX, posY = stackY; currentPos < maxSize && currentPos < XRES-1; posX += directionX, posY += directionY)
		{
			if (!(posX < XRES && posY < YRES && posX >= 0 && posY >= 0))
			{
				break;
			}
			r = pmap[posY][posX];
			if (!r || (r&0xFF) == block || (!sticky && (r&0xFF) != PT_FRME))
			{
				break;
			}
			else
			{
				foundParts = 1;
				tempParts[currentPos++] = r>>8;
			}
		}
		if (foundParts)
		{
			//Move particles
			for (j = 0; j < currentPos; j++)
			{
				int jP = tempParts[j];
				int srcX = (int)(parts[jP].x + 0.5f), srcY = (int)(parts[jP].y + 0.5f);
				int destX = srcX-directionX*amount, destY = srcY-directionY*amount;
				pmap[srcY][srcX] = 0;
				parts[jP].x = (float)destX;
				parts[jP].y = (float)destY;
				pmap[destY][destX] = parts[jP].type|(jP<<8);
			}
			return amount;
		}
		if (!foundParts && foundEnd)
			return amount;
	}
	else
	{
		currentPos = CanMoveStack(sim, stackX, stackY, directionX, directionY, maxSize, amount, retract, block);
		if (currentPos)
		{
			//Move particles
			int possibleMovement = 0;
			for (j = currentPos-1; j >= 0; j--)
			{
				int jP = tempParts[j];
				if (jP < 0)
				{
					possibleMovement++;
					continue;
				}
				if (!possibleMovement)
					continue;
				int srcX = (int)(parts[jP].x + 0.5f), srcY = (int)(parts[jP].y + 0.5f);
				int destX = srcX+directionX*possibleMovement, destY = srcY+directionY*possibleMovement;
				pmap[srcY][srcX] = 0;
				parts[jP].x = (float)destX;
				parts[jP].y = (float)destY;
				pmap[destY][destX] = parts[jP].type|(jP<<8);
			}
			return possibleMovement;
		}
		if (!foundParts && spaces)
			return spaces;
	}
	return 0;
}

int PSTN_update(UPDATE_FUNC_ARGS)
{
	int maxSize = parts[i].tmp ? parts[i].tmp : DEFAULT_LIMIT;
	int armLimit = parts[i].tmp2 ? parts[i].tmp2 : DEFAULT_ARM_LIMIT;
	int state = 0;
	int r, nxx, nyy, nxi, nyi, rx, ry;
	int directionX = 0, directionY = 0;
	if (parts[i].life)
		return 0;
	if (state == PISTON_INACTIVE)
	{
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (BOUNDS_CHECK && (rx || ry) && (!rx || !ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)==PT_SPRK && parts[r>>8].life==3)
					{
						if (parts[r>>8].ctype == PT_PSCN)
							state = PISTON_EXTEND;
						else
							state = PISTON_RETRACT;
					}
				}
	}
	if (state == PISTON_EXTEND || state == PISTON_RETRACT)
	{
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (rx || ry) && (!rx || !ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF) == PT_PSTN)
					{
						int movedPiston = 0;
						int foundEnd = 0;
						int pistonEndX = 0, pistonEndY = 0;
						int pistonCount = -1; // number of PSTN particles minus 1
						int newSpace = 0;
						int armCount = 0;
						directionX = rx;
						directionY = ry;
						for (nxx = 0, nyy = 0, nxi = directionX, nyi = directionY; ; nyy += nyi, nxx += nxi)
						{
							if (!(x+nxx<XRES && y+nyy<YRES && x+nxx >= 0 && y+nyy >= 0))
							{
								break;
							}
							r = pmap[y+nyy][x+nxx];
							if ((r&0xFF)==PT_PSTN)
							{
								if (parts[r>>8].life)
									armCount++;
								else if (armCount)
								{
									pistonEndX = x+nxx;
									pistonEndY = y+nyy;
									foundEnd = 1;
									break;
								}
								else
								{
									//pistonCount += floor((parts[r>>8].temp-268.15)/10);// How many tens of degrees above 0 C, rounded to nearest ten degrees. Can be negative.
									pistonCount++;
								}
							}
							else if (nxx==0 && nyy==0)
							{
								pistonCount++;
								//pistonCount += floor((parts[i].temp-268.15)/10);
								continue;
							}
							else
							{
								pistonEndX = x+nxx;
								pistonEndY = y+nyy;
								foundEnd = 1;
								break;
							}
						}
						if (foundEnd)
						{
							if (state == PISTON_EXTEND)
							{
								if (armCount+pistonCount > armLimit)
									pistonCount = armLimit-armCount;
								if (pistonCount > 0)
								{
									newSpace = MoveStack(sim, pistonEndX, pistonEndY, directionX, directionY, maxSize, pistonCount, 0, parts[i].ctype, 1, 0);
									if (newSpace)
									{
										//Create new piston section
										for (int j = 0; j < newSpace; j++)
										{
											int nr = sim->part_create(-3, pistonEndX+(nxi*j), pistonEndY+(nyi*j), PT_PSTN);
											if (nr > -1)
											{
												parts[nr].life = 1;
												if (parts[i].dcolour)
												{
													int colour = parts[i].dcolour;
													parts[nr].dcolour = COLARGB(COLA(colour), std::max(COLR(colour)-60, 0), std::max(COLG(colour)-60, 0), std::max(COLB(colour)-60,0));
												}
											}
										}
										movedPiston =  1;
									}
								}
							}
							else if (state == PISTON_RETRACT)
							{
								if (pistonCount > armCount)
									pistonCount = armCount;
								if (armCount && pistonCount > 0)
								{
									MoveStack(sim, pistonEndX, pistonEndY, directionX, directionY, maxSize, pistonCount, 1, parts[i].ctype, 1, 0);
									movedPiston = 1;
								}
							}
						}
						if (movedPiston)
							return 0;
					}
				}

	}
	return 0;
}

int PSTN_graphics(GRAPHICS_FUNC_ARGS)
{
	if(cpart->life)
	{
		*colr -= 60;
		*colg -= 60;
	}
	return 0;
}

void PSTN_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_PSTN";
	elem->Name = "PSTN";
	elem->Colour = COLPACK(0xAA9999);
	elem->MenuVisible = 1;
	elem->MenuSection = SC_FORCE;
	elem->Enabled = 1;

	elem->Advection = 0.0f;
	elem->AirDrag = 0.00f * CFDS;
	elem->AirLoss = 0.90f;
	elem->Loss = 0.00f;
	elem->Collision = 0.0f;
	elem->Gravity = 0.0f;
	elem->Diffusion = 0.00f;
	elem->HotAir = 0.000f	* CFDS;
	elem->Falldown = 0;

	elem->Flammable = 0;
	elem->Explosive = 0;
	elem->Meltable = 0;
	elem->Hardness = 0;

	elem->Weight = 100;

	elem->DefaultProperties.temp = 283.15f;
	elem->HeatConduct = 0;
	elem->Latent = 0;
	elem->Description = "Piston, extends and pushes particles.";

	elem->State = ST_SOLID;
	elem->Properties = TYPE_SOLID;

	elem->LowPressureTransitionThreshold = IPL;
	elem->LowPressureTransitionElement = NT;
	elem->HighPressureTransitionThreshold = IPH;
	elem->HighPressureTransitionElement = NT;
	elem->LowTemperatureTransitionThreshold = ITL;
	elem->LowTemperatureTransitionElement = NT;
	elem->HighTemperatureTransitionThreshold = ITH;
	elem->HighTemperatureTransitionElement = NT;

	elem->Update = &PSTN_update;
	elem->Graphics = &PSTN_graphics;
	elem->Init = &PSTN_init_element;
}
