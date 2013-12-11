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

int NPTCT_update(UPDATE_FUNC_ARGS);

int SPRK_update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, rt, nearp, pavg, ct = parts[i].ctype, rd = 2;
	update_PYRO(UPDATE_FUNC_SUBCALL_ARGS);

	if (parts[i].life<=0)
	{
		if (ct==PT_WATR||ct==PT_SLTW||ct==PT_PSCN||ct==PT_NSCN||ct==PT_ETRD||ct==PT_INWR)
			parts[i].temp = R_TEMP + 273.15f;
		if (ct<=0 || ct>=PT_NUM || !ptypes[ct].enabled)
			ct = PT_METL;
		if (ct==PT_OTWR)
		{
			if (parts[i].tmp == 0)
				ct = PT_BREL;
			else
				parts[i].tmp--;
		}
		part_change_type(i,x,y,ct);
		parts[i].ctype = PT_NONE;
		parts[i].life = 4;
		if (ct == PT_WATR)
			parts[i].life = 64;
		else if (ct == PT_SLTW)
			parts[i].life = 54;
		else if (ct == PT_SWCH || ct == PT_BUTN)
			parts[i].life = 14;
		return 0;
	}
	switch (ct)
	{
	case PT_SPRK:
		kill_part(i);
		return 1;
	case PT_NTCT:
	case PT_PTCT:
		NPTCT_update(UPDATE_FUNC_SUBCALL_ARGS);
		break;
	case PT_ETRD:
		if (parts[i].life == 1)
		{
			nearp = nearest_part(i, PT_ETRD, -1);
			if (nearp!=-1&&parts_avg(i, nearp, PT_INSL)!=PT_INSL)
			{
				create_line(x, y, (int)(parts[nearp].x+0.5f), (int)(parts[nearp].y+0.5f), 0, 0, PT_PLSM, 0);
				part_change_type(i, x, y, ct);
				ct = parts[i].ctype = PT_NONE;
				parts[i].life = 20;
				part_change_type(nearp, (int)(parts[nearp].x+0.5f), (int)(parts[nearp].y+0.5f), PT_SPRK);
				parts[nearp].life = 9;
				parts[nearp].ctype = PT_ETRD;
			}
		}
		break;
	case PT_NBLE:
		if (parts[i].life<=1 && parts[i].temp<5273.15f)
		{
			parts[i].life = rand()%150+50;
			part_change_type(i, x, y, PT_PLSM);
			parts[i].ctype = PT_NBLE;
			if (parts[i].temp > 5273.15)
				parts[i].tmp |= 4;
			parts[i].temp = 3500;
			pv[y/CELL][x/CELL] += 1;
		}
		break;
	case PT_TESC:
		if (parts[i].tmp>300)
			parts[i].tmp=300;
		for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
		if (BOUNDS_CHECK && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if (r)
				continue;
			if (parts[i].tmp>4 && rand()%(parts[i].tmp*parts[i].tmp/20+6)==0)
			{
				int p=sim->part_create(-1, x+rx*2, y+ry*2, PT_LIGH);
				if (p!=-1)
				{
					parts[p].life=rand()%(2+parts[i].tmp/15)+parts[i].tmp/7;
					if (parts[i].life>60)
						parts[i].life=60;
					parts[p].temp=parts[p].life*parts[i].tmp/2.5;
					parts[p].tmp2=1;
					parts[p].tmp=(int)(atan2((float)-ry, (float)rx)/M_PI*360);
					parts[i].temp-=parts[i].tmp*2+parts[i].temp/5; // slight self-cooling
					if (fabs(pv[y/CELL][x/CELL])!=0.0f)
					{
						if (fabs(pv[y/CELL][x/CELL])<=0.5f)
							pv[y/CELL][x/CELL]=0;
						else
							pv[y/CELL][x/CELL]-=(pv[y/CELL][x/CELL]>0)?0.5:-0.5;
					}
				}
			}
		}
		break;
	case PT_IRON:
		for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
		if (BOUNDS_CHECK && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if (!r)
				continue;
			if ((r&0xFF) == PT_DSTW || (r&0xFF) == PT_SLTW || ((r&0xFF) == PT_WATR))
			{
				int rnd = rand()%100;
				if (!rnd)
					part_change_type(r>>8, x+rx, y+ry, PT_O2);
				else if (3 > rnd)
					part_change_type(r>>8, x+rx, y+ry, PT_H2);
			}
		}
		break;
	case PT_TUNG:
		if (parts[i].temp < 3595.0)
			parts[i].temp += (rand()%20)-4;
		break;
	case PT_COND:
		rd = parts[i].tmp2>MAX_DISTANCE?(int)MAX_DISTANCE:parts[i].tmp2;
		break;
	default:
		break;
	}
	for (rx=-rd; rx<=rd; rx++)
		for (ry=-rd; ry<=rd; ry++)
			if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				rt = r&0xFF;

				// ct = spark from material, rt = spark to material

				pavg = parts_avg(r>>8, i,PT_INSL);
				switch (rt)
				{
				case PT_SWCH:
				case PT_BUTN:
					// make sparked SWCH and BUTN turn off correctly
					if (!(parts[i].flags & FLAG_INSTACTV) && pavg != PT_INSL && parts[i].life < 4)
					{
						if (ct == PT_PSCN && parts[r>>8].life<10)
						{
							parts[r>>8].life = 10;
						}
						else if (ct == PT_NSCN)
						{
							parts[r>>8].ctype = PT_NONE;
							parts[r>>8].life = 9;
						}
					}
					break;
				case PT_SPRK:
					if (pavg != PT_INSL && parts[i].life < 4)
					{
						if (parts[r>>8].ctype == PT_SWCH || parts[r>>8].ctype == PT_BUTN)
						{
							if (!(parts[i].flags & FLAG_INSTACTV) && ct == PT_NSCN)
							{
								part_change_type(r>>8, x+rx, y+ry, parts[r>>8].ctype);
								parts[r>>8].ctype = PT_NONE;
								parts[r>>8].life = 9;
							}
						}
						else if (parts[r>>8].ctype==PT_NTCT || parts[r>>8].ctype==PT_PTCT)
						{
							if (ct == PT_METL)
								parts[r>>8].temp = 473.0f;
						}
					}
					break;
				case PT_PUMP:
				case PT_GPMP:
				case PT_HSWC:
				case PT_PBCN:
					if (!(parts[i].flags & FLAG_INSTACTV) && parts[i].life < 4) // PROP_PTOGGLE, Maybe? We seem to use 2 different methods for handling actived elements, this one seems better. Yes, use this one
					{
						if (ct==PT_PSCN)
							parts[r>>8].life = 10;
						else if (ct==PT_NSCN && parts[r>>8].life>=10)
							parts[r>>8].life = 9;
					}
					break;
				case PT_LCRY:
					if (abs(rx) < 2 && abs(ry) < 2 && parts[i].life < 4)
					{
						if (ct==PT_PSCN && parts[r>>8].tmp == 0)
							parts[r>>8].tmp = 2;
						else if (ct==PT_NSCN && parts[r>>8].tmp == 3)
							parts[r>>8].tmp = 1;
					}
					break;
				case PT_PPIP:
					if (parts[i].life == 3 && pavg!=PT_INSL)
					{
						if (ct == PT_NSCN || ct == PT_PSCN || ct == PT_INST)
							PPIP_flood_trigger(x+rx, y+ry, ct);
					}
					break;
				case PT_NTCT:
				case PT_PTCT:
				case PT_INWR:
					if (ct==PT_METL && pavg!=PT_INSL && parts[i].life<4)
					{
						parts[r>>8].temp = 473.0f;
						if (rt==PT_NTCT || rt==PT_PTCT)
							continue;
					}
					break;
				default:
					break;
				}

				//the crazy conduct checks
				if (pavg == PT_INSL)
					continue;
				if (!((ptypes[rt].properties&PROP_CONDUCTS) || rt==PT_INST || rt==PT_QRTZ))
					continue;
				if (abs(rx)+abs(ry)>=4 && rt!=PT_SWCH && ct!=PT_SWCH && ct!=PT_COND)
					continue;
				if (rt == ct && rt != PT_INST && rt != PT_COND)
					goto conduct;

				switch (ct)
				{
				case PT_INST:
					if (rt == PT_NSCN)
						goto conduct;
					continue;
				case PT_SWCH:
				case PT_BUTN:
					if (rt==PT_PSCN||rt==PT_NSCN||rt==PT_WATR||rt==PT_SLTW||rt==PT_NTCT||rt==PT_PTCT||rt==PT_INWR)
						continue;
					goto conduct;
				case PT_ETRD:
					if (rt==PT_METL||rt==PT_BMTL||rt==PT_BRMT||rt==PT_LRBD||rt==PT_RBDM||rt==PT_PSCN||rt==PT_NSCN)
						goto conduct;
					continue;
				case PT_NTCT:
					if (rt==PT_PSCN || (rt==PT_NSCN&&parts[i].temp>373.0f))
						goto conduct;
					continue;
				case PT_PTCT:
					if (rt==PT_PSCN || (rt==PT_NSCN&&parts[i].temp<373.0f))
						goto conduct;
					continue;
				case PT_INWR:
					if (rt==PT_NSCN || rt==PT_PSCN)
						goto conduct;
					continue;
				case PT_COND:
					if (rt == PT_COND && parts[i].tmp != parts[r>>8].tmp)
						continue;
					goto conduct;
				}

				switch (rt)
				{
				case PT_QRTZ:
					if ((ct==PT_NSCN||ct==PT_METL||ct==PT_PSCN) && (parts[r>>8].temp<173.15||pv[(y+ry)/CELL][(x+rx)/CELL]>8))
						goto conduct;
					continue;
				case PT_NTCT:
					if (ct==PT_NSCN || (ct==PT_PSCN&&parts[r>>8].temp>373.0f))
						goto conduct;
					continue;
				case PT_PTCT:
					if (ct==PT_NSCN || (ct==PT_PSCN&&parts[r>>8].temp<373.0f))
						goto conduct;
					continue;
				case PT_INWR:
					if (ct==PT_NSCN || ct==PT_PSCN)
						goto conduct;
					continue;
				case PT_INST:
					if (ct == PT_PSCN)
						goto conduct;
					continue;
				case PT_NBLE:
					if (parts[r>>8].temp < 5273.15)
						goto conduct;
					continue;
				case PT_PSCN:
					if (ct != PT_NSCN)
						goto conduct;
					continue;
				}

