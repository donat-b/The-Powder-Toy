#include <element.h>

int update_POWERED(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	if (parts[i].life>0 && parts[i].life!=10)
		parts[i].life--;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_SPRK)
				{
					int tmp = parts[i].tmp;
					//if (parts[i].type == PT_PCLN)
					//	tmp = parts[i].tmp2;

					if (tmp != 1)
					{
						if (parts[r>>8].ctype==PT_PSCN)
							parts[i].life = 10;
						else if (parts[r>>8].ctype==PT_NSCN)
							parts[i].life = 9;
					}
					else
					{
						int tenlife = 10, ninelife = 9;
						if (parts[r>>8].ctype==PT_PSCN && parts[i].life < 10)
							flood_prop(x,y,offsetof(particle, life),&tenlife,0);
						else if (parts[r>>8].ctype==PT_NSCN && parts[i].life >= 10)
							flood_prop(x,y,offsetof(particle, life),&ninelife,0);
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
	return 0;
}