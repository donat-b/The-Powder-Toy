#include <element.h>

int update_MOVS(UPDATE_FUNC_ARGS) {
	int bn = parts[i].life;
	float tmp = 0, tmp2 = 0;
	int type;
	if (!msindex[bn])
	{
		if (rand()%100<1)
		{
			kill_part(i);
			return 1;
		}
		tmp = parts[i].tmp;
		tmp2 = parts[i].tmp2;
	}
	else if (parts[i].tmp != 0)
	{
		float angle = atan((float)parts[i].tmp2/parts[i].tmp);
		float distance = sqrt(pow((float)parts[i].tmp,2)+pow((float)parts[i].tmp2,2));
		if (parts[i].tmp < 0)
			angle += 3.1415926535f;
		tmp = distance*cos(angle+msrotation[bn]);
		tmp2 = distance*sin(angle+msrotation[bn]);
	}
	else if (parts[i].tmp2 != 0)
	{
		float angle = 3.1415926535f/2;
		tmp = parts[msindex[bn]].x + parts[i].tmp2*cos(angle+msrotation[bn]);
		if (parts[i].tmp2 < 0)
			tmp2 = parts[i].tmp2*sin(angle+msrotation[bn]);
		else
			tmp2 = -1*parts[i].tmp2*sin(angle+msrotation[bn]);
	}
	if (!tmp && !tmp2 && pv[y/CELL][x/CELL] > 10 || pv[y/CELL][x/CELL] < -10)
	{
		kill_part(i);
		return 1;
	}
	type = pmap[y+1][x]&0xFF;
	if (y+1 < YRES && tmp2 > 0 && type && (type != PT_MOVS || (type == PT_MOVS && parts[pmap[y+1][x]>>8].life != bn)))
	{
		parts[i].vy -= tmp2;
		newmsrotation[bn] -= tmp/5000;
	}
	type = pmap[y-1][x]&0xFF;
	if (y-1 >= 0 && tmp2 < 0 && type && (type != PT_MOVS || (type == PT_MOVS && parts[pmap[y-1][x]>>8].life != bn)))
	{
		parts[i].vy -= tmp2;
		newmsrotation[bn] -= tmp/5000;
	}
	type = pmap[y][x+1]&0xFF;
	if (x+1 < XRES && tmp > 0 && type && (type != PT_MOVS || (type == PT_MOVS && parts[pmap[y][x+1]>>8].life != bn)))
	{
		parts[i].vx -= tmp;
		newmsrotation[bn] -= tmp2/5000;
	}
	type = pmap[y][x-1]&0xFF;
	if (x-1 >= 0 && tmp < 0 && type && (type != PT_MOVS || (type == PT_MOVS && parts[pmap[y][x-1]>>8].life != bn)))
	{
		parts[i].vx -= tmp;
		newmsrotation[bn] -= tmp2/5000;
	}
	return 0;
}
