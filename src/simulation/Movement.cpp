#include <cmath>
#include "Simulation.h"
#include "WallNumbers.h"
#include "simulation/elements/PRTI.h"
#include "simulation/elements/FIGH.h"
#include "simulation/elements/STKM.h"

bool Simulation::OutOfBounds(int x, int y)
{
	if (edgeMode != 3)
		return (x < CELL || x >= XRES-CELL || y < CELL || y >= YRES-CELL);
	else
		return (x < 0 || x >= XRES || y < 0 || y >= YRES);
}

bool Simulation::IsWallBlocking(int x, int y, int type)
{
	if (bmap[y/CELL][x/CELL])
	{
		int wall = bmap[y/CELL][x/CELL];
		if (wall == WL_ALLOWGAS && !(ptypes[type].properties&TYPE_GAS))
			return true;
		else if (wall == WL_ALLOWENERGY && !(ptypes[type].properties&TYPE_ENERGY))
			return true;
		else if (wall == WL_ALLOWLIQUID && ptypes[type].falldown!=2)
			return true;
		else if (wall == WL_ALLOWSOLID && ptypes[type].falldown!=1)
			return true;
		else if (wall == WL_ALLOWAIR || wall == WL_WALL || wall == WL_WALLELEC)
			return true;
		else if (wall == WL_EWALL && !emap[y/CELL][x/CELL])
			return true;
	}
	return false;
}

// create photons when PHOT moves through GLOW
void Simulation::CreateGainPhoton(int pp)
{
	int lr = rand() % 2;

	float xx, yy;
	if (lr)
	{
		xx = parts[pp].x - 0.3f*parts[pp].vy;
		yy = parts[pp].y + 0.3f*parts[pp].vx;
	}
	else
	{
		xx = parts[pp].x + 0.3f*parts[pp].vy;
		yy = parts[pp].y - 0.3f*parts[pp].vx;
	}

	int nx = (int)(xx + 0.5f);
	int ny = (int)(yy + 0.5f);

	if (nx<0 || ny<0 || nx>=XRES || ny>=YRES)
		return;

	if ((pmap[ny][nx] & 0xFF) != PT_GLOW)
		return;

	int i = part_create(-1, nx, ny, PT_PHOT);
	if (i < 0)
		return;

	parts[i].life = 680;
	parts[i].x = xx;
	parts[i].y = yy;
	parts[i].vx = parts[pp].vx;
	parts[i].vy = parts[pp].vy;
	parts[i].temp = parts[pmap[ny][nx] >> 8].temp;

	int temp_bin = (int)((parts[i].temp-273.0f)*0.25f);
	temp_bin = std::max(0, std::min(25, temp_bin));
	parts[i].ctype = 0x1F << temp_bin;
}

// create photons when NEUT moves through GLAS
void Simulation::CreateCherenkovPhoton(int pp)
{
	int nx = (int)(parts[pp].x + 0.5f);
	int ny = (int)(parts[pp].y + 0.5f);
	if ((pmap[ny][nx] & 0xFF) != PT_GLAS)
		return;

	if (hypotf(parts[pp].vx, parts[pp].vy) < 1.44f)
		return;

	int i = part_create(-1, nx, ny, PT_PHOT);
	if (i < 0)
		return;

	parts[i].ctype = 0x00000F80;
	parts[i].life = 680;
	parts[i].x = parts[pp].x;
	parts[i].y = parts[pp].y;
	parts[i].temp = parts[pmap[ny][nx] >> 8].temp;
	parts[i].pavg[0] = parts[i].pavg[1] = 0.0f;

	int lr = rand() % 2;
	if (lr)
	{
		parts[i].vx = parts[pp].vx - 2.5f*parts[pp].vy;
		parts[i].vy = parts[pp].vy + 2.5f*parts[pp].vx;
	}
	else
	{
		parts[i].vx = parts[pp].vx + 2.5f*parts[pp].vy;
		parts[i].vy = parts[pp].vy - 2.5f*parts[pp].vx;
	}

	/* photons have speed of light. no discussion. */
	float r = 1.269f / hypotf(parts[i].vx, parts[i].vy);
	parts[i].vx *= r;
	parts[i].vy *= r;
}

