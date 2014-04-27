/**
 * Powder Toy - particle simulation
 *
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

#include <stdint.h>
#include <math.h>
#include "defines.h"
#include "powder.h"
#include "air.h"
#include "misc.h"
#include "gravity.h"
#include "interface.h"
#ifdef LUACONSOLE
#include "luaconsole.h"
#endif

#include "game/Brush.h"
#include "simulation/Simulation.h"
#include "simulation/Tool.h"
#include "simulation/WallNumbers.h"
#include "simulation/GolNumbers.h"
#include "simulation/ElementDataContainer.h"
#include "simulation/elements/PRTI.h"

part_type ptypes[PT_NUM];
part_transition ptransitions[PT_NUM];
unsigned int platent[PT_NUM];

int force_stacking_check = 0;//whether to force a check for excessively stacked particles

playerst player;
playerst player2;

particle *parts;
particle *cb_parts;

int airMode = 0;
int water_equal_test = 0;

unsigned char bmap[YRES/CELL][XRES/CELL];
unsigned char emap[YRES/CELL][XRES/CELL];

unsigned char cb_bmap[YRES/CELL][XRES/CELL];
unsigned char cb_emap[YRES/CELL][XRES/CELL];

unsigned pmap[YRES][XRES];
int pmap_count[YRES][XRES];
unsigned cb_pmap[YRES][XRES];
unsigned photons[YRES][XRES];
int NUM_PARTS = 0;

int GRAV;
int GRAV_R;
int GRAV_G;
int GRAV_B;
int GRAV_R2;
int GRAV_G2;
int GRAV_B2;
int GSPEED = 1;
int GENERATION = 0;
int CGOL = 0;

int love[XRES/9][YRES/9];
int lolz[XRES/9][YRES/9];
unsigned char gol[YRES][XRES];
short gol2[YRES][XRES][9];

float msvx[256], msvy[256], msrotation[256], newmsrotation[256];
int msindex[256], msnum[256], numballs = 0, ms_rotation = 1;

void get_gravity_field(int x, int y, float particleGrav, float newtonGrav, float *pGravX, float *pGravY)
{
	int angle;
	*pGravX = newtonGrav*gravx[(y/CELL)*(XRES/CELL)+(x/CELL)];
	*pGravY = newtonGrav*gravy[(y/CELL)*(XRES/CELL)+(x/CELL)];
	switch (gravityMode)
	{
		default:
		case 0: //normal, vertical gravity
			*pGravY += particleGrav;
			break;
		case 1: //no gravity
			angle = rand()%360;
			*pGravX -= cosf((float)angle);
			*pGravY -= sinf((float)angle);
			break;
		case 2: //radial gravity
			if (x-XCNTR != 0 || y-YCNTR != 0)
			{
				float pGravMult = particleGrav/sqrtf((float)((x-XCNTR)*(x-XCNTR) + (y-YCNTR)*(y-YCNTR)));
				*pGravX -= pGravMult * (float)(x - XCNTR);
				*pGravY -= pGravMult * (float)(y - YCNTR);
			}
	}
}

static int pn_junction_sprk(int x, int y, int pt)
{
	unsigned r = pmap[y][x];
	if ((r & 0xFF) != pt)
		return 0;
	r >>= 8;
	if (parts[r].type != pt)
		return 0;
	return globalSim->spark_conductive_attempt(r, x, y);
}

static void photoelectric_effect(int nx, int ny)//create sparks from PHOT when hitting PSCN and NSCN
{
	unsigned r = pmap[ny][nx];

	if ((r&0xFF) == PT_PSCN) {
		if ((pmap[ny][nx-1] & 0xFF) == PT_NSCN ||
		        (pmap[ny][nx+1] & 0xFF) == PT_NSCN ||
		        (pmap[ny-1][nx] & 0xFF) == PT_NSCN ||
		        (pmap[ny+1][nx] & 0xFF) == PT_NSCN)
			pn_junction_sprk(nx, ny, PT_PSCN);
	}
}


int IsWallBlocking(int x, int y, int type)
{
	if (bmap[y/CELL][x/CELL])
	{
		int wall = bmap[y/CELL][x/CELL];
		if (wall == WL_ALLOWGAS && !(ptypes[type].properties&TYPE_GAS))
			return 1;
		else if (wall == WL_ALLOWENERGY && !(ptypes[type].properties&TYPE_ENERGY))
			return 1;
		else if (wall == WL_ALLOWLIQUID && ptypes[type].falldown!=2)
			return 1;
		else if (wall == WL_ALLOWSOLID && ptypes[type].falldown!=1)
			return 1;
		else if (wall == WL_ALLOWAIR || wall == WL_WALL || wall == WL_WALLELEC)
			return 1;
		else if (wall == WL_EWALL && !emap[y/CELL][x/CELL])
			return 1;
	}
	return 0;
}

unsigned char can_move[PT_NUM][PT_NUM];

void init_can_move()
{
	int movingType, destinationType;
	// can_move[moving type][type at destination]
	//  0 = No move/Bounce
	//  1 = Swap
	//  2 = Both particles occupy the same space.
	//  3 = Varies, go run some extra checks

	// particles that don't exist shouldn't move...
	for (destinationType = 0; destinationType < PT_NUM; destinationType++)
		can_move[0][destinationType] = 0;

	//initialize everything else to swapping by default 
	for (movingType = 1; movingType < PT_NUM; movingType++)
		for (destinationType = 0; destinationType < PT_NUM; destinationType++)
			can_move[movingType][destinationType] = 1;

	//photons go through everything by default 
	for (destinationType = 1; destinationType < PT_NUM; destinationType++)
		can_move[PT_PHOT][destinationType] = 2;

	for (movingType = 1; movingType < PT_NUM; movingType++)
	{
		for (destinationType = 1; destinationType < PT_NUM; destinationType++)
		{
			// weight check, also prevents particles of same type displacing each other
			if (ptypes[movingType].weight <= ptypes[destinationType].weight || destinationType == PT_GEL)
				can_move[movingType][destinationType] = 0;

			//other checks for NEUT and energy particles
			if (movingType == PT_NEUT && ptypes[destinationType].properties&PROP_NEUTPASS)
				can_move[movingType][destinationType] = 2;
			if (movingType == PT_NEUT && ptypes[destinationType].properties&PROP_NEUTABSORB)
				can_move[movingType][destinationType] = 1;
			if (movingType == PT_NEUT && ptypes[destinationType].properties&PROP_NEUTPENETRATE)
				can_move[movingType][destinationType] = 1;
			if (ptypes[movingType].properties&PROP_NEUTPENETRATE && destinationType == PT_NEUT)
				can_move[movingType][destinationType] = 0;
			if (ptypes[movingType].properties&TYPE_ENERGY && ptypes[destinationType].properties&TYPE_ENERGY)
				can_move[movingType][destinationType] = 2;
			if (ptypes[destinationType].properties&PROP_INDESTRUCTIBLE)
				can_move[movingType][destinationType] = 0;
		}
	}
	for (destinationType = 0; destinationType < PT_NUM; destinationType++)
	{
		//set what stickmen can move through
		int stkm_move = 0;
		if (ptypes[destinationType].properties & (TYPE_LIQUID | TYPE_GAS))
			stkm_move = 2;
		if (!destinationType || destinationType == PT_PRTO || destinationType == PT_SPAWN || destinationType == PT_SPAWN2)
			stkm_move = 2;
		can_move[PT_STKM][destinationType] = stkm_move;
		can_move[PT_STKM2][destinationType] = stkm_move;
		can_move[PT_FIGH][destinationType] = stkm_move;

		//spark shouldn't move
		can_move[PT_SPRK][destinationType] = 0;
	}
	for (movingType = 1; movingType < PT_NUM; movingType++)
	{
		//everything "swaps" with VACU and BHOL to make them eat things
		can_move[movingType][PT_BHOL] = 1;
		can_move[movingType][PT_NBHL] = 1;
		//nothing goes through stickmen
		can_move[movingType][PT_STKM] = 0;
		can_move[movingType][PT_STKM2] = 0;
		can_move[movingType][PT_FIGH] = 0;
		//INVS behavior varies with pressure
		can_move[movingType][PT_INVIS] = 3;
		can_move[movingType][PT_PINV] = 3;
		//stop CNCT from being displaced by other particles
		can_move[movingType][PT_CNCT] = 0;
		//VOID and PVOD behavior varies with powered state and ctype
		can_move[movingType][PT_PVOD] = 3;
		can_move[movingType][PT_VOID] = 3;
		//nothing moves through EMBR (not sure why, but it's killed when it touches anything)
		can_move[movingType][PT_EMBR] = 0;
		can_move[PT_EMBR][movingType] = 0;
		//Energy particles move through VIBR and BVBR, so it can absorb them
		if (ptypes[movingType].properties&TYPE_ENERGY)
		{
			can_move[movingType][PT_VIBR] = 1;
			can_move[movingType][PT_BVBR] = 1;
		}
		if (ptypes[movingType].properties&PROP_MOVS)
			can_move[movingType][movingType] = 2;
	}
	//a list of lots of things PHOT can move through
	for (destinationType = 0; destinationType < PT_NUM; destinationType++)
	{
		if (destinationType == PT_GLAS || destinationType == PT_PHOT || destinationType == PT_FILT || destinationType == PT_H2
		 || destinationType == PT_WATR || destinationType == PT_DSTW || destinationType == PT_SLTW || destinationType == PT_GLOW
		 || destinationType == PT_ISOZ || destinationType == PT_ISZS || destinationType == PT_QRTZ || destinationType == PT_PQRT
		 || destinationType == PT_INVIS || destinationType == PT_PINV
		 || (ptypes[destinationType].properties&PROP_CLONE) || (ptypes[destinationType].properties&PROP_BREAKABLECLONE))
			can_move[PT_PHOT][destinationType] = 2;
		if (destinationType != PT_DMND && destinationType != PT_INSL && destinationType != PT_VOID && destinationType != PT_PVOD && destinationType != PT_VIBR)
		{
			can_move[PT_PROT][destinationType] = 2;
			can_move[PT_GRVT][destinationType] = 2;
		}
	}

	//other special cases that weren't covered above
	can_move[PT_DEST][PT_DMND] = 0;
	can_move[PT_DEST][PT_CLNE] = 0;
	can_move[PT_DEST][PT_PCLN] = 0;
	can_move[PT_DEST][PT_BCLN] = 0;
	can_move[PT_DEST][PT_PBCN] = 0;

	can_move[PT_NEUT][PT_INVIS] = 2; 
	can_move[PT_ELEC][PT_PINV] = 2;
	can_move[PT_ELEC][PT_LCRY] = 2;
	can_move[PT_ELEC][PT_EXOT] = 2;
	can_move[PT_PHOT][PT_LCRY] = 3; //varies according to LCRY life
	
	can_move[PT_PHOT][PT_BIZR] = 2;
	can_move[PT_ELEC][PT_BIZR] = 2;
	can_move[PT_PHOT][PT_BIZRG] = 2;
	can_move[PT_ELEC][PT_BIZRG] = 2;
	can_move[PT_PHOT][PT_BIZRS] = 2;
	can_move[PT_ELEC][PT_BIZRS] = 2;
	can_move[PT_BIZR][PT_FILT] = 2;
	can_move[PT_BIZRG][PT_FILT] = 2;
	
	can_move[PT_ANAR][PT_WHOL] = 1; //WHOL eats ANAR
	can_move[PT_ANAR][PT_NWHL] = 1;
	can_move[PT_ELEC][PT_DEUT] = 1;
	can_move[PT_SPNG][PT_SPNG] = 3;
	can_move[PT_RAZR][PT_CNCT] = 1;
	can_move[PT_THDR][PT_THDR] = 2;
	can_move[PT_EMBR][PT_EMBR] = 2;
	can_move[PT_TRON][PT_SWCH] = 3;
}

/*
   RETURN-value explanation
1 = Swap
0 = No move/Bounce
2 = Both particles occupy the same space.
 */
