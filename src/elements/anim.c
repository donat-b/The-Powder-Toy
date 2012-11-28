#include <element.h>

int update_ANIM(UPDATE_FUNC_ARGS) {
	int oldtmp = parts[i].tmp, oldtmp2 = parts[i].tmp2;
	if (!parts[i].animations)
	{
		kill_part(i);
		return 1;
	}
	if (parts[i].tmp >= 0 && parts[i].life == 10)
		parts[i].tmp--;
	if (parts[i].tmp == 0)
	{
		parts[i].tmp = (int)(parts[i].temp-273.15);
		parts[i].tmp2++;
	}
	if (parts[i].tmp2 > parts[i].ctype)
		parts[i].tmp2 = 0;
	parts[i].dcolour = parts[i].animations[parts[i].tmp2];
	if (parts[i].life==10)
	{
		int r, rx, ry;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)==PT_ANIM)
					{
						if (parts[r>>8].life<10&&parts[r>>8].life>0)
						{
							parts[i].life = 9;
							parts[i].tmp = parts[r>>8].tmp;
							parts[i].tmp2 = parts[r>>8].tmp2;
						}
						else if (parts[r>>8].life==0)
						{
							parts[r>>8].life = 10;
							parts[r>>8].tmp = (r>>8 > i) ? oldtmp : parts[i].tmp;
							parts[r>>8].tmp2 = (r>>8 > i) ? oldtmp2 : parts[i].tmp2;
						}
					}
				}
	}
	ISANIM = 1;
	return 0;
}