#include <element.h>
 
int update_VIRS(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	if (parts[i].tmp&0xFF)
	{
		parts[i].tmp -= rand()%2 < 1 ? 0:1;
		if ((parts[i].tmp&0xFF) == 0)
		{
			parts[i].type = parts[i].tmp2;
			parts[i].tmp2 = 0;
			parts[i].tmp = 0;
			return 0;
		}
	}
	if ((parts[i].tmp&0xFF00) > 0)
	{
		if (rand()%20 < 1)
			parts[i].tmp -= 256;
	}
	else
	{
		kill_part(i);
		return 1;
	}
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (((r&0xFF) == PT_VIRS || (r&0xFF) == PT_VRSS || (r&0xFF) == PT_VRSG) && (parts[r>>8].tmp&0xFF) && !(parts[i].tmp&0xFF))
				{
					int newtmp = (parts[r>>8].tmp&0xFF) + (rand()%6 < 1 ? 1:2);
					if (newtmp > 255)
						newtmp = 255;
					parts[i].tmp = (parts[i].tmp&0xFF00) + newtmp;
				}
				else if ((!(parts[i].tmp&0xFF) || (parts[i].tmp&0xFF) > 10) && (r&0xFF) == PT_CURE)
				{
					parts[i].tmp = (parts[i].tmp&0xFF00) + 10;
				}
				else if (!(parts[i].tmp&0xFF) && (r&0xFF) != PT_VIRS && (r&0xFF) != PT_VRSS && (r&0xFF) != PT_VRSG)
				{
					if (rand()%50<1)
					{
						int newtmp = parts[i].tmp + (rand()%3 < 1 ? 0:256);
						if (newtmp >= 65536)
							newtmp = 65280;
						parts[r>>8].tmp2 = (r&0xFF);
						parts[r>>8].tmp = newtmp;
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
