#include <element.h>

int update_PWHT(UPDATE_FUNC_ARGS) {
	int r = pmap[y-1][x];
	float temp = parts[i].temp;
	if (parts[i].life < 10)
		return 0;
	if ((pmap[y-1][x]&0xFF) == PT_PWHT)
	{
		int rx, ry;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES)
				{
					r = pmap[y+ry][x+rx];
					if(!r)
						r = photons[y+ry][x+rx];
					if ((r>>8)>=NPART || !r || (r&0xFF) != PT_PWHT)
						continue;
					kill_part(r>>8);
				}
		return 1;
	}
	else if (pmap[y-1][x]&0xFF)
	{
		if (!parts[i].ctype && !parts[i].tmp2)
			flood_prop(x,y-1,offsetof(particle, temp),&parts[i].temp,2);
		else
		{
			if (parts[i].tmp2 == 2)
				flood_prop(x,y-1,parts[i].ctype,&parts[i].temp,parts[i].tmp2);
			else
			{
				int value = (int) parts[i].temp;
				flood_prop(x,y-1,parts[i].ctype,&value,parts[i].tmp2);
			}
		}
	}
	return 0;
}

int graphics_PWHT(GRAPHICS_FUNC_ARGS)
{
	int lifemod = ((cpart->life>10?10:cpart->life)*19);
	*colb += lifemod;
	return 0;
}