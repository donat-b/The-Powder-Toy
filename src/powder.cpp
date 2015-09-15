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

#include "game/Brush.h"
#include "game/Sign.h"
#include "simulation/Simulation.h"
#include "simulation/Tool.h"
#include "simulation/WallNumbers.h"

part_type ptypes[PT_NUM];
part_transition ptransitions[PT_NUM];
unsigned int platent[PT_NUM];

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

void decrease_life(int i)
{
	int t;
	unsigned int elem_properties;
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

bool transfer_heat(int i, int t, int surround[8])
{
	int x = (int)(parts[i].x+0.5f), y = (int)(parts[i].y+0.5f);
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
				c_heat = parts[i].temp*96.645f/ptypes[t].hconduct*gel_scale*fabs((float)ptypes[t].weight) + hv[y/CELL][x/CELL]*100*(pv[y/CELL][x/CELL]+273.15f)/256;
				c_Cm = 96.645f/ptypes[t].hconduct*gel_scale*fabs((float)ptypes[t].weight) + 100*(pv[y/CELL][x/CELL]+273.15f)/256;
				pt = c_heat/c_Cm;
				pt = restrict_flt(pt, -MAX_TEMP+MIN_TEMP, MAX_TEMP-MIN_TEMP);
				parts[i].temp = pt;
				//Pressure increase from heat (temporary)
				pv[y/CELL][x/CELL] += (pt-hv[y/CELL][x/CELL])*0.004f;
				hv[y/CELL][x/CELL] = pt;

				c_heat = 0.0f;
				c_Cm = 0.0f;
			}
			else
			{
				c_heat = (hv[y/CELL][x/CELL]-parts[i].temp)*0.04f;
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

					c_heat += parts[r>>8].temp*96.645f/ptypes[rt].hconduct*fabs((float)ptypes[rt].weight);
					c_Cm += 96.645f/ptypes[rt].hconduct*fabs((float)ptypes[rt].weight);
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
				pt = (c_heat+parts[i].temp*96.645f)/(c_Cm+96.645f);
			else
				pt = (c_heat+parts[i].temp*96.645f/ptypes[t].hconduct*gel_scale*fabs((float)ptypes[t].weight))/(c_Cm+96.645f/ptypes[t].hconduct*gel_scale*fabs((float)ptypes[t].weight));
			c_heat += parts[i].temp*96.645f/ptypes[t].hconduct*gel_scale*fabs((float)ptypes[t].weight);
			c_Cm += 96.645f/ptypes[t].hconduct*gel_scale*fabs((float)ptypes[t].weight);
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
			if ((t==PT_ICEI || t==PT_SNOW) && (parts[i].ctype<=0 || parts[i].ctype>=PT_NUM || parts[i].ctype==PT_ICEI || parts[i].ctype==PT_SNOW || !globalSim->elements[parts[i].ctype].Enabled))
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
							if ((ptransitions[parts[i].ctype].tlt==PT_ICEI || ptransitions[parts[i].ctype].tlt==PT_SNOW))
							{
								if (pt < ptransitions[parts[i].ctype].tlv)
									s = 0;
							}
							else if (pt < 273.15f)
								s = 0;
							if (s)
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
							if ((ptransitions[parts[i].ctype].tlt==PT_ICEI || ptransitions[parts[i].ctype].tlt==PT_SNOW))
							{
								if (pt < ptransitions[parts[i].ctype].tlv)
									s = 0;
							}
							else if (pt < 273.15f)
								s = 0;
							if (s)
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
						if (ctemph < ptransitions[PT_TUNG].thv)
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
#ifndef NOMOD
				else if (t == PT_CRMC)
				{
					float pres = std::max((pv[y/CELL][x/CELL]+pv[(y-2)/CELL][x/CELL]+pv[(y+2)/CELL][x/CELL]+pv[y/CELL][(x-2)/CELL]+pv[y/CELL][(x+2)/CELL])*2.0f, 0.0f);
					if (ctemph < pres+ptransitions[PT_CRMC].thv)
						s = 0;
					else
						t = PT_LAVA;
				}
#endif
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
				else if (t == PT_WTRV)
				{
					if (pt<273.0f)
						t = PT_RIME;
					else
						t = PT_DSTW;
				}
				else if (t==PT_LAVA)
				{
					if (parts[i].ctype>0 && parts[i].ctype<PT_NUM && parts[i].ctype!=PT_LAVA && globalSim->elements[parts[i].ctype].Enabled)
					{
						if (parts[i].ctype == PT_THRM && pt>=ptransitions[PT_BMTL].thv)
							s = 0;
						else if ((parts[i].ctype == PT_VIBR || parts[i].ctype == PT_BVBR) && pt>=273.15f)
							s = 0;
						else if (parts[i].ctype == PT_TUNG)
						{

							// TUNG does its own melting in its update function, so HighTemperatureTransition is not LAVA so it won't be handled by the code for HighTemperatureTransition==PT_LAVA below
							// However, the threshold is stored in HighTemperature to allow it to be changed from Lua
							if (pt >= ptransitions[PT_TUNG].thv)
								s = 0;
						}
#ifndef NOMOD
						else if (parts[i].ctype == PT_CRMC)
						{
							float pres = std::max((pv[y/CELL][x/CELL]+pv[(y-2)/CELL][x/CELL]+pv[(y+2)/CELL][x/CELL]+pv[y/CELL][(x-2)/CELL]+pv[y/CELL][(x+2)/CELL])*2.0f, 0.0f);
							if (ctemph >= pres+ptransitions[PT_CRMC].thv)
								s = 0;
						}
#endif
						else if (ptransitions[parts[i].ctype].tht == PT_LAVA)
						{
							if (pt>=ptransitions[parts[i].ctype].thv)
								s = 0;
						}
						else if (pt >= 973.0f)
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
					else if (pt < 973.0f)
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
				if (t == PT_FIRE)
				{
					//hackish, if tmp isn't 0 the FIRE might turn into DSTW later
					//idealy transitions should use create_part(i) but some elements rely on properties staying constant
					//and I don't feel like checking each one right now
					parts[i].tmp = 0;
				}
				if (ptypes[t].state==ST_GAS && ptypes[parts[i].type].state!=ST_GAS)
					pv[y/CELL][x/CELL] += 0.50f;
				if (t==PT_NONE)
				{
					kill_part(i);
					return true;
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
		else
			s = 0;

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
		return s == 1;
	}
	else
	{
		if (!(bmap_blockairh[y/CELL][x/CELL]&0x8))
			bmap_blockairh[y/CELL][x/CELL]++;

		parts[i].temp = restrict_flt(parts[i].temp, MIN_TEMP, MAX_TEMP);
		return false;
	}
}

bool particle_transitions(int i, int t)
{
	int x = (int)(parts[i].x+0.5f), y = (int)(parts[i].y+0.5f);
	float gravtot = fabs(gravy[(y/CELL)*(XRES/CELL)+(x/CELL)])+fabs(gravx[(y/CELL)*(XRES/CELL)+(x/CELL)]);

	// particle type change due to high pressure
	if (ptransitions[t].pht>-1 && pv[y/CELL][x/CELL]>ptransitions[t].phv)
	{
		if (ptransitions[t].pht != PT_NUM)
			t = ptransitions[t].pht;
		else if (t == PT_BMTL)
		{
			if (pv[y/CELL][x/CELL] > 2.5f)
				t = PT_BRMT;
			else if (pv[y/CELL][x/CELL]>1.0f && parts[i].tmp==1)
				t = PT_BRMT;
			else
				return false;
		}
	}
	// particle type change due to low pressure
	else if (ptransitions[t].plt>-1 && pv[y/CELL][x/CELL]<ptransitions[t].plv)
	{
		if (ptransitions[t].plt != PT_NUM)
			t = ptransitions[t].plt;
		else
			return false;
	}
	// particle type change due to high gravity
	else if (ptransitions[t].pht>-1 && gravtot>(ptransitions[t].phv/4.0f))
	{
		if (ptransitions[t].pht != PT_NUM)
			t = ptransitions[t].pht;
		else if (t == PT_BMTL)
		{
			if (gravtot > 0.625f)
				t = PT_BRMT;
			else if (gravtot>0.25f && parts[i].tmp==1)
				t = PT_BRMT;
			else
				return false;
		}
		else
			return false;
	}
	else
		return false;

	// particle type change occurred
	parts[i].life = 0;
	if (!t)
		kill_part(i);
	else
		part_change_type(i,x,y,t);
	if (t == PT_FIRE)
		parts[i].life = rand()%50+120;
	return true;
}

void clear_area(int area_x, int area_y, int area_w, int area_h)
{
	float fx = area_x-.5f, fy = area_y-.5f;
	for (int i = 0; i <= globalSim->parts_lastActiveIndex; i++)
	{
		if (parts[i].type)
			if (parts[i].x >= fx && parts[i].x <= fx+area_w && parts[i].y >= fy && parts[i].y <= fy+area_h)
				kill_part(i);
	}
	for (int cy = 0; cy < area_h; cy++)
	{
		for (int cx = 0; cx < area_w; cx++)
		{
			bmap[(cy+area_y)/CELL][(cx+area_x)/CELL] = 0;
		}
	}
	DeleteSignsInArea(Point(area_x, area_y), Point(area_x+area_w, area_y+area_h));
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
		if ( ((y-1) > originaly) && !pmap[y-1][x] && globalSim->EvalMove(parts[i].type, x, y-1))
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