int eval_move(int pt, int nx, int ny, unsigned *rr)
{
	unsigned r;
	int result;

	if (nx<0 || ny<0 || nx>=XRES || ny>=YRES)
		return 0;

	r = pmap[ny][nx];
	if (r)
		r = (r&~0xFF) | parts[r>>8].type;
	if ((r&0xFF) == PT_PINV && parts[r>>8].tmp2)
		r = parts[r>>8].tmp2;
	if (rr)
		*rr = r;
	if (pt>=PT_NUM || (r&0xFF)>=PT_NUM)
		return 0;
	result = can_move[pt][r&0xFF];
	if (result==3)
	{
		if ((pt==PT_PHOT || pt==PT_ELEC) && (r&0xFF)==PT_LCRY)
			result = (parts[r>>8].life > 5)? 2 : 0;
		if ((r&0xFF)==PT_INVIS)
		{
			if (pv[ny/CELL][nx/CELL]>4.0f || pv[ny/CELL][nx/CELL]<-4.0f) result = 2;
			else result = 0;
		}
		else if ((r&0xFF)==PT_PINV)
		{
			if (parts[r>>8].life >= 10) result = 2;
			else result = 0;
		}
		else if ((r&0xFF)==PT_PVOD)
		{
			if (parts[r>>8].life == 10)
			{
				if(!parts[r>>8].ctype || (parts[r>>8].ctype==pt)!=(parts[r>>8].tmp&1))
					result = 1;
				else
					result = 0;
			}
			else result = 0;
		}
		else if ((r&0xFF)==PT_VOID)
		{
			if(!parts[r>>8].ctype || (parts[r>>8].ctype==pt)!=(parts[r>>8].tmp&1))
				result = 1;
			else
				result = 0;
		}
		else if (pt == PT_TRON && (r&0xFF) == PT_SWCH)
		{
			if (parts[r>>8].life >= 10)
				return 2;
			else
				return 0;
		}
		else if ((r&0xFF)==PT_SPNG)
		{
			if (parts[r>>8].vx == 0 && parts[r>>8].vy == 0) result = 0;
			else result = 2;
		}
		else if (pt == PT_SPNG)
		{
			int vx = (int)parts[pt].vx;
			int vy = (int)parts[pt].vy;
			parts[r>>8].x += vy;
			parts[r>>8].x += vy;
			result = 2;
		}
	}
	if (bmap[ny/CELL][nx/CELL])
	{
		if (bmap[ny/CELL][nx/CELL]==WL_ALLOWGAS && !(ptypes[pt].properties&TYPE_GAS))// && ptypes[pt].falldown!=0 && pt!=PT_FIRE && pt!=PT_SMKE)
			return 0;
		if (bmap[ny/CELL][nx/CELL]==WL_ALLOWENERGY && !(ptypes[pt].properties&TYPE_ENERGY))// && ptypes[pt].falldown!=0 && pt!=PT_FIRE && pt!=PT_SMKE)
			return 0;
		if (bmap[ny/CELL][nx/CELL]==WL_ALLOWLIQUID && ptypes[pt].falldown!=2)
			return 0;
		if (bmap[ny/CELL][nx/CELL]==WL_ALLOWSOLID && ptypes[pt].falldown!=1)
			return 0;
		if (bmap[ny/CELL][nx/CELL]==WL_ALLOWAIR || bmap[ny/CELL][nx/CELL]==WL_WALL || bmap[ny/CELL][nx/CELL]==WL_WALLELEC)
			return 0;
		if (bmap[ny/CELL][nx/CELL]==WL_EWALL && !emap[ny/CELL][nx/CELL])
			return 0;
		if (bmap[ny/CELL][nx/CELL]==WL_EHOLE && !emap[ny/CELL][nx/CELL] && !(ptypes[pt].properties&TYPE_SOLID) && !(ptypes[r&0xFF].properties&TYPE_SOLID))
			return 2;
	}
	return result;
}

