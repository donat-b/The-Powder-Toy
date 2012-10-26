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

void transferProp(UPDATE_FUNC_ARGS, int propOffset)
{
	int r, rx, ry, trade, transfer;
	for (trade = 0; trade < 9; trade++)
	{
		int random = rand();
		rx = random%7-3;
		ry = (random>>3)%7-3;
		if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if ((r&0xFF)!=PT_VIBR)
				continue;
			if (*((int*)(((char*)&parts[i])+propOffset)) > *((int*)(((char*)&parts[r>>8])+propOffset)))
			{
				transfer = *((int*)(((char*)&parts[i])+propOffset)) - *((int*)(((char*)&parts[r>>8])+propOffset));
				if (transfer == 1)
				{
					*((int*)(((char*)&parts[r>>8])+propOffset)) += 1;
					*((int*)(((char*)&parts[i])+propOffset)) -= 1;
					trade = 9;
				}
				else if (transfer > 0)
				{
					*((int*)(((char*)&parts[r>>8])+propOffset)) += transfer/2;
					*((int*)(((char*)&parts[i])+propOffset)) -= transfer/2;
					trade = 9;
				}
			}
		}
	}	
}

int update_VIBR(UPDATE_FUNC_ARGS) {
	int r, rx, ry, transfer, trade;
	if (!parts[i].life)
	{
		//Heat absorption code
		if (parts[i].temp>274.65f)
		{
			parts[i].ctype++;
			parts[i].temp-=3;
		}
		if (parts[i].temp<271.65f)
		{
			parts[i].ctype--;
			parts[i].temp+=3;
		}
		//Pressure absorption code
		if (pv[y/CELL][x/CELL]>2.5f)
		{
			parts[i].tmp++;
			pv[y/CELL][x/CELL]--;
		}
		if (pv[y/CELL][x/CELL]<-2.5f)
		{
			parts[i].tmp--;
			pv[y/CELL][x/CELL]++;
		}
	}
	//Release sparks before explode
	if (parts[i].life && parts[i].life < 300)
	{
		rx = rand()%3-1;
		ry = rand()%3-1;
		r = pmap[y+ry][x+rx];
		if ((r&0xFF) && (r&0xFF) != PT_BREL && (ptypes[r&0xFF].properties&PROP_CONDUCTS) && !parts[r>>8].life)
		{
			parts[r>>8].life = 4;
			parts[r>>8].ctype = r>>8;
			part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
		}
	}
	//initiate explosion counter
	if (!parts[i].life && (parts[i].ctype > 1200 || parts[i].tmp > 100 || parts[i].tmp2 > 100))
		parts[i].life = 750;
	//Release all heat
	if (parts[i].life && parts[i].life < 500)
	{
		int random = rand();
		rx = random%7-3;
		ry = (random>>3)%7-3;
		if(x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES)
		{
			r = pmap[y+ry][x+rx];
			if ((r&0xFF) && (r&0xFF)!=PT_VIBR)
			{
				parts[r>>8].temp += parts[i].ctype*6;
				parts[i].ctype -= parts[i].ctype*2;
			}
		}
	}
	//Explosion code
	if (parts[i].life == 1)
	{
		int random = rand(), index;
		create_part(i, x, y, PT_EXOT);
		parts[i].tmp2 = 100;
		index = create_part(-3,x+(random&3)-1,y+((random>>2)&3)-1,PT_ELEC);
		if (index != -1)
			parts[index].temp = 7000;
		index = create_part(-3,x+((random>>4)&3)-1,y+((random>>6)&3)-1,PT_NEUT);
		if (index != -1)
			parts[index].temp = 7000;
		index = create_part(-3,x+((random>>8)&3)-1,y+((random>>10)&3)-1,PT_PHOT);
		if (index != -1)
			parts[index].temp = 7000;
		index = create_part(-3,x+((random>>12)&3)-1,y+rand()%3-1,PT_BREL);
		if (index != -1)
			parts[index].temp = 7000;
		parts[i].temp=9000;
		pv[y/CELL][x/CELL]=200;
	}
	//Neighbor check loop
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				//Melts into EXOT
				if ((r&0xFF)==PT_EXOT && !(rand()%250))
				{
					part_change_type(i,x,y,PT_EXOT);
				}
				//Absorbs energy particles
				if (ptypes[r&0xFF].properties & TYPE_ENERGY)
				{
					parts[i].tmp2++;
					kill_part(r>>8);
				}
				if ((r&0xFF)==PT_BOYL)
				{
					part_change_type(i,x,y,PT_BVBR);
				}
			}
	transferProp(UPDATE_FUNC_SUBCALL_ARGS, offsetof(particle, tmp));
	transferProp(UPDATE_FUNC_SUBCALL_ARGS, offsetof(particle, tmp2));
	transferProp(UPDATE_FUNC_SUBCALL_ARGS, offsetof(particle, ctype));
	return 0;
}

int graphics_VIBR(GRAPHICS_FUNC_ARGS)
{
	float maxtemp = fmaxf(cpart->tmp,cpart->temp);
	int gradient = cpart->ctype/12.0f;
	if (cpart->tmp>gradient)
		gradient = cpart->tmp;
	if (cpart->tmp2>gradient)
		gradient = cpart->tmp2;
	if (gradient >= 100 || cpart->life)
	{
		*pixel_mode = PMODE_NONE;
		*pixel_mode |= FIRE_BLEND;
		*firea = 90;
		*colr = 146;
		*colg = 158;
		*colb = 113;
		*firer = *colr;
		*fireg = *colg;
		*fireb = *colb;
	}
	else if (gradient >= 94 && gradient < 100)
	{
		*colr += (int)restrict_flt((gradient-94)*19.7+100,100,218);
		*colg += (int)restrict_flt((gradient-94)*17.5+87,87,192);
		*colb += (int)restrict_flt((gradient-94)*19.7+100,100,218);
	}
	else if (gradient >= 63 && gradient < 94)
	{
		*colr += (int)restrict_flt((gradient-63)*1.58+51,51,100);
		*colg += (int)restrict_flt((gradient-63)*1.03+55,55,87);
		*colb += (int)restrict_flt((gradient-63)*1.58+51,51,100);
	}
	else if (gradient > 31 && gradient < 63)
	{
		*colr += (int)restrict_flt((gradient-31)*1.59,0,51);
		*colg += (int)restrict_flt((gradient-31)*1.72,0,55);
		*colb += (int)restrict_flt((gradient-31)*1.59,0,51);
	}
	return 0;
}