conduct:
				if (rt==PT_WATR||rt==PT_SLTW)
				{
					if (parts[r>>8].life==0 && parts[i].life<3)
					{
						part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
						if (rt==PT_WATR) parts[r>>8].life = 6;
						else parts[r>>8].life = 5;
						parts[r>>8].ctype = rt;
					}
				}
				else if (rt==PT_INST)
				{
					if (parts[r>>8].life==0 && parts[i].life<4)
					{
						flood_INST(x+rx,y+ry,PT_SPRK,PT_INST);//spark the wire
					}
				}
				else if (parts[r>>8].life==0 && parts[i].life<4)
				{
					parts[r>>8].life = 4;
					parts[r>>8].ctype = rt;
					part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
					if (parts[r>>8].temp+10.0f<673.0f&&!legacy_enable&&(rt==PT_METL||rt==PT_BMTL||rt==PT_BRMT||rt==PT_PSCN||rt==PT_NSCN||rt==PT_ETRD||rt==PT_NBLE||rt==PT_IRON))
						parts[r>>8].temp = parts[r>>8].temp+10.0f;
				}
				else if (ct==PT_ETRD && parts[i].life==5)
				{
					part_change_type(i,x,y,ct);
					parts[i].ctype = PT_NONE;
					parts[i].life = 20;
					parts[r>>8].life = 4;
					parts[r>>8].ctype = rt;
					part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
				}
			}
	return 0;
}