// ************************* //
// Photon movement functions //
// ************************* //

// create sparks from PHOT when hitting PSCN and NSCN
void Simulation::PhotoelectricEffect(int nx, int ny)
{
	unsigned r = pmap[ny][nx];

	if ((r&0xFF) == PT_PSCN)
	{
		if ((pmap[ny][nx-1] & 0xFF) == PT_NSCN || (pmap[ny][nx+1] & 0xFF) == PT_NSCN ||
		    (pmap[ny-1][nx] & 0xFF) == PT_NSCN || (pmap[ny+1][nx] & 0xFF) == PT_NSCN)
			spark_conductive_attempt(r>>8, nx, ny);
	}
}

unsigned Simulation::DirectionToMap(float dx, float dy, int t)
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

bool Simulation::IsBlocking(int t, int x, int y)
{
	if (t & REFRACT) {
		if (x<0 || y<0 || x>=XRES || y>=YRES)
			return false;
		if ((pmap[y][x] & 0xFF) == PT_GLAS)
			return true;
		return false;
	}

	return !EvalMove(t, x, y);
}

bool Simulation::IsBoundary(int pt, int x, int y)
{
	if (!IsBlocking(pt, x, y))
		return false;
	if (IsBlocking(pt, x, y-1) && IsBlocking(pt, x, y+1) && IsBlocking(pt, x-1, y) && IsBlocking(pt, x+1, y))
		return false;
	return true;
}