int try_move(int i, int x, int y, int nx, int ny)
{
	unsigned r, e;

	if (x==nx && y==ny)
		return 1;
	if (nx<0 || ny<0 || nx>=XRES || ny>=YRES)
		return 1;

	e = eval_move(parts[i].type, nx, ny, &r);

	/* half-silvered mirror */
	if (!e && parts[i].type==PT_PHOT &&
	        (((r&0xFF)==PT_BMTL && rand()<RAND_MAX/2) ||
	         (pmap[y][x]&0xFF)==PT_BMTL))
		e = 2;

	if (!e) //if no movement
	{
		if (!(ptypes[parts[i].type].properties & TYPE_ENERGY))
			return 0;
		if (!legacy_enable && parts[i].type==PT_PHOT && r)//PHOT heat conduction
		{
			if ((r & 0xFF) == PT_COAL || (r & 0xFF) == PT_BCOL)
				parts[r>>8].temp = parts[i].temp;

			if ((r & 0xFF) < PT_NUM && ptypes[r&0xFF].hconduct && ((r&0xFF)!=PT_HSWC||parts[r>>8].life==10) && (r&0xFF)!=PT_FILT)
				parts[i].temp = parts[r>>8].temp = restrict_flt((parts[r>>8].temp+parts[i].temp)/2, MIN_TEMP, MAX_TEMP);
		}
		else if ((parts[i].type==PT_NEUT || parts[i].type==PT_ELEC) && ((ptypes[r&0xFF].properties&PROP_CLONE) || (ptypes[r&0xFF].properties&PROP_BREAKABLECLONE))) {
			if (!parts[r>>8].ctype)
				parts[r>>8].ctype = parts[i].type;
		}
		if (((r&0xFF)==PT_PRTI || (r&0xFF)==PT_PPTI) && (ptypes[parts[i].type].properties & TYPE_ENERGY))
		{
			PortalChannel *channel = ((PRTI_ElementDataContainer*)globalSim->elementData[PT_PRTI])->GetParticleChannel(globalSim, r>>8);
			int slot = PRTI_ElementDataContainer::GetSlot(x-nx,y-ny);
			if (channel->StoreParticle(globalSim, i, slot))
				return -1;
		}
		return 0;
	}

	if (parts[i].type == PT_SPNG)
	{
		int vx = (int)parts[i].vx, vy = (int)parts[i].vy, x2, y2;
		int vx2 = vx, vy2 = vy;
		unsigned int r2;
		if (vx > 0) vx2 = -1; else if (vx < 0) vx2 = 1;
		if (vy > 0) vy2 = -1; else if (vy < 0) vy2 = 1;
		x2 = x + vx2;
		y2 = y + vy2;
		r2 = pmap[y2][x2];
		while ((r2&0xFF) && ((r2&0xFF) != PT_SPNG) && !(ptypes[r2&0xFF].properties&PROP_INDESTRUCTIBLE) && (vx2 || vy2))
		{
			parts[r2>>8].x += vx;
			parts[r2>>8].y += vy;
			x2 += vx2;
			y2 += vy2;
			r2 = pmap[y2][x2];
		}
	}
	if (e == 2) //if occupy same space
	{
		if (parts[i].type == PT_PHOT)
		{
			if ((r&0xFF) == PT_GLOW)
			{
				if (!parts[r>>8].life && rand() < RAND_MAX/30)
				{
					parts[r>>8].life = 120;
					create_gain_photon(i);
				}
			}
			else if ((r&0xFF) == PT_FILT)
			{
				parts[i].ctype = interactWavelengths(&parts[r>>8], parts[i].ctype);
			}
			else if ((r&0xFF) == PT_INVIS)
			{
				if (pv[ny/CELL][nx/CELL]<=4.0f && pv[ny/CELL][nx/CELL]>=-4.0f)
				{
					part_change_type(i, x, y, PT_NEUT);
					parts[i].ctype = 0;
				}
				else if ((r&0xFF) == PT_PINV)
				{
					if (!parts[r>>8].life)
					{
						part_change_type(i, x, y, PT_ELEC);
						parts[i].ctype = 0;
					}
				}
			}
			else if ((r&0xFF)==PT_BIZR || (r&0xFF)==PT_BIZRG || (r&0xFF)==PT_BIZRS)
			{
				part_change_type(i, x, y, PT_ELEC);
				parts[i].ctype = 0;
			}
			else if ((r&0xFF) == PT_H2 && !(parts[i].tmp&0x1))
			{
				part_change_type(i, x, y, PT_PROT);
				parts[i].ctype = 0;
				parts[i].tmp2 = 0x1;

				create_part(r>>8, x, y, PT_ELEC);
				return -1;
			}
		}
		else if (parts[i].type == PT_NEUT)
		{
			if ((r&0xFF) == PT_GLAS)
			{
				if (rand() < RAND_MAX/10)
					create_cherenkov_photon(i);
			}
		}
		else if (parts[i].type==PT_BIZR || parts[i].type==PT_BIZRG)
		{
			if ((r&0xFF) == PT_FILT)
				parts[i].ctype = interactWavelengths(&parts[r>>8], parts[i].ctype);
		}
		else if (parts[i].type == PT_PROT)
		{
			if ((r&0xFF) == PT_INVIS)
				part_change_type(i, x, y, PT_NEUT);
		}
		return 1;
	}
	//else e=1 , we are trying to swap the particles, return 0 no swap/move, 1 is still overlap/move, because the swap takes place later

	if ((r&0xFF)==PT_VOID || (r&0xFF)==PT_PVOD) //this is where void eats particles
	{
		//void ctype already checked in eval_move
		kill_part(i);
		return 0;
	}
	else if ((r&0xFF)==PT_BHOL || (r&0xFF)==PT_NBHL) //this is where blackhole eats particles
	{
		kill_part(i);
		if (!legacy_enable)
		{
			parts[r>>8].temp = restrict_flt(parts[r>>8].temp+parts[i].temp/2, MIN_TEMP, MAX_TEMP);//3.0f;
		}
		return 0;
	}
	else if ((r&0xFF)==PT_WHOL || (r&0xFF)==PT_NWHL) //whitehole eats anar
	{
		if (parts[i].type == PT_ANAR)
		{
			if (!legacy_enable)
			{
				parts[r>>8].temp = restrict_flt(parts[r>>8].temp- (MAX_TEMP-parts[i].temp)/2, MIN_TEMP, MAX_TEMP);
			}
			kill_part(i);
			return 0;
		}
	}
	else if ((r&0xFF) == PT_DEUT)
	{
		if (parts[i].type == PT_ELEC)
		{
			if (parts[r>>8].life < 6000)
				parts[r>>8].life += 1;
			parts[r>>8].temp = 0;
			kill_part(i);
			return 0;
		}
	}
	else if ((r&0xFF)==PT_VIBR || (r&0xFF)==PT_BVBR)
	{
		if (ptypes[parts[i].type].properties & TYPE_ENERGY)
		{
			parts[r>>8].tmp += 20;
			kill_part(i);
			return 0;
		}
	}

	if (parts[i].type == PT_NEUT)
	{
		if (ptypes[r&0xFF].properties & PROP_NEUTABSORB)
		{
			kill_part(i);
			return 0;
		}
	}
	else if (parts[i].type == PT_CNCT)
	{
		//check below CNCT for another CNCT
		if (y<ny && (pmap[y+1][x]&0xFF)==PT_CNCT)
			return 0;
	}
	else if (parts[i].type == PT_GBMB)
	{
		if (parts[i].life > 0)
			return 0;
	}

	if ((bmap[y/CELL][x/CELL]==WL_EHOLE && !emap[y/CELL][x/CELL]) && !(bmap[ny/CELL][nx/CELL]==WL_EHOLE && !emap[ny/CELL][nx/CELL]))
		return 0;

	e = r >> 8; //e is now the particle number at r (pmap[ny][nx])
	if (r)//the swap part, if we make it this far, swap
	{
		if (parts[i].type==PT_NEUT) {
			// target material is NEUTPENETRATE, meaning it gets moved around when neutron passes
			unsigned s = pmap[y][x];
			if (s && !(ptypes[s&0xFF].properties&PROP_NEUTPENETRATE))
				return 1; // if the element currently underneath neutron isn't NEUTPENETRATE, don't move anything except the neutron
			// if nothing is currently underneath neutron, only move target particle
			if (bmap[y/CELL][x/CELL] == WL_ALLOWENERGY)
				return 1; // do not drag target particle into an energy only wall
			if (s)
			{
				pmap[ny][nx] = (s&~(0xFF))|parts[s>>8].type;
				parts[s>>8].x = (float)nx;
				parts[s>>8].y = (float)ny;
			}
			else pmap[ny][nx] = 0;
			parts[e].x = (float)x;
			parts[e].y = (float)y;
			pmap[y][x] = (e<<8)|parts[e].type;
			return 1;
		}

		if (!OutOfBounds((int)(parts[e].x+0.5f)+x-nx, (int)(parts[e].y+0.5f)+y-ny))
		{
			if (!OutOfBounds(nx, ny) && (pmap[ny][nx]>>8)==e) pmap[ny][nx] = 0;
			parts[e].x += x-nx;
			parts[e].y += y-ny;
			pmap[(int)(parts[e].y+0.5f)][(int)(parts[e].x+0.5f)] = (e<<8)|parts[e].type;
		}
	}
	return 1;
}

int OutOfBounds(int x, int y)
{
	if (edgeMode != 3)
		return (x < CELL || x >= XRES-CELL || y < CELL || y >= YRES-CELL);
	else
		return (x < 0 || x >= XRES || y < 0 || y >= YRES);
}

// try to move particle, and if successful call move() to update variables
int do_move(int i, int x, int y, float nxf, float nyf)
{
	// volatile to hopefully force truncation of floats in x87 registers by storing and reloading from memory, so that rounding issues don't cause particles to appear in the wrong pmap list. If using -mfpmath=sse or an ARM CPU, this may be unnecessary.
	volatile float tmpx = nxf, tmpy = nyf;
	int nx = (int)(tmpx+0.5f), ny = (int)(tmpy+0.5f), result;

	if (edgeMode == 2)
	{
		if (nx < CELL)
			nxf += XRES-CELL*2;
		if (nx >= XRES-CELL)
			nxf -= XRES-CELL*2;
		if (ny < CELL)
			nyf += YRES-CELL*2;
		if (ny >= YRES-CELL)
			nyf -= YRES-CELL*2;
		nx = (int)(nxf+0.5f);
		ny = (int)(nyf+0.5f);
	}
	if (parts[i].type == PT_NONE)
		return 0;
	result = try_move(i, x, y, nx, ny);
	if (result && move(i,x,y,nxf,nyf))
		return -1;
	return result;
}

//update pmap and parts[i].x,y
int move(int i, int x, int y, float nxf, float nyf)
{
	// volatile to hopefully force truncation of floats in x87 registers by storing and reloading from memory, so that rounding issues don't cause particles to appear in the wrong pmap list. If using -mfpmath=sse or an ARM CPU, this may be unnecessary.
	volatile float tmpx = nxf, tmpy = nyf;
	int nx = (int)(tmpx+0.5f), ny = (int)(tmpy+0.5f), t = parts[i].type;
	parts[i].x = nxf;
	parts[i].y = nyf;
	if (ny!=y || nx!=x)
	{
		if ((pmap[y][x]>>8)==i) pmap[y][x] = 0;
		else if ((pmap[y][x]&0xFF)==PT_PINV && (parts[pmap[y][x]>>8].tmp2>>8)==i) parts[pmap[y][x]>>8].tmp2 = 0;
		else if ((photons[y][x]>>8)==i) photons[y][x] = 0;
		if (OutOfBounds(nx, ny))//kill_part if particle is out of bounds
		{
			if (!(ptypes[t].properties&PROP_MOVS) || (parts[i].tmp2 < 0 || parts[i].tmp2 > 255 || !msindex[parts[i].tmp2]))
				kill_part(i);
			return -1;
		}
		if (ptypes[t].properties & TYPE_ENERGY)
			photons[ny][nx] = t|(i<<8);
		else if (t && (pmap[ny][nx]&0xFF) != PT_PINV && (!(ptypes[t].properties&PROP_MOVS) || !(pmap[ny][nx]&0xFF) || (ptypes[pmap[ny][nx]&0xFF].properties&PROP_MOVS)))
			pmap[ny][nx] = t|(i<<8);
		else if (t && (pmap[ny][nx]&0xFF) == PT_PINV)
			parts[pmap[ny][nx]>>8].tmp2 = t|(i<<8);
	}
	return 0;
}

static unsigned direction_to_map(float dx, float dy, int t)
{
	// TODO:
	// Adding extra directions causes some inaccuracies.
	// Not adding them causes problems with some diagonal surfaces (photons absorbed instead of reflected).
	// For now, don't add them.
	// Solution may involve more intelligent setting of initial i0 value in find_next_boundary?
	// or rewriting normal/boundary finding code

	return (dx >= 0) |
	       (((dx + dy) >= 0) << 1) |     /*  567  */
	       ((dy >= 0) << 2) |            /*  4+0  */
	       (((dy - dx) >= 0) << 3) |     /*  321  */
	       ((dx <= 0) << 4) |
	       (((dx + dy) <= 0) << 5) |
	       ((dy <= 0) << 6) |
	       (((dy - dx) <= 0) << 7);
	/*
	return (dx >= -0.001) |
	       (((dx + dy) >= -0.001) << 1) |     //  567
	       ((dy >= -0.001) << 2) |            //  4+0
	       (((dy - dx) >= -0.001) << 3) |     //  321
	       ((dx <= 0.001) << 4) |
	       (((dx + dy) <= 0.001) << 5) |
	       ((dy <= 0.001) << 6) |
	       (((dy - dx) <= 0.001) << 7);
	}*/
}

static int is_blocking(int t, int x, int y)
{
	if (t & REFRACT) {
		if (x<0 || y<0 || x>=XRES || y>=YRES)
			return 0;
		if ((pmap[y][x] & 0xFF) == PT_GLAS)
			return 1;
		return 0;
	}

	return !eval_move(t, x, y, NULL);
}

