#include <element.h>

int update_ACTV(UPDATE_FUNC_ARGS) {
	int r, rt, rx, ry;
	if (parts[i].life>0 && parts[i].life!=10)
		parts[i].life--;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (parts_avg(i,r>>8,PT_INSL)!=PT_INSL) {
					rt = r&0xFF;
					if (rt==PT_ACTV)
					{
						if (parts[i].life>=10&&parts[r>>8].life<10&&parts[r>>8].life>0)
							parts[i].life = 9;
						else if (parts[i].life==0&&parts[r>>8].life==10)
							parts[i].life = 10;
					}
					else if (rt==PT_SPRK&&parts[i].life==10&&parts[r>>8].ctype!=PT_PSCN&&parts[r>>8].ctype!=PT_NSCN) {
						part_change_type(i,x,y,PT_SPRK);
						parts[i].ctype = PT_ACTV;
						parts[i].life = 4;
					}
				}
			}
	return 0;
}
