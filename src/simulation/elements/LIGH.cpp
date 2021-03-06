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
#include "simulation/elements/STKM.h"

#define LIGHTING_POWER 0.65

int LIGH_nearest_part(int ci, int max_d)
{
	int distance = (int)((max_d!=-1)?max_d:MAX_DISTANCE);
	int ndistance = 0;
	int id = -1;
	int cx = (int)parts[ci].x;
	int cy = (int)parts[ci].y;
	for (int i = 0; i <= globalSim->parts_lastActiveIndex; i++)
	{
		if (parts[i].type && !parts[i].life && i!=ci && parts[i].type!=PT_LIGH && parts[i].type!=PT_THDR && parts[i].type!=PT_NEUT && parts[i].type!=PT_PHOT)
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

int contact_part(int i, int tp)
{
	int x=(int)parts[i].x, y=(int)parts[i].y;
	int r,rx,ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==tp)
					return r>>8;
			}
	return -1;
}

int create_LIGH(Simulation *sim, int x, int y, int c, int temp, int life, int tmp, int tmp2, bool last)
{
	int p = sim->part_create(-1, x, y,c);
	if (p != -1)
	{
		parts[p].temp = (float)temp;
		parts[p].tmp = tmp;
		if (last)
		{
			sim->parts[p].tmp2=1+(rand()%200>tmp2*tmp2/10+60);
			sim->parts[p].life=(int)(life/1.5-rand()%2);
		}
		else
		{
			parts[p].life = life;
			parts[p].tmp2 = 0;
		}
	}
	else if (x >= 0 && x < XRES && y >= 0 && y < YRES)
	{
		int r = pmap[y][x];
		if ((((r&0xFF)==PT_VOID || ((r&0xFF)==PT_PVOD && parts[r>>8].life >= 10)) && (!parts[r>>8].ctype || (parts[r>>8].ctype==c)!=(parts[r>>8].tmp&1))) || (r&0xFF)==PT_BHOL || (r&0xFF)==PT_NBHL) // VOID, PVOD, VACU, and BHOL eat LIGH here
			return 1;
	}
	else
		return 1;
	return 0;
}

void create_line_par(Simulation *sim, int x1, int y1, int x2, int y2, int c, int temp, int life, int tmp, int tmp2)
{
	int reverseXY=abs(y2-y1)>abs(x2-x1), back = 0, x, y, dx, dy, Ystep;
	float e = 0.0f, de;
	if (reverseXY)
	{
		y = x1;
		x1 = y1;
		y1 = y;
		y = x2;
		x2 = y2;
		y2 = y;
	}
	if (x1 > x2)
		back = 1;
	dx = x2 - x1;
	dy = abs(y2 - y1);
	if (dx)
		de = dy/(float)dx;
	else
		de = 0.0f;
	y = y1;
	Ystep = (y1<y2) ? 1 : -1;
	if (!back)
	{
		for (x = x1; x <= x2; x++)
		{
			int ret;
			if (reverseXY)
				ret = create_LIGH(sim, y, x, c, temp, life, tmp, tmp2, x==x2);
			else
				ret = create_LIGH(sim, x, y, c, temp, life, tmp, tmp2, x==x2);
			if (ret)
				return;

			e += de;
			if (e >= 0.5f)
			{
				y += Ystep;
				e -= 1.0f;
			}
		}
	}
	else
	{
		for (x = x1; x >= x2; x--)
		{
			int ret;
			if (reverseXY)
				ret = create_LIGH(sim, y, x, c, temp, life, tmp, tmp2, x==x2);
			else
				ret = create_LIGH(sim, x, y, c, temp, life, tmp, tmp2, x==x2);
			if (ret)
				return;

			e += de;
			if (e <= -0.5f)
			{
				y += Ystep;
				e += 1.0f;
			}
		}
	}
}

int FIRE_update(UPDATE_FUNC_ARGS);