static int is_boundary(int pt, int x, int y)
{
	if (!is_blocking(pt,x,y))
		return 0;
	if (is_blocking(pt,x,y-1) && is_blocking(pt,x,y+1) && is_blocking(pt,x-1,y) && is_blocking(pt,x+1,y))
		return 0;
	return 1;
}

static int find_next_boundary(int pt, int *x, int *y, int dm, int *em)
{
	static int dx[8] = {1,1,0,-1,-1,-1,0,1};
	static int dy[8] = {0,1,1,1,0,-1,-1,-1};
	static int de[8] = {0x83,0x07,0x0E,0x1C,0x38,0x70,0xE0,0xC1};
	int i, ii, i0;

	if (*x <= 0 || *x >= XRES-1 || *y <= 0 || *y >= YRES-1)
		return 0;

	if (*em != -1) {
		i0 = *em;
		dm &= de[i0];
	} else
		i0 = 0;

	for (ii=0; ii<8; ii++) {
		i = (ii + i0) & 7;
		if ((dm & (1 << i)) && is_boundary(pt, *x+dx[i], *y+dy[i])) {
			*x += dx[i];
			*y += dy[i];
			*em = i;
			return 1;
		}
	}

	return 0;
}

int get_normal(int pt, int x, int y, float dx, float dy, float *nx, float *ny)
{
	int ldm, rdm, lm, rm;
	int lx, ly, lv, rx, ry, rv;
	int i, j;
	float r, ex, ey;

	if (!dx && !dy)
		return 0;

	if (!is_boundary(pt, x, y))
		return 0;

	ldm = direction_to_map(-dy, dx, pt);
	rdm = direction_to_map(dy, -dx, pt);
	lx = rx = x;
	ly = ry = y;
	lv = rv = 1;
	lm = rm = -1;

	j = 0;
	for (i=0; i<SURF_RANGE; i++) {
		if (lv)
			lv = find_next_boundary(pt, &lx, &ly, ldm, &lm);
		if (rv)
			rv = find_next_boundary(pt, &rx, &ry, rdm, &rm);
		j += lv + rv;
		if (!lv && !rv)
			break;
	}

	if (j < NORMAL_MIN_EST)
		return 0;

	if ((lx == rx) && (ly == ry))
		return 0;

	ex = (float)rx - lx;
	ey = (float)ry - ly;
	r = 1.0f/hypot(ex, ey);
	*nx =  ey * r;
	*ny = -ex * r;

	return 1;
}

int get_normal_interp(int pt, float x0, float y0, float dx, float dy, float *nx, float *ny)
{
	int x, y, i;

	dx /= NORMAL_FRAC;
	dy /= NORMAL_FRAC;

	for (i=0; i<NORMAL_INTERP; i++) {
		x = (int)(x0 + 0.5f);
		y = (int)(y0 + 0.5f);
		if (is_boundary(pt, x, y))
			break;
		x0 += dx;
		y0 += dy;
	}
	if (i >= NORMAL_INTERP)
		return 0;

	if (pt == PT_PHOT)
		photoelectric_effect(x, y);

	return get_normal(pt, x, y, dx, dy, nx, ny);
}

//For soap only
void detach(int i)
{
	if ((parts[i].ctype&2) == 2)
	{
		if ((parts[parts[i].tmp].ctype&4) == 4)
			parts[parts[i].tmp].ctype ^= 4;
	}

	if ((parts[i].ctype&4) == 4)
	{
		if ((parts[parts[i].tmp2].ctype&2) == 2)
			parts[parts[i].tmp2].ctype ^= 2;
	}

	parts[i].ctype = 0;
}

void kill_part(int i)//kills particle number i
{
	//TODO: replace everything with this
	globalSim->part_kill(i);
}

void part_change_type(int i, int x, int y, int t)//changes the type of particle number i, to t.  This also changes pmap at the same time.
{
	globalSim->part_change_type(i, x, y, t);
}

//the function for creating a particle, use p=-1 for creating a new particle, -2 is from a brush, or a particle number to replace a particle.
//TODO: replace with Simulation::part_create now
int create_part(int p, int x, int y, int tv)
{
	int i;

	int t = tv & 0xFF;
	int v = (tv >> 8) & 0xFFFFFFFF;

	i = globalSim->part_create(p, x, y, t, v);
	return i;
}

static void create_gain_photon(int pp)//photons from PHOT going through GLOW
{
	float xx, yy;
	int i, lr, temp_bin, nx, ny;

	lr = rand() % 2;

	if (lr) {
		xx = parts[pp].x - 0.3*parts[pp].vy;
		yy = parts[pp].y + 0.3*parts[pp].vx;
	} else {
		xx = parts[pp].x + 0.3*parts[pp].vy;
		yy = parts[pp].y - 0.3*parts[pp].vx;
	}

	nx = (int)(xx + 0.5f);
	ny = (int)(yy + 0.5f);

	if (nx<0 || ny<0 || nx>=XRES || ny>=YRES)
		return;

	if ((pmap[ny][nx] & 0xFF) != PT_GLOW)
		return;

	i = globalSim->part_create(-1, nx, ny, PT_PHOT);
	if (i<0)
		return;

	parts[i].life = 680;
	parts[i].x = xx;
	parts[i].y = yy;
	parts[i].vx = parts[pp].vx;
	parts[i].vy = parts[pp].vy;
	parts[i].temp = parts[pmap[ny][nx] >> 8].temp;

	temp_bin = (int)((parts[i].temp-273.0f)*0.25f);
	if (temp_bin < 0) temp_bin = 0;
	if (temp_bin > 25) temp_bin = 25;
	parts[i].ctype = 0x1F << temp_bin;
}

static void create_cherenkov_photon(int pp)//photons from NEUT going through GLAS
{
	int i, lr, nx, ny;
	float r, eff_ior;

	nx = (int)(parts[pp].x + 0.5f);
	ny = (int)(parts[pp].y + 0.5f);
	if ((pmap[ny][nx] & 0xFF) != PT_GLAS)
		return;

	if (hypotf(parts[pp].vx, parts[pp].vy) < 1.44f)
		return;

	i = globalSim->part_create(-1, nx, ny, PT_PHOT);
	if (i<0)
		return;

	lr = rand() % 2;

	parts[i].ctype = 0x00000F80;
	parts[i].life = 680;
	parts[i].x = parts[pp].x;
	parts[i].y = parts[pp].y;
	parts[i].temp = parts[pmap[ny][nx] >> 8].temp;
	parts[i].pavg[0] = parts[i].pavg[1] = 0.0f;

	if (lr) {
		parts[i].vx = parts[pp].vx - 2.5f*parts[pp].vy;
		parts[i].vy = parts[pp].vy + 2.5f*parts[pp].vx;
	} else {
		parts[i].vx = parts[pp].vx + 2.5f*parts[pp].vy;
		parts[i].vy = parts[pp].vy - 2.5f*parts[pp].vx;
	}

	/* photons have speed of light. no discussion. */
	r = 1.269 / hypotf(parts[i].vx, parts[i].vy);
	parts[i].vx *= r;
	parts[i].vy *= r;
}

TPT_INLINE void delete_part(int x, int y, int flags)//calls kill_part with the particle located at x,y
{
	unsigned i;

	if (x<0 || y<0 || x>=XRES || y>=YRES)
		return;
	if (photons[y][x]) {
		i = photons[y][x];
	} else {
		i = pmap[y][x];
	}

	if (!i)
		return;
	if (!(flags&BRUSH_SPECIFIC_DELETE) || parts[i>>8].type == ((ElementTool*)activeTools[2])->GetID() || ((ElementTool*)activeTools[2])->GetID() <= 0)//specific deletion
	{
		kill_part(i>>8);
	}
	else
		return;
}

TPT_INLINE int is_wire(int x, int y)
{
	return bmap[y][x]==WL_DETECT || bmap[y][x]==WL_EWALL || bmap[y][x]==WL_ALLOWLIQUID || bmap[y][x]==WL_WALLELEC || bmap[y][x]==WL_ALLOWALLELEC || bmap[y][x]==WL_EHOLE;
}

TPT_INLINE int is_wire_off(int x, int y)
{
	return (bmap[y][x]==WL_DETECT || bmap[y][x]==WL_EWALL || bmap[y][x]==WL_ALLOWLIQUID || bmap[y][x]==WL_WALLELEC || bmap[y][x]==WL_ALLOWALLELEC || bmap[y][x]==WL_EHOLE) && emap[y][x]<8;
}

int get_wavelength_bin(int *wm)
{
	int i, w0=30, wM=0;

	if (!*wm)
		return -1;

	for (i=0; i<30; i++)
		if (*wm & (1<<i)) {
			if (i < w0)
				w0 = i;
			if (i > wM)
				wM = i;
		}

	if (wM-w0 < 5)
		return (wM+w0)/2;

	i = rand() % (wM-w0-3);
	i += w0;

	*wm &= 0x1F << i;
	return i + 2;
}

void set_emap(int x, int y)
{
	int x1, x2;

	if (!is_wire_off(x, y))
		return;

	// go left as far as possible
	x1 = x2 = x;
	while (x1>0)
	{
		if (!is_wire_off(x1-1, y))
			break;
		x1--;
	}
	while (x2<XRES/CELL-1)
	{
		if (!is_wire_off(x2+1, y))
			break;
		x2++;
	}

	// fill span
	for (x=x1; x<=x2; x++)
		emap[y][x] = 16;

	// fill children

	if (y>1 && x1==x2 &&
	        is_wire(x1-1, y-1) && is_wire(x1, y-1) && is_wire(x1+1, y-1) &&
	        !is_wire(x1-1, y-2) && is_wire(x1, y-2) && !is_wire(x1+1, y-2))
		set_emap(x1, y-2);
	else if (y>0)
		for (x=x1; x<=x2; x++)
			if (is_wire_off(x, y-1))
			{
				if (x==x1 || x==x2 || y>=YRES/CELL-1 ||
				        is_wire(x-1, y-1) || is_wire(x+1, y-1) ||
				        is_wire(x-1, y+1) || !is_wire(x, y+1) || is_wire(x+1, y+1))
					set_emap(x, y-1);
			}

	if (y<YRES/CELL-2 && x1==x2 &&
	        is_wire(x1-1, y+1) && is_wire(x1, y+1) && is_wire(x1+1, y+1) &&
	        !is_wire(x1-1, y+2) && is_wire(x1, y+2) && !is_wire(x1+1, y+2))
		set_emap(x1, y+2);
	else if (y<YRES/CELL-1)
		for (x=x1; x<=x2; x++)
			if (is_wire_off(x, y+1))
			{
				if (x==x1 || x==x2 || y<0 ||
				        is_wire(x-1, y+1) || is_wire(x+1, y+1) ||
				        is_wire(x-1, y-1) || !is_wire(x, y-1) || is_wire(x+1, y-1))
					set_emap(x, y+1);
			}
}