bool Simulation::FindNextBoundary(int pt, int *x, int *y, int dm, int *em)
{
	static int dx[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	static int dy[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
	static int de[8] = { 0x83, 0x07, 0x0E, 0x1C, 0x38, 0x70, 0xE0, 0xC1 };

	if (*x <= 0 || *x >= XRES-1 || *y <= 0 || *y >= YRES-1)
		return false;

	int i0;
	if (*em != -1)
	{
		i0 = *em;
		dm &= de[i0];
	}
	else
		i0 = 0;

	int i;
	for (int ii = 0; ii < 8; ii++)
	{
		i = (ii + i0) & 7;
		if ((dm & (1 << i)) && IsBoundary(pt, *x+dx[i], *y+dy[i]))
		{
			*x += dx[i];
			*y += dy[i];
			*em = i;
			return true;
		}
	}

	return false;
}

bool Simulation::GetNormal(int pt, int x, int y, float dx, float dy, float *nx, float *ny)
{
	if (!dx && !dy)
		return false;

	if (!IsBoundary(pt, x, y))
		return false;

	int ldm = DirectionToMap(-dy, dx, pt);
	int rdm = DirectionToMap(dy, -dx, pt);
	int lx = x, rx = x;
	int ly = y, ry = y;
	int lv = 1, rv = 1;
	int lm = -1, rm = -1;

	int j = 0;
	for (int i = 0; i < SURF_RANGE; i++)
	{
		if (lv)
			lv = FindNextBoundary(pt, &lx, &ly, ldm, &lm);
		if (rv)
			rv = FindNextBoundary(pt, &rx, &ry, rdm, &rm);
		j += lv + rv;
		if (!lv && !rv)
			break;
	}

	if (j < NORMAL_MIN_EST)
		return false;

	if ((lx == rx) && (ly == ry))
		return false;

	float ex = (float)rx - lx;
	float ey = (float)ry - ly;
	float r = 1.0f/hypot(ex, ey);
	*nx =  ey * r;
	*ny = -ex * r;

	return true;
}

bool Simulation::GetNormalInterp(int pt, float x0, float y0, float dx, float dy, float *nx, float *ny)
{
	dx /= NORMAL_FRAC;
	dy /= NORMAL_FRAC;

	int i, x, y;
	for (i = 0; i < NORMAL_INTERP; i++)
	{
		x = (int)(x0 + 0.5f);
		y = (int)(y0 + 0.5f);
		if (IsBoundary(pt, x, y))
			break;
		x0 += dx;
		y0 += dy;
	}
	if (i >= NORMAL_INTERP)
		return false;

	if (pt == PT_PHOT)
		PhotoelectricEffect(x, y);

	return GetNormal(pt, x, y, dx, dy, nx, ny);
}

// ************************* //
// Actual movement functions //
// ************************* //

void Simulation::InitCanMove()
{
	// can_move[moving type][type at destination]
	//  0 = No move/Bounce
	//  1 = Swap
	//  2 = Both particles occupy the same space.
	//  3 = Varies, go run some extra checks

	// particles that don't exist shouldn't move...
	for (int destinationType = 0; destinationType < PT_NUM; destinationType++)
		can_move[0][destinationType] = 0;

	//initialize everything else to swapping by default
	for (int movingType = 1; movingType < PT_NUM; movingType++)
		for (int destinationType = 0; destinationType < PT_NUM; destinationType++)
			can_move[movingType][destinationType] = 1;

	//photons go through everything by default
	for (int destinationType = 1; destinationType < PT_NUM; destinationType++)
		can_move[PT_PHOT][destinationType] = 2;

	for (int movingType = 1; movingType < PT_NUM; movingType++)
	{
		for (int destinationType = 1; destinationType < PT_NUM; destinationType++)
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
	for (int destinationType = 0; destinationType < PT_NUM; destinationType++)
	{
		//set what stickmen can move through
		unsigned char stkm_move = 0;
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
	for (int movingType = 1; movingType < PT_NUM; movingType++)
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
#ifndef NOMOD
		can_move[movingType][PT_PINV] = 3;
#endif
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
	}
	//a list of lots of things PHOT can move through
	for (int destinationType = 0; destinationType < PT_NUM; destinationType++)
	{
		if (destinationType == PT_GLAS || destinationType == PT_PHOT || destinationType == PT_FILT || destinationType == PT_H2
		 || destinationType == PT_WATR || destinationType == PT_DSTW || destinationType == PT_SLTW || destinationType == PT_GLOW
		 || destinationType == PT_ISOZ || destinationType == PT_ISZS || destinationType == PT_QRTZ || destinationType == PT_PQRT
		 || destinationType == PT_INVIS
#ifndef NOMOD
		 || destinationType == PT_PINV
#endif
		 || (ptypes[destinationType].properties&PROP_CLONE) || (ptypes[destinationType].properties&PROP_BREAKABLECLONE))
			can_move[PT_PHOT][destinationType] = 2;
		if (destinationType != PT_DMND && destinationType != PT_INSL && destinationType != PT_VOID && destinationType != PT_PVOD
			 && destinationType != PT_VIBR && destinationType != PT_BVBR && destinationType != PT_PRTO && destinationType != PT_PRTI
#ifndef NOMOD
			 && destinationType != PT_PPTO && destinationType != PT_PPTI
#endif
			 )
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
#ifndef NOMOD
	can_move[PT_ELEC][PT_PINV] = 2;
#endif
	can_move[PT_ELEC][PT_LCRY] = 2;
	can_move[PT_ELEC][PT_EXOT] = 2;
	can_move[PT_ELEC][PT_GLOW] = 2;
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
	can_move[PT_THDR][PT_THDR] = 2;
	can_move[PT_EMBR][PT_EMBR] = 2;
	can_move[PT_TRON][PT_SWCH] = 3;

#ifndef NOMOD
	can_move[PT_RAZR][PT_CNCT] = 1;
	can_move[PT_RAZR][PT_GEL] = 1;
	can_move[PT_MOVS][PT_MOVS] = 2;
#endif
}

/*
   RETURN-value explanation
1 = Swap
0 = No move/Bounce
2 = Both particles occupy the same space.
 */
unsigned char Simulation::EvalMove(int pt, int nx, int ny, unsigned *rr)
{
	unsigned r;
	int result;

	if (nx<0 || ny<0 || nx>=XRES || ny>=YRES)
		return 0;

	r = pmap[ny][nx];
	if (r)
		r = (r&~0xFF) | parts[r>>8].type;
#ifndef NOMOD
	if ((r&0xFF) == PT_PINV && parts[r>>8].tmp2)
		r = parts[r>>8].tmp2;
#endif
	if (rr)
		*rr = r;
	if (pt>=PT_NUM || (r&0xFF)>=PT_NUM)
		return 0;
	result = can_move[pt][r&0xFF];
	if (result == 3)
	{
		if ((pt==PT_PHOT || pt==PT_ELEC) && (r&0xFF)==PT_LCRY)
			result = (parts[r>>8].life > 5)? 2 : 0;
		if ((r&0xFF)==PT_INVIS)
		{
			if (pv[ny/CELL][nx/CELL]>4.0f || pv[ny/CELL][nx/CELL]<-4.0f) result = 2;
			else result = 0;
		}
#ifndef NOMOD
		else if ((r&0xFF)==PT_PINV)
		{
			if (parts[r>>8].life >= 10) result = 2;
			else result = 0;
		}
#endif
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
			parts[r>>8].x += vx;
			parts[r>>8].y += vy;
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

int Simulation::TryMove(int i, int x, int y, int nx, int ny)
{
	unsigned r, e;

	if (x==nx && y==ny)
		return 1;
	if (nx<0 || ny<0 || nx>=XRES || ny>=YRES)
		return 1;

	e = EvalMove(parts[i].type, nx, ny, &r);

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
#ifdef NOMOD
		if ((r&0xFF)==PT_PRTI && (ptypes[parts[i].type].properties & TYPE_ENERGY))
#else
		if (((r&0xFF)==PT_PRTI || (r&0xFF)==PT_PPTI) && (ptypes[parts[i].type].properties & TYPE_ENERGY))
#endif
		{
			PortalChannel *channel = ((PRTI_ElementDataContainer*)elementData[PT_PRTI])->GetParticleChannel(this, r>>8);
			int slot = PRTI_ElementDataContainer::GetSlot(x-nx,y-ny);
			if (channel->StoreParticle(this, i, slot))
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
					CreateGainPhoton(i);
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
#ifndef NOMOD
				else if ((r&0xFF) == PT_PINV)
				{
					if (!parts[r>>8].life)
					{
						part_change_type(i, x, y, PT_ELEC);
						parts[i].ctype = 0;
					}
				}
#endif
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

				part_create(r>>8, x, y, PT_ELEC);
				return -1;
			}
		}
		else if (parts[i].type == PT_NEUT)
		{
			if ((r&0xFF) == PT_GLAS)
			{
				if (rand() < RAND_MAX/10)
					CreateCherenkovPhoton(i);
			}
		}
		else if (parts[i].type == PT_ELEC)
		{
			if ((r&0xFF) == PT_GLOW)
			{
				part_change_type(i, x, y, PT_PHOT);
				parts[i].ctype = 0x3FFFFFFF;
			}
		}
		else if (parts[i].type == PT_PROT)
		{
			if ((r&0xFF) == PT_INVIS)
				part_change_type(i, x, y, PT_NEUT);
		}
		else if (parts[i].type == PT_BIZR || parts[i].type == PT_BIZRG)
		{
			if ((r&0xFF) == PT_FILT)
				parts[i].ctype = interactWavelengths(&parts[r>>8], parts[i].ctype);
		}
		return 1;
	}
	//else e=1 , we are trying to swap the particles, return 0 no swap/move, 1 is still overlap/move, because the swap takes place later

	if ((r&0xFF)==PT_VOID || (r&0xFF)==PT_PVOD) //this is where void eats particles
	{
		//void ctype already checked in eval_move
		part_kill(i);
		return 0;
	}
	else if ((r&0xFF)==PT_BHOL || (r&0xFF)==PT_NBHL) //this is where blackhole eats particles
	{
		part_kill(i);
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
			part_kill(i);
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
			part_kill(i);
			return 0;
		}
	}
	else if ((r&0xFF)==PT_VIBR || (r&0xFF)==PT_BVBR)
	{
		if (ptypes[parts[i].type].properties & TYPE_ENERGY)
		{
			parts[r>>8].tmp += 20;
			part_kill(i);
			return 0;
		}
	}

	if (parts[i].type == PT_NEUT)
	{
		if (ptypes[r&0xFF].properties & PROP_NEUTABSORB)
		{
			part_kill(i);
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
		if (parts[i].type==PT_NEUT)
		{
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

// try to move particle, and if successful call move() to update variables
int Simulation::DoMove(int i, int x, int y, float nxf, float nyf)
{
	// volatile to hopefully force truncation of floats in x87 registers by storing and reloading from memory, so that rounding issues don't cause particles to appear in the wrong pmap list. If using -mfpmath=sse or an ARM CPU, this may be unnecessary.
	volatile float tmpx = nxf, tmpy = nyf;
	int nx = (int)(tmpx+0.5f), ny = (int)(tmpy+0.5f), result;

	if (parts[i].type == PT_NONE)
		return 0;
	result = TryMove(i, x, y, nx, ny);
	if (result && Move(i,x,y,nxf,nyf))
		return -1;
	return result;
}

//update pmap and parts[i].x,y
int Simulation::Move(int i, int x, int y, float nxf, float nyf)
{
	// volatile to hopefully force truncation of floats in x87 registers by storing and reloading from memory, so that rounding issues don't cause particles to appear in the wrong pmap list. If using -mfpmath=sse or an ARM CPU, this may be unnecessary.
	volatile float tmpx = nxf, tmpy = nyf;
	int nx = (int)(tmpx+0.5f), ny = (int)(tmpy+0.5f), t = parts[i].type;

	if (edgeMode == 2)
	{
		float diffx = 0.0f, diffy = 0.0f;
		if (nx < CELL)
			diffx = XRES-CELL*2;
		if (nx >= XRES-CELL)
			diffx = -(XRES-CELL*2);
		if (ny < CELL)
			diffy = YRES-CELL*2;
		if (ny >= YRES-CELL)
			diffy = -(YRES-CELL*2);
		if (diffx || diffy)
		{
			nxf += diffx;
			nyf += diffy;
			nx = (int)(nxf+0.5f);
			ny = (int)(nyf+0.5f);

			//kill particle if particle is out of bounds (below eval_move would return before kill)
			if (OutOfBounds(nx, ny))
			{
				part_kill(i);
				return -1;
			}
			//make sure there isn't something blocking it on the other side
			if (!EvalMove(t, nx, ny) || (t == PT_PHOT && pmap[ny][nx]))
				return -1;

			//adjust stickmen legs
			Stickman *stickman = NULL;
			if (t == PT_STKM)
				stickman = ((STKM_ElementDataContainer*)elementData[PT_STKM])->GetStickman1();
			else if (t == PT_STKM2)
				stickman = ((STKM_ElementDataContainer*)elementData[PT_STKM])->GetStickman2();
			else if (t == PT_FIGH && parts[i].tmp >= 0 && parts[i].tmp < ((FIGH_ElementDataContainer*)elementData[PT_FIGH])->MaxFighters())
				stickman = ((FIGH_ElementDataContainer*)elementData[PT_FIGH])->Get((unsigned char)parts[i].tmp);

			if (stickman)
				for (int i = 0; i < 16; i += 2)
				{
					stickman->legs[i] += diffx;
					stickman->legs[i+1] += diffy;
				}
		}
	}

	parts[i].x = nxf;
	parts[i].y = nyf;
	if (ny!=y || nx!=x)
	{
		if ((int)(pmap[y][x]>>8)==i)
			pmap[y][x] = 0;
#ifndef NOMOD
		else if ((pmap[y][x]&0xFF)==PT_PINV && (parts[pmap[y][x]>>8].tmp2>>8)==i)
			parts[pmap[y][x]>>8].tmp2 = 0;
#endif
		else if ((int)(photons[y][x]>>8)==i)
			photons[y][x] = 0;

		//kill particle if particle is out of bounds
		if (OutOfBounds(nx, ny))
		{
			part_kill(i);
			return -1;
		}

		if (ptypes[t].properties & TYPE_ENERGY)
			photons[ny][nx] = t|(i<<8);
#ifndef NOMOD
		else if (t && (pmap[ny][nx]&0xFF) != PT_PINV && (t!=PT_MOVS || !(pmap[ny][nx]&0xFF) || (pmap[ny][nx]&0xFF) == PT_MOVS))
			pmap[ny][nx] = t|(i<<8);
		else if (t && (pmap[ny][nx]&0xFF) == PT_PINV)
			parts[pmap[ny][nx]>>8].tmp2 = t|(i<<8);
#else
		else
			pmap[ny][nx] = t|(i<<8);
#endif
	}
	return 0;
}