int LIGH_update(UPDATE_FUNC_ARGS)
{
	/*
	 *
	 * tmp2:
	 * -1 - part will be removed
	 * 0 - "branches" of the lightning
	 * 1 - bending
	 * 2 - branching
	 * 3 - transfer spark or make destruction
	 * 4 - first pixel
	 *
	 * life - "thickness" of lighting (but anyway one pixel)
	 *
	 * tmp - angle of lighting, measured in degrees anticlockwise from the positive x direction
	 *
	*/
	int r,rx,ry,rt, multipler, powderful=(int)(parts[i].temp*(1+parts[i].life/40)*LIGHTING_POWER);
	float angle, angle2=-1;
	FIRE_update(UPDATE_FUNC_SUBCALL_ARGS);
	if (aheat_enable)
	{
		hv[y/CELL][x/CELL]+=powderful/50;
		if (hv[y/CELL][x/CELL]>MAX_TEMP)
			hv[y/CELL][x/CELL]=MAX_TEMP;
	}

	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				rt = r&0xFF;

				if (ptypes[rt].properties & PROP_INDESTRUCTIBLE)
				{
					if (sim->elements[rt].HeatConduct)
						parts[r>>8].temp = restrict_flt(parts[r>>8].temp+powderful/10, MIN_TEMP, MAX_TEMP);
					continue;
				}
				switch (rt)
				{
				case PT_LIGH:
				case PT_TESC:
					continue;
				case PT_THDR:
				case PT_CLNE:
				case PT_FIRE:
					parts[r>>8].temp = restrict_flt(parts[r>>8].temp+powderful/10, MIN_TEMP, MAX_TEMP);
					continue;
				case PT_DEUT:
				case PT_PLUT:
					//start nuclear reactions
					parts[r>>8].temp = restrict_flt(parts[r>>8].temp+powderful, MIN_TEMP, MAX_TEMP);
					pv[y/CELL][x/CELL] += powderful/35;
					if (!(rand()%3))
					{
						part_change_type(r>>8,x+rx,y+ry,PT_NEUT);
						parts[r>>8].life = rand()%480+480;
						parts[r>>8].vx=rand()%10-5.0f;
						parts[r>>8].vy=rand()%10-5.0f;
					}
					break;
				case PT_COAL:
				case PT_BCOL:
					//ignite coal
					if (parts[r>>8].life > 100)
						parts[r>>8].life = 99;
					break;
				case PT_STKM:
					if (((STKM_ElementDataContainer*)sim->elementData[PT_STKM])->GetStickman1()->elem != PT_LIGH)
						parts[r>>8].life -= powderful/100;
					break;
				case PT_STKM2:
					if (((STKM_ElementDataContainer*)sim->elementData[PT_STKM])->GetStickman2()->elem != PT_LIGH)
						parts[r>>8].life -= powderful/100;
					break;
				default:
					break;
				}
				if ((ptypes[rt].properties&PROP_CONDUCTS) && !parts[r>>8].life)
				{
					sim->spark_conductive(r>>8, x+rx, y+ry);
				}
				pv[y/CELL][x/CELL] += powderful/400;
				if (sim->elements[rt].HeatConduct)
					parts[r>>8].temp = restrict_flt(parts[r>>8].temp+powderful/1.3f, MIN_TEMP, MAX_TEMP);
			}
	if (parts[i].tmp2 == 3)
	{
		parts[i].tmp2=0;
		return 1;
	}
	else if (parts[i].tmp2 == -1)
	{
		kill_part(i);
		return 1;
	}
	else if (parts[i].tmp2 <= 0 || parts[i].life<=1)
	{
		if (parts[i].tmp2>0)
			parts[i].tmp2=0;
		parts[i].tmp2--;
		return 1;
	}

	//Completely broken and laggy function, possibly can be fixed later
	/*near = LIGH_nearest_part(i, (int)(parts[i].life*2.5));
	if (near!=-1)
	{
		int t=parts[near].type;
		float n_angle; // angle to nearest part
		float angle_diff;
		rx=(int)parts[near].x-x;
		ry=(int)parts[near].y-y;
		if (rx!=0 || ry!=0)
			n_angle = atan2f((float)-ry, (float)rx);
		else
			n_angle = 0;
		if (n_angle<0)
			n_angle+=M_PI*2;
		angle_diff = fabsf(n_angle-parts[i].tmp*M_PI/180);
		if (angle_diff>M_PI)
			angle_diff = M_PI*2 - angle_diff;
		if (parts[i].life<5 || angle_diff<M_PI*0.8) // lightning strike
		{
			create_line_par(sim, x, y, x+rx, y+ry, PT_LIGH, (int)parts[i].temp, parts[i].life, parts[i].tmp-90, 0);

			if (t!=PT_TESC)
			{
				near=contact_part(near, PT_LIGH);
				if (near!=-1)
				{
					parts[near].tmp2=3;
					parts[near].life=(int)(1.0*parts[i].life/2-1);
					parts[near].tmp=parts[i].tmp-180;
					parts[near].temp=parts[i].temp;
				}
			}
		}
		else near=-1;
	}*/

	//if (parts[i].tmp2==1/* || near!=-1*/)
	//angle=0;//parts[i].tmp-30+rand()%60;
	angle = (float)((parts[i].tmp-30+rand()%60)%360);
	multipler = (int)(parts[i].life*1.5+rand()%((int)(parts[i].life+1)));
	rx = (int)(cos(angle*M_PI/180)*multipler);
	ry = (int)(-sin(angle*M_PI/180)*multipler);
	create_line_par(sim, x, y, x+rx, y+ry, PT_LIGH, (int)parts[i].temp, parts[i].life, (int)angle, parts[i].tmp2);

	if (parts[i].tmp2 == 2)// && pNear==-1)
	{
		angle2 = (float)(((int)angle+100-rand()%200)%360);
		multipler = (int)(parts[i].life*1.5+rand()%((int)(parts[i].life+1)));
		rx = (int)(cos(angle2*M_PI/180)*multipler);
		ry = (int)(-sin(angle2*M_PI/180)*multipler);
		create_line_par(sim, x, y, x+rx, y+ry, PT_LIGH, (int)parts[i].temp, parts[i].life, (int)angle2, parts[i].tmp2);
	}

	parts[i].tmp2=-1;
	return 1;
}