TPT_GNU_INLINE int parts_avg(int ci, int ni,int t)
{
	if (t==PT_INSL)//to keep electronics working
	{
		int pmr = pmap[((int)(parts[ci].y+0.5f) + (int)(parts[ni].y+0.5f))/2][((int)(parts[ci].x+0.5f) + (int)(parts[ni].x+0.5f))/2];
		if (pmr)
			return parts[pmr>>8].type;
		else
			return PT_NONE;
	}
	else
	{
		int pmr2 = pmap[(int)((parts[ci].y + parts[ni].y)/2+0.5f)][(int)((parts[ci].x + parts[ni].x)/2+0.5f)];//seems to be more accurate.
		if (pmr2)
		{
			if (parts[pmr2>>8].type==t)
				return t;
		}
		else
			return PT_NONE;
	}
	return PT_NONE;
}


int nearest_part(int ci, int t, int max_d)
{
	int distance = (int)((max_d!=-1)?max_d:MAX_DISTANCE);
	int ndistance = 0;
	int id = -1;
	int i = 0;
	int cx = (int)parts[ci].x;
	int cy = (int)parts[ci].y;
	for (i = 0; i <= globalSim->parts_lastActiveIndex; i++)
	{
		if ((parts[i].type==t||(t==-1&&parts[i].type))&&!parts[i].life&&i!=ci)
		{
			ndistance = abs(cx-(int)parts[i].x)+abs(cy-(int)parts[i].y);// Faster but less accurate  Older: sqrt(pow(cx-parts[i].x, 2)+pow(cy-parts[i].y, 2));
			if (ndistance<distance)
			{
				distance = ndistance;
				id = i;
			}
		}
	}
	return id;
}

void stacking_check()
{
	int i, t, x, y, excessive_stacking_found;
	if (force_stacking_check || (rand()%10)==0)
	{
		force_stacking_check = 0;
		excessive_stacking_found = 0;
		for (y=0; y<YRES; y++)
		{
			for (x=0; x<XRES; x++)
			{
				// Use a threshold, since some particle stacking can be normal (e.g. BIZR + FILT)
				// Setting pmap_count[y][x] > NPART means BHOL will form in that spot
				if (pmap_count[y][x]>5)
				{
					if (bmap[y/CELL][x/CELL]==WL_EHOLE)
					{
						// Allow more stacking in E-hole
						if (pmap_count[y][x]>1500)
						{
							pmap_count[y][x] = pmap_count[y][x] + NPART;
							excessive_stacking_found = 1;
						}
					}
					// Random chance to turn into BHOL that increases with the amount of stacking, up to a threshold where it is certain to turn into BHOL
					else if (pmap_count[y][x]>1500 || (rand()%1600)<=(pmap_count[y][x]+100))
					{
						pmap_count[y][x] = pmap_count[y][x] + NPART;
						excessive_stacking_found = 1;
					}
				}
			}
		}
		if (excessive_stacking_found)
		{
			for (i = 0; i <= globalSim->parts_lastActiveIndex; i++)
			{
				if (parts[i].type)
				{
					t = parts[i].type;
					x = (int)(parts[i].x+0.5f);
					y = (int)(parts[i].y+0.5f);
					if (x>=0 && y>=0 && x<XRES && y<YRES && !(ptypes[t].properties&TYPE_ENERGY))
					{
						if (pmap_count[y][x]>=NPART)
						{
							if (pmap_count[y][x]>NPART)
							{
								create_part(i, x, y, PT_NBHL);
								parts[i].temp = MAX_TEMP;
								parts[i].tmp = pmap_count[y][x]-NPART;//strength of grav field
								if (parts[i].tmp>51200) parts[i].tmp = 51200;
								pmap_count[y][x] = NPART;
							}
							else
							{
								kill_part(i);
							}
						}
					}
				}
			}
		}
	}
}

void GRAV_update()
{
	int q;
	GRAV ++;
	GRAV_R = 60;
	GRAV_G = 0;
	GRAV_B = 0;
	GRAV_R2 = 30;
	GRAV_G2 = 30;
	GRAV_B2 = 0;
	for ( q = 0; q <= GRAV; q++)
	{
		if (GRAV_R >0 && GRAV_G==0)
		{
			GRAV_R--;
			GRAV_B++;
		}
		if (GRAV_B >0 && GRAV_R==0)
		{
			GRAV_B--;
			GRAV_G++;
		}
		if (GRAV_G >0 && GRAV_B==0)
		{
			GRAV_G--;
			GRAV_R++;
		}
		if (GRAV_R2 >0 && GRAV_G2==0)
		{
			GRAV_R2--;
			GRAV_B2++;
		}
		if (GRAV_B2 >0 && GRAV_R2==0)
		{
			GRAV_B2--;
			GRAV_G2++;
		}
		if (GRAV_G2 >0 && GRAV_B2==0)
		{
			GRAV_G2--;
			GRAV_R2++;
		}
	}
	if (GRAV>180) GRAV = 0;
}

void LOVELOLZ_update()
{
	int nx, nnx, ny, nny, r, rt;
	for (ny=0; ny<YRES-4; ny++)
	{
		for (nx=0; nx<XRES-4; nx++)
		{
			r=pmap[ny][nx];
			if (!r)
			{
				continue;
			}
			else if ((ny<9||nx<9||ny>YRES-7||nx>XRES-10)&&(parts[r>>8].type==PT_LOVE||parts[r>>8].type==PT_LOLZ))
				kill_part(r>>8);
			else if (parts[r>>8].type==PT_LOVE)
			{
				love[nx/9][ny/9] = 1;
			}
			else if (parts[r>>8].type==PT_LOLZ)
			{
				lolz[nx/9][ny/9] = 1;
			}
		}
	}
	for (nx=9; nx<=XRES-18; nx++)
	{
		for (ny=9; ny<=YRES-7; ny++)
		{
			if (love[nx/9][ny/9]==1)
			{
				for ( nnx=0; nnx<9; nnx++)
					for ( nny=0; nny<9; nny++)
					{
						if (ny+nny>0&&ny+nny<YRES&&nx+nnx>=0&&nx+nnx<XRES)
						{
							rt=pmap[ny+nny][nx+nnx];
							if (!rt&&loverule[nnx][nny]==1)
								create_part(-1,nx+nnx,ny+nny,PT_LOVE);
							else if (!rt)
								continue;
							else if (parts[rt>>8].type==PT_LOVE&&loverule[nnx][nny]==0)
								kill_part(rt>>8);
						}
					}
			}
			love[nx/9][ny/9]=0;
			if (lolz[nx/9][ny/9]==1)
			{
				for ( nnx=0; nnx<9; nnx++)
					for ( nny=0; nny<9; nny++)
					{
						if (ny+nny>0&&ny+nny<YRES&&nx+nnx>=0&&nx+nnx<XRES)
						{
							rt=pmap[ny+nny][nx+nnx];
							if (!rt&&lolzrule[nny][nnx]==1)
								create_part(-1,nx+nnx,ny+nny,PT_LOLZ);
							else if (!rt)
								continue;
							else if (parts[rt>>8].type==PT_LOLZ&&lolzrule[nny][nnx]==0)
								kill_part(rt>>8);

						}
					}
			}
			lolz[nx/9][ny/9]=0;
		}
	}
}

void WIRE_update()
{
	int nx, ny, r;
	for (nx=0; nx<XRES; nx++)
	{
		for (ny=0; ny<YRES; ny++)
		{
			r = pmap[ny][nx];
			if (!r)
				continue;
			if(parts[r>>8].type == PT_WIRE)
				parts[r>>8].tmp = parts[r>>8].ctype;
		}
	}
}

void PPIP_update()
{
	for (int i = 0; i <= globalSim->parts_lastActiveIndex; i++)
	{
		if (parts[i].type == PT_PPIP)
		{
			parts[i].tmp |= (parts[i].tmp&0xE0000000)>>3;
			parts[i].tmp &= ~0xE0000000;
		}
	}
	ppip_changed = 0;
}

