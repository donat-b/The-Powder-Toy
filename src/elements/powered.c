#include <element.h>

int update_POWERED(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	if ((parts[i].type == PT_PPTI || parts[i].type == PT_PPTO) && parts[i].tmp2>0 && parts[i].tmp2!=10)
		parts[i].tmp2--;
	else if (parts[i].life>0 && parts[i].life!=10)
		parts[i].life--;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((parts[i].type != PT_SWCH && parts[i].type != PT_ACTV) || parts_avg(i,r>>8,PT_INSL)!=PT_INSL)
				{
					if ((r&0xFF)==PT_SPRK && parts[r>>8].life>0 && parts[r>>8].life<4)
					{
						int tmp = parts[i].tmp;
						if (parts[i].type == PT_PCLN)
							tmp = parts[i].tmp2;
						else if (parts[i].type == PT_ANIM || parts[i].type == PT_PPTI || parts[i].type == PT_PPTO)
							tmp = 1;
						else if (parts[i].type == PT_PBCN)
							tmp = 0;

						if (tmp != 1)
						{
							if (parts[r>>8].ctype==PT_PSCN && parts[i].life < 10)
								parts[i].life = 10;
							else if (parts[r>>8].ctype==PT_NSCN)
								parts[i].life = 9;
							else if ((parts[i].type == PT_SWCH || parts[i].type == PT_ACTV) && parts[r>>8].ctype != PT_PSCN && parts[r>>8].ctype != PT_NSCN && !(parts[r>>8].ctype == PT_INWR && parts[r>>8].tmp == 1) && parts[i].life == 10)
							{
								parts[i].ctype = parts[i].type;
								part_change_type(i,x,y,PT_SPRK);
								parts[i].life = 4;
								return 0;
							}
						}
						else if (parts[i].type == PT_PPTI || parts[i].type == PT_PPTO)
						{
							int tentmp2 = 10, ninetmp2 = 9;
							if (parts[r>>8].ctype==PT_PSCN && parts[i].tmp2 < 10)
								flood_prop(x,y,offsetof(particle, tmp2),&tentmp2,0);
							else if (parts[r>>8].ctype==PT_NSCN && parts[i].tmp2 >= 10)
								flood_prop(x,y,offsetof(particle, tmp2),&ninetmp2,0);
						}
						else if (parts[i].type == PT_ANIM) //unused, it causes small problems because it doesn't set tmp depending on if the particle has been updated yet
						{
							int tenlife = 10, ninelife = 9, twelvelife = 12, zero = 0;
							if (parts[r>>8].ctype==PT_PSCN && parts[i].life < 10)
								flood_prop(x,y,offsetof(particle, life),&tenlife,0);
							else if (parts[r>>8].ctype==PT_NSCN && parts[i].life >= 10)
							{
								flood_prop(x,y,offsetof(particle, life),&ninelife,0);
								flood_prop(x,y,offsetof(particle, tmp),&zero,0);
								flood_prop(x,y,offsetof(particle, tmp2),&zero,0);
							}
							else if (parts[r>>8].ctype==PT_METL)
							{
								if (parts[i].life == 10)
									flood_prop(x,y,offsetof(particle, life),&ninelife,0);
								else if (parts[i].life == 0)
									flood_prop(x,y,offsetof(particle, life),&twelvelife,0);
							}
						}
						else
						{
							int tenlife = 10, ninelife = 9;
							if (parts[r>>8].ctype==PT_PSCN && parts[i].life < 10)
								flood_prop(x,y,offsetof(particle, life),&tenlife,0);
							else if (parts[r>>8].ctype==PT_NSCN && parts[i].life >= 10)
								flood_prop(x,y,offsetof(particle, life),&ninelife,0);
							else if ((parts[i].type == PT_SWCH || parts[i].type == PT_ACTV) && parts[r>>8].ctype != PT_PSCN && parts[r>>8].ctype != PT_NSCN && !(parts[r>>8].ctype == PT_INWR && parts[r>>8].tmp == 1) && parts[i].life == 10)
							{
								parts[i].ctype = parts[i].type;
								part_change_type(i,x,y,PT_SPRK);
								parts[i].life = 4;
								return 0;
							}
						}
					}
					if ((r&0xFF)==parts[i].type)
					{
						if (parts[i].life==10&&parts[r>>8].life<10&&parts[r>>8].life>0)
							parts[i].life = 9;
						else if (parts[i].life==0&&parts[r>>8].life==10)
							parts[i].life = 10;
					}
				}
			}
	return 0;
}