int LIGH_graphics(GRAPHICS_FUNC_ARGS)
{
	*firea = 120;
	*firer = *colr = 235;
	*fireg = *colg = 245;
	*fireb = *colb = 255;
	*pixel_mode |= PMODE_GLOW | FIRE_ADD;
	return 1;
}

void LIGH_create(ELEMENT_CREATE_FUNC_ARGS)
{
	float gx, gy, gsize;
	if (v >= 0)
	{
		if (v > 55)
			v = 55;
		sim->parts[i].life = v;
	}
	else
		sim->parts[i].life = 30;
	sim->parts[i].temp = sim->parts[i].life*150.0f; // temperature of the lightning shows the power of the lightning
	get_gravity_field(x, y, 1.0f, 1.0f, &gx, &gy);
	gsize = gx*gx+gy*gy;
	if (gsize<0.0016f)
	{
		float angle = (rand()%6284)*0.001f;//(in radians, between 0 and 2*pi)
		gsize = sqrtf(gsize);
		// randomness in weak gravity fields (more randomness with weaker fields)
		gx += cosf(angle)*(0.04f-gsize);
		gy += sinf(angle)*(0.04f-gsize);
	}
	sim->parts[i].tmp = (((int)(atan2f(-gy, gx)*(180.0f/M_PI)))+rand()%40-20+360)%360;
	sim->parts[i].tmp2 = 4;
}

void LIGH_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_LIGH";
	elem->Name = "LIGH";
	elem->Colour = COLPACK(0xFFFFC0);
	elem->MenuVisible = 1;
	elem->MenuSection = SC_EXPLOSIVE;
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
	elem->Hardness = 1;

	elem->Weight = 100;

	elem->DefaultProperties.temp = R_TEMP+0.0f	+273.15f;
	elem->HeatConduct = 0;
	elem->Latent = 0;
	elem->Description = "More realistic lightning. Set pen size to set the size of the lightning.";

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

	elem->Update = &LIGH_update;
	elem->Graphics = &LIGH_graphics;
	elem->Func_Create = &LIGH_create;
	elem->Init = &LIGH_init_element;
}