void LIFE_update()
{
	int i, nx, ny, nnx, nny, r, rt, z, golnum, neighbors, createdsomething = 0;
	CGOL=0;
	for (ny=CELL; ny<YRES-CELL; ny++)
	{//go through every particle and set neighbor map
		for (nx=CELL; nx<XRES-CELL; nx++)
		{
			r = pmap[ny][nx];
			if (!r)
			{
				gol[ny][nx] = 0;
				continue;
			}
			if ((r&0xFF)==PT_LIFE)
			{
				golnum = parts[r>>8].ctype+1;
				if (golnum<=0 || golnum>NGOL) {
					kill_part(r>>8);
					continue;
				}
				gol[ny][nx] = golnum;
				if (parts[r>>8].tmp == grule[golnum][9]-1) {
					for (nnx=-1; nnx<2; nnx++)
					{
						for (nny=-1; nny<2; nny++) //it will count itself as its own neighbor, which is needed, but will have 1 extra for delete check
						{
							int adx = ((nx+nnx+XRES-3*CELL)%(XRES-2*CELL))+CELL;
							int ady = ((ny+nny+YRES-3*CELL)%(YRES-2*CELL))+CELL;
							rt = pmap[ady][adx];
							if (!rt || (rt&0xFF)==PT_LIFE)
							{
								//the total neighbor count is in 0
								gol2[ady][adx][0] ++;
								//insert golnum into neighbor table
								for (i=1; i<9; i++)
								{
									if (!gol2[ady][adx][i])
									{
										gol2[ady][adx][i] = (golnum<<4)+1;
										break;
									}
									else if((gol2[ady][adx][i]>>4)==golnum)
									{
										gol2[ady][adx][i]++;
										break;
									}
								}
							}
						}
					}
				} else {
					parts[r>>8].tmp --;
				}
			}
		}
	}
	for (ny=CELL; ny<YRES-CELL; ny++)
	{ //go through every particle again, but check neighbor map, then update particles
		for (nx=CELL; nx<XRES-CELL; nx++)
		{
			r = pmap[ny][nx];
			if (r && (r&0xFF)!=PT_LIFE)
				continue;
			neighbors = gol2[ny][nx][0];
			if (neighbors)
			{
				golnum = gol[ny][nx];
				if (!r)
				{
					//Find which type we can try and create
					int creategol = 0xFF;
					for ( i=1; i<9; i++)
					{
						if (!gol2[ny][nx][i]) break;
						golnum = (gol2[ny][nx][i]>>4);
						if (grule[golnum][neighbors]>=2 && (gol2[ny][nx][i]&0xF)>=(neighbors%2)+neighbors/2)
						{
							if (golnum<creategol) creategol=golnum;
						}
					}
					if (creategol<0xFF)
						if (create_part(-1, nx, ny, PT_LIFE|((creategol-1)<<8)) > -1)
							createdsomething = 1;
				}
				else if (grule[golnum][neighbors-1]==0 || grule[golnum][neighbors-1]==2)//subtract 1 because it counted itself
				{
					if (parts[r>>8].tmp==grule[golnum][9]-1)
						parts[r>>8].tmp --;
				}
				for (z = 0; z<9; z++)
					gol2[ny][nx][z] = 0;//this improves performance A LOT compared to the memset, i was getting ~23 more fps with this.
			}
			//we still need to kill things with 0 neighbors (higher state life)
			if (r && parts[r>>8].tmp<=0)
				kill_part(r>>8);
		}
	}
	if (createdsomething)
		GENERATION ++;
}

void decrease_life(int i)
{
	int t;
	unsigned int elem_properties;
	if (parts[i].type)
	{
		t = parts[i].type;
#ifdef OGLR
		parts[i].lastX = parts[i].x;
		parts[i].lastY = parts[i].y;
#endif
		if (t<0 || t>=PT_NUM)
		{
			kill_part(i);
			return;
		}
		elem_properties = ptypes[t].properties;
		if (parts[i].life>0 && (elem_properties&PROP_LIFE_DEC))
		{
			// automatically decrease life
			parts[i].life--;
			if (parts[i].life<=0 && (elem_properties&(PROP_LIFE_KILL_DEC|PROP_LIFE_KILL)))
			{
				// kill on change to no life
				kill_part(i);
				return;
			}
		}
		else if (parts[i].life<=0 && (elem_properties&PROP_LIFE_KILL))
		{
			// kill if no life
			kill_part(i);
			return;
		}
	}
}

