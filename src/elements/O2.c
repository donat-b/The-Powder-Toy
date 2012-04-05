#include <element.h>

int update_O2(UPDATE_FUNC_ARGS)
{
	int r,rx,ry;
	if (parts[i].temp < 9773.15)
	{
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;

					if ((r&0xFF)==PT_FIRE)
					{
						parts[r>>8].temp+=(rand()/(RAND_MAX/100));
						if(parts[r>>8].tmp&0x01)
						parts[r>>8].temp=3473;
						parts[r>>8].tmp |= 2;
					}
					if ((r&0xFF)==PT_FIRE || (r&0xFF)==PT_PLSM)
					{
						create_part(i,x,y,PT_FIRE);
						parts[i].temp+=(rand()/(RAND_MAX/100));
						parts[i].tmp |= 2;
					}

				}
	}
	else if (parts[i].temp > 9973.15 && pv[y/CELL][x/CELL] > 250.0f && abs((int)gravx[(((y/sdl_scale)/CELL)*(XRES/CELL))+((x/sdl_scale)/CELL)]) + abs((int)gravy[(((y/sdl_scale)/CELL)*(XRES/CELL))+((x/sdl_scale)/CELL)]) > 20)
	{
		if (rand()%5 < 1)
		{
			int j;
			part_change_type(i,x,y,PT_PLSM);
			parts[i].life = rand()%150+50;
			j = create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_NEUT); if (j != -1) parts[j].temp = 15000;
			j = create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_PHOT); if (j != -1) parts[j].temp = 15000;
			j = create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_BRMT); if (j != -1) parts[j].temp = 15000;
			j = create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_SING); if (j != -1) { parts[j].temp = 15000; parts[i].life = rand()%25+50; }

			parts[i].temp += 15000;
			pv[y/CELL][x/CELL] += 300;
		}
	}
	return 0;
}