int SPRK_graphics(GRAPHICS_FUNC_ARGS)
{
	*firea = 80;
	
	*firer = 170;
	*fireg = 200;
	*fireb = 220;
	//*pixel_mode |= FIRE_ADD;
	*pixel_mode |= FIRE_ADD;
	return 1;
}

void SPRK_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_SPRK";
	elem->Name = "SPRK";
	elem->Colour = COLPACK(0xFFFF80);
	elem->MenuVisible = 1;
	elem->MenuSection = SC_ELEC;
	elem->Enabled = 1;

	elem->Advection = 0.0f;
	elem->AirDrag = 0.00f * CFDS;
	elem->AirLoss = 0.90f;
	elem->Loss = 0.00f;
	elem->Collision = 0.0f;
	elem->Gravity = 0.0f;
	elem->Diffusion = 0.00f;
	elem->PressureAdd_NoAmbHeat = 0.001f	* CFDS;
	elem->Falldown = 0;

	elem->Flammable = 0;
	elem->Explosive = 0;
	elem->Meltable = 0;
	elem->Hardness = 1;
	elem->PhotonReflectWavelengths = 0x00000000;

	elem->Weight = 100;

	elem->DefaultProperties.temp = R_TEMP+0.0f	+273.15f;
	elem->HeatConduct = 251;
	elem->Latent = 0;
	elem->Description = "Electricity. The basis of all electronics in TPT, travels along wires and other conductive elements.";

	elem->State = ST_SOLID;
	elem->Properties = TYPE_SOLID|PROP_LIFE_DEC;

	elem->LowPressureTransitionThreshold = IPL;
	elem->LowPressureTransitionElement = NT;
	elem->HighPressureTransitionThreshold = IPH;
	elem->HighPressureTransitionElement = NT;
	elem->LowTemperatureTransitionThreshold = ITL;
	elem->LowTemperatureTransitionElement = NT;
	elem->HighTemperatureTransitionThreshold = ITH;
	elem->HighTemperatureTransitionElement = NT;

	elem->Update = &SPRK_update;
	elem->Graphics = &SPRK_graphics;
	elem->Init = &SPRK_init_element;
}