int transfer_heat(int i, int surround[8])
{
	int t = parts[i].type, x = (int)(parts[i].x+0.5f), y = (int)(parts[i].y+0.5f);
	int j, r, rt, s, h_count = 0, surround_hconduct[8];
	float gel_scale = 1.0f, ctemph, ctempl, swappage, pt = R_TEMP, c_heat = 0.0f;

	if (t == PT_GEL)
		gel_scale = parts[i].tmp*2.55f;

	//some heat convection for liquids
	if ((ptypes[t].properties&TYPE_LIQUID) && (t!=PT_GEL || gel_scale > (1+rand()%255)) && y-2 >= 0 && y-2 < YRES)
	{
		r = pmap[y-2][x];
		if (!(!r || parts[i].type != (r&0xFF)))
		{
			if (parts[i].temp>parts[r>>8].temp)
			{
				swappage = parts[i].temp;
				parts[i].temp = parts[r>>8].temp;
				parts[r>>8].temp = swappage;
			}
		}
	}

	//heat transfer code
	if ((t!=PT_HSWC || parts[i].life==10) && (ptypes[t].hconduct*gel_scale) && (realistic || (ptypes[t].hconduct*gel_scale) > (rand()%250)))
	{
		float c_Cm = 0.0f;
		if (aheat_enable && !(ptypes[t].properties&PROP_NOAMBHEAT))
		{
			if (realistic)
			{
				c_heat = parts[i].temp*96.645/ptypes[t].hconduct*gel_scale*fabs((float)ptypes[t].weight) + hv[y/CELL][x/CELL]*100*(pv[y/CELL][x/CELL]+273.15f)/256;
				c_Cm = 96.645/ptypes[t].hconduct*gel_scale*fabs((float)ptypes[t].weight) + 100*(pv[y/CELL][x/CELL]+273.15f)/256;
				pt = c_heat/c_Cm;
				pt = restrict_flt(pt, -MAX_TEMP+MIN_TEMP, MAX_TEMP-MIN_TEMP);
				parts[i].temp = pt;
				//Pressure increase from heat (temporary)
				pv[y/CELL][x/CELL] += (pt-hv[y/CELL][x/CELL])*0.004;
				hv[y/CELL][x/CELL] = pt;

				c_heat = 0.0f;
				c_Cm = 0.0f;
			}
			else
			{
				c_heat = (hv[y/CELL][x/CELL]-parts[i].temp)*0.04;
				c_heat = restrict_flt(c_heat, -MAX_TEMP+MIN_TEMP, MAX_TEMP-MIN_TEMP);
				parts[i].temp += c_heat;
				hv[y/CELL][x/CELL] -= c_heat;

				c_heat= 0.0f;
			}
		}
		for (j=0; j<8; j++)
		{
			surround_hconduct[j] = i;
			r = surround[j];
			if (!r)
				continue;
			rt = r&0xFF;

			if (rt && ptypes[rt].hconduct && (rt!=PT_HSWC || parts[r>>8].life == 10)
				   && (t !=PT_FILT || (rt!=PT_BRAY && rt!=PT_BIZR && rt!=PT_BIZRG))
				   && (rt!=PT_FILT || (t !=PT_BRAY && t !=PT_BIZR && t!=PT_BIZRG && t!=PT_PHOT))
				   && (t !=PT_ELEC || rt!=PT_DEUT)
				   && (t !=PT_DEUT || rt!=PT_ELEC))
			{
				surround_hconduct[j] = r>>8;
				if (realistic)
				{
					if (rt==PT_GEL)
						gel_scale = parts[r>>8].tmp*2.55f;
					else gel_scale = 1.0f;

					c_heat += parts[r>>8].temp*96.645/ptypes[rt].hconduct*fabs((float)ptypes[rt].weight);
					c_Cm += 96.645/ptypes[rt].hconduct*fabs((float)ptypes[rt].weight);
				}
				else
				{
					c_heat += parts[r>>8].temp;
				}
				h_count++;
			}
		}
		if (realistic)
		{
			if (rt==PT_GEL)
				gel_scale = parts[r>>8].tmp*2.55f;
			else gel_scale = 1.0f;

			if (t == PT_PHOT)
				pt = (c_heat+parts[i].temp*96.645)/(c_Cm+96.645);
			else
				pt = (c_heat+parts[i].temp*96.645/ptypes[t].hconduct*gel_scale*fabs((float)ptypes[t].weight))/(c_Cm+96.645/ptypes[t].hconduct*gel_scale*fabs((float)ptypes[t].weight));
			c_heat += parts[i].temp*96.645/ptypes[t].hconduct*gel_scale*fabs((float)ptypes[t].weight);
			c_Cm += 96.645/ptypes[t].hconduct*gel_scale*fabs((float)ptypes[t].weight);
			parts[i].temp = restrict_flt(pt, MIN_TEMP, MAX_TEMP);
		}
		else
		{
			pt = (c_heat+parts[i].temp)/(h_count+1);
			pt = parts[i].temp = restrict_flt(pt, MIN_TEMP, MAX_TEMP);
			for (j=0; j<8; j++)
			{
				parts[surround_hconduct[j]].temp = pt;
			}
		}

		ctemph = ctempl = pt;
		// change boiling point with pressure
		if ((ptypes[t].state == ST_LIQUID && ptransitions[t].tht > -1 && ptransitions[t].tht < PT_NUM && ptypes[ptransitions[t].tht].state == ST_GAS) || t==PT_LNTG || t==PT_SLTW)
			ctemph -= 2.0f*pv[y/CELL][x/CELL];
		else if ((ptypes[t].state == ST_GAS && ptransitions[t].tlt > -1 && ptransitions[t].tlt < PT_NUM && ptypes[ptransitions[t].tlt].state == ST_LIQUID) || t==PT_WTRV)
			ctempl -= 2.0f*pv[y/CELL][x/CELL];
		s = 1;

		if (!(ptypes[t].properties&PROP_INDESTRUCTIBLE))
		{
			//A fix for ice with ctype = 0
			if ((t==PT_ICEI || t==PT_SNOW) && (parts[i].ctype==0 || parts[i].ctype>=PT_NUM || parts[i].ctype==PT_ICEI || parts[i].ctype==PT_SNOW || !globalSim->elements[parts[i].ctype].Enabled))
				parts[i].ctype = PT_WATR;
			if (ptransitions[t].tht > -1 && ctemph >= ptransitions[t].thv)
			{
				// particle type change due to high temperature
				float dbt = ctempl - pt;
				if (ptransitions[t].tht != PT_NUM)
				{
					if (realistic)
					{
						if (platent[t] <= (c_heat - (ptransitions[t].thv - dbt)*c_Cm))
						{
							pt = (c_heat - platent[t])/c_Cm;
							t = ptransitions[t].tht;
						}
						else
						{
							parts[i].temp = restrict_flt(ptransitions[t].thv - dbt, MIN_TEMP, MAX_TEMP);
							s = 0;
						}
					}
					else
						t = ptransitions[t].tht;
				}
				else if (t==PT_ICEI || t==PT_SNOW)
				{
					if (realistic)
					{
						if (parts[i].ctype > 0 && parts[i].ctype < PT_NUM&&parts[i].ctype != t)
						{
							if (ptransitions[parts[i].ctype].tlt==t&&pt<ptransitions[parts[i].ctype].tlv)
								s = 0;
							else
							{
								//One ice table value for all it's kinds
								if (platent[t] <= (c_heat - (ptransitions[parts[i].ctype].tlv - dbt)*c_Cm))
								{
									pt = (c_heat - platent[t])/c_Cm;
									t = parts[i].ctype;
									parts[i].ctype = PT_NONE;
									parts[i].life = 0;
								}
								else
								{
									parts[i].temp = restrict_flt(ptransitions[parts[i].ctype].tlv - dbt, MIN_TEMP, MAX_TEMP);
									s = 0;
								}
							}
						}
						else
							s = 0;
					}
					else
					{
						if (parts[i].ctype>0&&parts[i].ctype<PT_NUM&&parts[i].ctype!=t) 
						{
							if (ptransitions[parts[i].ctype].tlt==t&&pt<ptransitions[parts[i].ctype].tlv)
								s = 0;
							else
							{
								t = parts[i].ctype;
								parts[i].ctype = PT_NONE;
								parts[i].life = 0;
							}
						}
						else
							s = 0;
					}
				}
				else if (t==PT_SLTW)
				{
					if (realistic)
					{
						if (platent[t] <= (c_heat - (ptransitions[t].thv - dbt)*c_Cm))
						{
							pt = (c_heat - platent[t])/c_Cm;

							if (1>rand()%6)
								t = PT_SALT;
							else
								t = PT_WTRV;
						}
						else
						{
							parts[i].temp = restrict_flt(ptransitions[t].thv - dbt, MIN_TEMP, MAX_TEMP);
							s = 0;
						}
					}
					else
					{
						if (1>rand()%6)
							t = PT_SALT;
						else
							t = PT_WTRV;
					}
				}
				else if (t == PT_BRMT)
				{
					if (parts[i].ctype == PT_TUNG)
					{
						if (ctemph < 3695.0)
							s = 0;
						else
						{
							t = PT_LAVA;
							parts[i].type = PT_TUNG;
						}
					}
					else if (ctemph >= ptransitions[t].tht)
						t = PT_LAVA;
					else
						s = 0;
				}
				else
					s = 0;
			}
			else if (ptransitions[t].tlt>-1 && ctempl<ptransitions[t].tlv)
			{
				// particle type change due to low temperature
				float dbt = ctempl - pt;
				if (ptransitions[t].tlt!=PT_NUM)
				{
					if (realistic)
					{
						if (platent[ptransitions[t].tlt] >= (c_heat - (ptransitions[t].tlv - dbt)*c_Cm))
						{
							pt = (c_heat + platent[ptransitions[t].tlt])/c_Cm;
							t = ptransitions[t].tlt;
						}
						else
						{
							parts[i].temp = restrict_flt(ptransitions[t].tlv - dbt, MIN_TEMP, MAX_TEMP);
							s = 0;
						}
					}
					else
					{
						t = ptransitions[t].tlt;
					}
				}
				else if (t==PT_WTRV)
				{
					if (pt<273.0f)
						t = PT_RIME;
					else
						t = PT_DSTW;
				}
				else if (t==PT_LAVA)
				{
					if (parts[i].ctype>0 && parts[i].ctype<PT_NUM && parts[i].ctype!=PT_LAVA && parts[i].ctype!=PT_LAVA && globalSim->elements[parts[i].ctype].Enabled)
					{
						if (parts[i].ctype==PT_THRM&&pt>=ptransitions[PT_BMTL].thv)
							s = 0;
						else if ((parts[i].ctype==PT_VIBR || parts[i].ctype==PT_BVBR) && pt>=273.15f)
							s = 0;
						else if (parts[i].ctype==PT_TUNG)
						{
							if (pt>=3695.0)
								s = 0;
						}
						else if (ptransitions[parts[i].ctype].tht==PT_LAVA)
						{
							if (pt>=ptransitions[parts[i].ctype].thv)
								s = 0;
						}
						else if (pt>=973.0f)
							s = 0; // freezing point for lava with any other (not listed in ptransitions as turning into lava) ctype
						if (s)
						{
							t = parts[i].ctype;
							parts[i].ctype = PT_NONE;
							if (t == PT_THRM)
							{
								parts[i].tmp = 0;
								t = PT_BMTL;
							}
							if (t == PT_PLUT)
							{
								parts[i].tmp = 0;
								t = PT_LAVA;
							}
						}
					}
					else if (pt<973.0f)
						t = PT_STNE;
					else
						s = 0;
				}
				else
					s = 0;
			}
			else
				s = 0;

			if (realistic)
			{
				pt = restrict_flt(pt, MIN_TEMP, MAX_TEMP);
				for (j=0; j<8; j++)
				{
					parts[surround_hconduct[j]].temp = pt;
				}
			}

			if (s)
			{ // particle type change occurred
				if (t==PT_ICEI || t==PT_LAVA || t==PT_SNOW)
					parts[i].ctype = parts[i].type;
				if (!(t==PT_ICEI && parts[i].ctype==PT_FRZW))
					parts[i].life = 0;
				if (ptypes[t].state==ST_GAS && ptypes[parts[i].type].state!=ST_GAS)
					pv[y/CELL][x/CELL] += 0.50f;
				if (t==PT_NONE)
				{
					kill_part(i);
					return t;
				}
				else
					part_change_type(i,x,y,t);
				if (t==PT_FIRE || t==PT_PLSM || t==PT_HFLM)
					parts[i].life = rand()%50+120;
				if (t==PT_LAVA)
				{
					if (parts[i].ctype==PT_BRMT)		parts[i].ctype = PT_BMTL;
					else if (parts[i].ctype==PT_SAND)	parts[i].ctype = PT_GLAS;
					else if (parts[i].ctype==PT_BGLA)	parts[i].ctype = PT_GLAS;
					else if (parts[i].ctype==PT_PQRT)	parts[i].ctype = PT_QRTZ;
					parts[i].life = rand()%120+240;
				}
			}
		}

		pt = parts[i].temp = restrict_flt(parts[i].temp, MIN_TEMP, MAX_TEMP);
		if (t==PT_LAVA)
		{
			parts[i].life = (int)restrict_flt((parts[i].temp-700)/7, 0.0f, 400.0f);
			if (parts[i].ctype==PT_THRM&&parts[i].tmp>0)
			{
				parts[i].tmp--;
				parts[i].temp = 3500;
			}
			if (parts[i].ctype==PT_PLUT&&parts[i].tmp>0)
			{
				parts[i].tmp--;
				parts[i].temp = MAX_TEMP;
			}
		}
	}
	else
	{
		if (!(bmap_blockairh[y/CELL][x/CELL]&0x8))
			bmap_blockairh[y/CELL][x/CELL]++;

		parts[i].temp = restrict_flt(parts[i].temp, MIN_TEMP, MAX_TEMP);
	}
	return t;
}

void particle_transitions(int i, int *t)
{
	int x = (int)(parts[i].x+0.5f), y = (int)(parts[i].y+0.5f);
	float gravtot = fabs(gravy[(y/CELL)*(XRES/CELL)+(x/CELL)])+fabs(gravx[(y/CELL)*(XRES/CELL)+(x/CELL)]);

	// particle type change due to high pressure
	if (ptransitions[*t].pht>-1 && pv[y/CELL][x/CELL]>ptransitions[*t].phv)
	{
		if (ptransitions[*t].pht != PT_NUM)
			*t = ptransitions[*t].pht;
		else if (*t == PT_BMTL)
		{
			if (pv[y/CELL][x/CELL] > 2.5f)
				*t = PT_BRMT;
			else if (pv[y/CELL][x/CELL]>1.0f && parts[i].tmp==1)
				*t = PT_BRMT;
			else
				return;
		}
	}
	// particle type change due to low pressure
	else if (ptransitions[*t].plt>-1 && pv[y/CELL][x/CELL]<ptransitions[*t].plv)
	{
		if (ptransitions[*t].plt != PT_NUM)
			*t = ptransitions[*t].plt;
		else
			return;
	}
	// particle type change due to high gravity
	else if (ptransitions[*t].pht>-1 && gravtot>(ptransitions[*t].phv/4.0f))
	{
		if (ptransitions[*t].pht != PT_NUM)
			*t = ptransitions[*t].pht;
		else if (*t == PT_BMTL)
		{
			if (gravtot > 0.625f)
				*t = PT_BRMT;
			else if (gravtot>0.25f && parts[i].tmp==1)
				*t = PT_BRMT;
			else
				return;
		}
		else
			return;
	}
	else
		return;

	// particle type change occurred
	parts[i].life = 0;
	if (!*t)
		kill_part(i);
	else
		part_change_type(i,x,y,*t);
	if (*t == PT_FIRE)
		parts[i].life = rand()%50+120;
}

//todo, I never really like this so maybe remove it ...
int PCLN_update(UPDATE_FUNC_ARGS);
int CLNE_update(UPDATE_FUNC_ARGS);
int PBCN_update(UPDATE_FUNC_ARGS);
int BCLN_update(UPDATE_FUNC_ARGS);
int MOVS_update(UPDATE_FUNC_ARGS);

