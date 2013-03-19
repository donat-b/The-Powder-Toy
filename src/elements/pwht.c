#include <element.h>

int PWHT_flood(UPDATE_FUNC_ARGS)
{
	int format = parts[i].tmp2, pwht_property;
	size_t propoffset;

	if (parts[i].ctype == 0)
		propoffset = offsetof(particle, type);
	else if (parts[i].ctype == 1)
		propoffset = offsetof(particle, life);
	else if (parts[i].ctype == 2)
		propoffset = offsetof(particle, ctype);
	else if (parts[i].ctype == 3)
		propoffset = offsetof(particle, temp);
	else if (parts[i].ctype == 4)
		propoffset = offsetof(particle, tmp);
	else if (parts[i].ctype == 5)
		propoffset = offsetof(particle, tmp2);
	else if (parts[i].ctype == 6)
		propoffset = offsetof(particle, vy);
	else if (parts[i].ctype == 7)
		propoffset = offsetof(particle, vx);
	else if (parts[i].ctype == 8)
		propoffset = offsetof(particle, x);
	else if (parts[i].ctype == 9)
		propoffset = offsetof(particle, y);
	else if (parts[i].ctype == 10)
		propoffset = offsetof(particle, dcolour);
	else if (parts[i].ctype == 11)
		propoffset = offsetof(particle, flags);
	else
	{
		parts[i].ctype = 0;
		parts[i].tmp2 = 0;
	}
	
	if (format == 2)
	{
		float valuef = parts[i].temp;
		return flood_prop(x, y-1, propoffset, &valuef, format);
	}
	else if (format == 3)
	{
		unsigned int valueui = parts[i].dcolour;
		return flood_prop(x, y-1, propoffset, &valueui, 0);
	}
	else
	{
		int valuei = parts[i].tmp;
		return flood_prop(x, y-1, propoffset, &valuei, format);
	}
}

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
				if (BOUNDS_CHECK)
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
			PWHT_flood(UPDATE_FUNC_SUBCALL_ARGS);
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