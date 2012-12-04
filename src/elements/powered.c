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
					if ((r&0xFF)==parts[i].type && parts[i].type == PT_SWCH)
					{
						if (parts[i].life>=10&&parts[r>>8].life<10&&parts[r>>8].life>0)
							parts[i].life = 9;
						else if (parts[i].life==0&&parts[r>>8].life>=10)
						{
							//Set to other particle's life instead of 10, otherwise spark loops form when SWCH is sparked while turning on
							parts[i].life = parts[r>>8].life;
						}
					}
					if ((r&0xFF)==PT_SPRK && ((parts[r>>8].life>0 && parts[r>>8].life<4) || parts[i].type == PT_SWCH))
					{
						if (!(parts[i].flags & FLAG_INSTACTV))
						{
							if ((parts[i].type == PT_PUMP || parts[i].type == PT_GPMP || parts[i].type == PT_HSWC || parts[i].type == PT_PBCN))
								continue;
							if (parts[i].type != PT_SWCH && parts[r>>8].ctype==PT_PSCN && parts[i].life < 10)
								parts[i].life = 10;
							else if (parts[i].type != PT_SWCH && parts[r>>8].ctype==PT_NSCN)
								parts[i].life = 9;
							else if ((parts[i].type == PT_SWCH || parts[i].type == PT_ACTV) && parts[r>>8].ctype != PT_PSCN && parts[r>>8].ctype != PT_NSCN && !(parts[r>>8].ctype == PT_INWR && parts[r>>8].tmp == 1) && parts[i].life == 10)
							{
								parts[i].ctype = parts[i].type;
								part_change_type(i,x,y,PT_SPRK);
								parts[i].life = 4;
								return 0;
							}
						}
						else if ((parts[i].type == PT_PPTI || parts[i].type == PT_PPTO) && parts[r>>8].life>2)
						{
							int tentmp2 = 10, ninetmp2 = 9, skipmoveflags = parts[i].flags|FLAG_SKIPMOVE;
							if (parts[r>>8].ctype==PT_PSCN && parts[i].tmp2 < 10)
							{
								flood_prop(x,y,offsetof(particle, tmp2),&tentmp2,0);
								flood_prop(x,y,offsetof(particle, flags),&skipmoveflags,0);
							}
							else if (parts[r>>8].ctype==PT_NSCN && parts[i].tmp2 >= 10)
							{
								flood_prop(x,y,offsetof(particle, tmp2),&ninetmp2,0);
								flood_prop(x,y,offsetof(particle, flags),&skipmoveflags,0);
							}
						}
						else if (parts[i].type == PT_ANIM && parts[r>>8].life>2)
						{
							int tenlife = 10, ninelife = 9, twelvelife = 12, zero = 0, skipmoveflags = parts[i].flags|FLAG_SKIPMOVE;
							if (parts[r>>8].ctype==PT_PSCN && parts[i].life < 10)
							{
								flood_prop(x,y,offsetof(particle, life),&tenlife,0);
								flood_prop(x,y,offsetof(particle, flags),&skipmoveflags,0);
							}
							else if (parts[r>>8].ctype==PT_NSCN && parts[i].life >= 10)
							{
								flood_prop(x,y,offsetof(particle, life),&ninelife,0);
								flood_prop(x,y,offsetof(particle, tmp),&zero,0);
								flood_prop(x,y,offsetof(particle, tmp2),&zero,0);
								flood_prop(x,y,offsetof(particle, flags),&skipmoveflags,0);
							}
							else if (parts[r>>8].ctype==PT_METL && parts[r>>8].life>2)
							{
								if (parts[i].life == 10)
								{
									flood_prop(x,y,offsetof(particle, life),&ninelife,0);
									flood_prop(x,y,offsetof(particle, flags),&skipmoveflags,0);
								}
								else if (parts[i].life == 0)
								{
									flood_prop(x,y,offsetof(particle, life),&twelvelife,0);
									flood_prop(x,y,offsetof(particle, flags),&skipmoveflags,0);
									return 1;
								}
							}
						}
						else
						{
							int tenlife = 10, ninelife = 9, skipmoveflags = parts[i].flags|FLAG_SKIPMOVE;
							if (parts[r>>8].ctype==PT_PSCN && parts[i].life < 10)
							{
								flood_prop(x,y,offsetof(particle, life),&tenlife,0);
								flood_prop(x,y,offsetof(particle, flags),&skipmoveflags,0);
							}
							else if (parts[r>>8].ctype==PT_NSCN && parts[i].life >= 10)
							{
								flood_prop(x,y,offsetof(particle, life),&ninelife,0);
								flood_prop(x,y,offsetof(particle, flags),&skipmoveflags,0);
							}
							else if ((parts[i].type == PT_SWCH || parts[i].type == PT_ACTV) && parts[r>>8].ctype != PT_PSCN && parts[r>>8].ctype != PT_NSCN && !(parts[r>>8].ctype == PT_INWR && parts[r>>8].tmp == 1) && parts[i].life == 10)
							{
								parts[i].ctype = parts[i].type;
								part_change_type(i,x,y,PT_SPRK);
								parts[i].life = 4;
								return 0;
							}
						}
					}
					if ((r&0xFF)==parts[i].type && parts[i].type != PT_SWCH)
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