//the main function for updating particles
void update_particles_i()
{
	stacking_check();

	if (globalSim->elementCount[PT_GRAV] > 0) //crappy grav color handling, i will change this someday
		GRAV_update();

	if (globalSim->elementCount[PT_LOVE] > 0 || globalSim->elementCount[PT_LOLZ] > 0) //LOVE and LOLZ element handling
		LOVELOLZ_update();

	if(globalSim->elementCount[PT_WIRE] > 0)
		WIRE_update();

	if (ppip_changed)
		PPIP_update();

	if (globalSim->elementCount[PT_LIFE] > 0 && ++CGOL >= GSPEED) //GSPEED is frames per generation
		LIFE_update();

	for (int t = 1; t < PT_NUM; t++)
	{
		if (globalSim->elementData[t])
		{
			globalSim->elementData[t]->Simulation_BeforeUpdate(globalSim);
		}
	}

	if (globalSim->lightning_recreate)
		globalSim->lightning_recreate--;

	//the main particle loop function, goes over all particles.
	for (int i = 0; i <= globalSim->parts_lastActiveIndex; i++)
		if (parts[i].type)
		{
			if (!globalSim->UpdateParticle(i))
			{
				if (globalSim->elements[parts[i].type].Properties&PROP_MOVS)
				{
					int bn = parts[i].tmp2;
					if (bn >= 0 && bn < 256 && msindex[bn])
					{
						msvx[bn] = msvx[bn] + parts[i].vx;
						msvy[bn] = msvy[bn] + parts[i].vy;
					}
				}
			}
		}
}

void rotate(float *x, float *y, float angle);
void update_moving_solids()
{
	if (numballs == 0)
		return;
	for (int bn = 0; bn < numballs; bn++)
	{
		msvx[bn] = msvx[bn]/msnum[bn];
		msvy[bn] = msvy[bn]/msnum[bn];
		msvy[bn] = msvy[bn] + .2;
		msrotation[bn] = newmsrotation[bn];
		if (!ms_rotation)
		{
			msrotation[bn] = 0;
		}
		else if (msrotation[bn] > 2*M_PI)
		{
			msrotation[bn] -= 2*M_PI;
		}
		else if (msrotation[bn] < -2*M_PI)
		{
			msrotation[bn] += 2*M_PI;
		}
	}
	for (int i = 0; i <= globalSim->parts_lastActiveIndex; i++)
	{
		if (ptypes[parts[i].type].properties&PROP_MOVS)
		{
			int bn = parts[i].tmp2;
			if (bn < 0 || bn > 255)
				continue;
			if (msindex[bn])
			{
				float tmp = parts[i].pavg[0];
				float tmp2 = parts[i].pavg[1];
				if (ms_rotation)
					rotate(&tmp, &tmp2, msrotation[bn]);
				float nx = parts[msindex[bn]-1].x + tmp;
				float ny = parts[msindex[bn]-1].y + tmp2;
				move(i,(int)(parts[i].x+.5f),(int)(parts[i].y+.5f),nx,ny);

				if (ms_rotation)
				{
					rotate(&tmp, &tmp2, .02f);
					if (parts[msindex[bn]-1].x + tmp != nx || parts[msindex[bn]-1].y + tmp2 != ny)
					{
						int j = globalSim->part_create(-1, (int)(parts[msindex[bn]-1].x + tmp), (int)(parts[msindex[bn]-1].y + tmp2), parts[i].type);
						if (j >= 0)
							parts[j].flags |= FLAG_DISAPPEAR;
					}
				}

				parts[i].vx = msvx[bn];
				parts[i].vy = msvy[bn];
			}
			if (parts[i].x<CELL || parts[i].x>=XRES-CELL || parts[i].y<CELL || parts[i].y>=YRES-CELL)//kill_part if particle is out of bounds
				kill_part(i);
		}
	}
	for (int bn = 0; bn < numballs; bn++)
	{
		msvx[bn] = 0;
		msvy[bn] = 0;
	}
}

void clear_area(int area_x, int area_y, int area_w, int area_h)
{
	int cx = 0;
	int cy = 0;
	int i;
	for (cy=0; cy<area_h; cy++)
	{
		for (cx=0; cx<area_w; cx++)
		{
			bmap[(cy+area_y)/CELL][(cx+area_x)/CELL] = 0;
			delete_part(cx+area_x, cy+area_y, 0);
		}
	}
	for (i=0; i<MAXSIGNS; i++)
	{
		if (signs[i].x>=area_x && signs[i].x<area_x+area_w && signs[i].y>=area_y && signs[i].y<area_y+area_h)
		{
			signs[i].text[0] = 0;
		}
	}
}

int flood_water(int x, int y, int i, int originaly, int check)
{
	int x1 = 0,x2 = 0;
	// go left as far as possible
	x1 = x2 = x;
	if (!pmap[y][x])
		return 1;

	while (x1>=CELL)
	{
		if ((ptypes[(pmap[y][x1-1]&0xFF)].falldown)!=2)
		{
			break;
		}
		x1--;
	}
	while (x2<XRES-CELL)
	{
		if ((ptypes[(pmap[y][x2+1]&0xFF)].falldown)!=2)
		{
			break;
		}
		x2++;
	}

	// fill span
	for (x=x1; x<=x2; x++)
	{
		if (check)
			parts[pmap[y][x]>>8].flags &= ~FLAG_WATEREQUAL;//flag it as checked (different from the original particle's checked flag)
		else
			parts[pmap[y][x]>>8].flags |= FLAG_WATEREQUAL;
		//check above, maybe around other sides too?
		if ( ((y-1) > originaly) && !pmap[y-1][x] && eval_move(parts[i].type, x, y-1, NULL))
		{
			int oldx = (int)(parts[i].x + 0.5f);
			int oldy = (int)(parts[i].y + 0.5f);
			pmap[y-1][x] = pmap[oldy][oldx];
			pmap[oldy][oldx] = 0;
			parts[i].x = (float)x;
			parts[i].y = y-1.0f;
			return 0;
		}
	}
	// fill children
	
	if (y>=CELL+1)
		for (x=x1; x<=x2; x++)
			if ((ptypes[(pmap[y-1][x]&0xFF)].falldown)==2 && (parts[pmap[y-1][x]>>8].flags & FLAG_WATEREQUAL) == check)
				if (!flood_water(x, y-1, i, originaly, check))
					return 0;
	if (y<YRES-CELL-1)
		for (x=x1; x<=x2; x++)
			if ((ptypes[(pmap[y+1][x]&0xFF)].falldown)==2 && (parts[pmap[y+1][x]>>8].flags & FLAG_WATEREQUAL) == check)
				if (!flood_water(x, y+1, i, originaly, check))
					return 0;
	return 1;
}

void create_moving_solid(int x, int y, int type, Brush* brush)
{
	int index, i, j, rx = brush->GetRadius().X, ry = brush->GetRadius().Y;
	//max of 255 moving solids for now
	if (numballs >= 255)
		return;

	//create the center "control" particle, set it's default properties and init moving solid variables
	index = create_part(-2, x, y, type);
	if (index < 0)
		return;
	parts[index].tmp2 = numballs;
	parts[index].pavg[0] = 0;
	parts[index].pavg[1] = 0;
	msindex[numballs] = index+1;
	msnum[numballs] = 1;
	msvx[numballs] = 0;
	msvy[numballs] = 0;
	msrotation[numballs] = 0;

	numballs = numballs + 1;

	for (j=-ry; j<=ry; j++)
		for (i=-rx; i<=rx; i++)
			if (brush->IsInside(i ,j) && (i || j))
			{
				index = create_part(-2, x+i, y+j, type);
				if (index < 0)
					continue;
				//set which ball # it belongs to, and which position in the ball it is
				parts[index].tmp2 = numballs-1;
				parts[index].pavg[0] = x+i - parts[msindex[numballs-1]-1].x;
				parts[index].pavg[1] = y+j - parts[msindex[numballs-1]-1].y;
				msnum[numballs-1]++;
			}
}

int get_brush_flags()
{
	int flags = 0;
	if (REPLACE_MODE)
		flags |= BRUSH_REPLACEMODE;
	if (SPECIFIC_DELETE)
		flags |= BRUSH_SPECIFIC_DELETE;
	return flags;
}

TPT_GNU_INLINE void orbitalparts_get(int block1, int block2, int resblock1[], int resblock2[])
{
	resblock1[0] = (block1&0x000000FF);
	resblock1[1] = (block1&0x0000FF00)>>8;
	resblock1[2] = (block1&0x00FF0000)>>16;
	resblock1[3] = (block1&0xFF000000)>>24;

	resblock2[0] = (block2&0x000000FF);
	resblock2[1] = (block2&0x0000FF00)>>8;
	resblock2[2] = (block2&0x00FF0000)>>16;
	resblock2[3] = (block2&0xFF000000)>>24;
}

TPT_GNU_INLINE void orbitalparts_set(int *block1, int *block2, int resblock1[], int resblock2[])
{
	int block1tmp = 0;
	int block2tmp = 0;

	block1tmp = (resblock1[0]&0xFF);
	block1tmp |= (resblock1[1]&0xFF)<<8;
	block1tmp |= (resblock1[2]&0xFF)<<16;
	block1tmp |= (resblock1[3]&0xFF)<<24;

	block2tmp = (resblock2[0]&0xFF);
	block2tmp |= (resblock2[1]&0xFF)<<8;
	block2tmp |= (resblock2[2]&0xFF)<<16;
	block2tmp |= (resblock2[3]&0xFF)<<24;

	*block1 = block1tmp;
	*block2 = block2tmp;
}

void draw_bframe()
{
	int i;
	for(i=0; i<(XRES/CELL); i++)
	{
		bmap[0][i]=WL_WALL;
		bmap[YRES/CELL-1][i]=WL_WALL;
	}
	for(i=1; i<((YRES/CELL)-1); i++)
	{
		bmap[i][0]=WL_WALL;
		bmap[i][XRES/CELL-1]=WL_WALL;
	}
}

void erase_bframe()
{
	int i;
	for(i=0; i<(XRES/CELL); i++)
	{
		bmap[0][i]=0;
		bmap[YRES/CELL-1][i]=0;
	}
	for(i=1; i<((YRES/CELL)-1); i++)
	{
		bmap[i][0]=0;
		bmap[i][XRES/CELL-1]=0;
	}
}
