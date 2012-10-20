#include <element.h>
 
int update_VIRS(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	if (parts[i].pavg[0])
	{
		parts[i].pavg[0] -= rand()%2 < 1 ? 0:1;
		if ((parts[i].pavg[0]) == 0)
		{
			part_change_type(i,x,y,parts[i].tmp2);
			parts[i].tmp2 = 0;
			parts[i].pavg[0] = 0;
			parts[i].pavg[1] = 0;
			return 0;
		}
	}
	if (parts[i].pavg[1] > 0)
	{
		if (rand()%15 < 1)
			parts[i].pavg[1]--;
	}
	else if (!parts[i].pavg[0])
	{
		kill_part(i);
		return 1;
	}
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = ((pmap[y+ry][x+rx]&0xFF)==PT_PINV&&parts[pmap[y+ry][x+rx]>>8].life==10)?0:pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (((r&0xFF) == PT_VIRS || (r&0xFF) == PT_VRSS || (r&0xFF) == PT_VRSG) && parts[r>>8].pavg[0] && !parts[i].pavg[0])
				{
					int newtmp = (int)parts[r>>8].pavg[0] + (rand()%6 < 1 ? 1:2);
					parts[i].pavg[0] = (float)newtmp;
				}
				else if (!(parts[i].pavg[0] || parts[i].pavg[0] > 10) && (r&0xFF) == PT_CURE)
				{
					parts[i].pavg[0] += 10;
					if (rand()%10<1)
						kill_part(r>>8);
				}
				else if (!parts[i].pavg[0] && (r&0xFF) != PT_VIRS && (r&0xFF) != PT_VRSS && (r&0xFF) != PT_VRSG && !(ptypes[r&0xFF].properties&PROP_INDESTRUCTIBLE))
				{
					if (rand()%50<1)
					{
						int newtmp = (int)parts[i].pavg[1] + (rand()%3 < 1 ? 0:1);
						parts[r>>8].tmp2 = (r&0xFF);
						parts[r>>8].pavg[0] = 0;
						parts[r>>8].pavg[1] = (float)newtmp;
						if (parts[r>>8].temp < 305)
							part_change_type(r>>8,x,y,PT_VRSS);
						else if (parts[r>>8].temp > 673)
							part_change_type(r>>8,x,y,PT_VRSG);
						else
							part_change_type(r>>8,x,y,PT_VIRS);
					}
				}
			}
	return 0;
}
