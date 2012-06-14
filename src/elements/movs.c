#include <element.h>

int update_MOVS(UPDATE_FUNC_ARGS) {
	int bn = parts[i].tmp2, type, bounce = 2;
	float tmp = 0, tmp2 = 0;
	if (bn < 0 || bn > 255)
		return 0;
	if (!msindex[bn])
	{
		if (rand()%500<1)
		{
			kill_part(i);
			return 1;
		}
		tmp = parts[i].pavg[0];
		tmp2 = parts[i].pavg[1];
	}
	else if (!ms_rotation)
	{
		tmp = parts[i].pavg[0];
		tmp2 = parts[i].pavg[1];
	}
	else if (parts[i].pavg[0] != 0)
	{
		float angle = atan((float)parts[i].pavg[1]/parts[i].pavg[0]);
		float distance = sqrt(pow((float)parts[i].pavg[0],2)+pow((float)parts[i].pavg[1],2));
		if (parts[i].pavg[0] < 0)
			angle += 3.1415926535f;
		tmp = distance*cos(angle+msrotation[bn]);
		tmp2 = distance*sin(angle+msrotation[bn]);
	}
	else if (parts[i].pavg[1] != 0)
	{
		float angle = 3.1415926535f/2;
		tmp = parts[msindex[bn]-1].x + parts[i].pavg[1]*cos(angle+msrotation[bn]);
		if (parts[i].pavg[1] < 0)
			tmp2 = parts[i].pavg[1]*sin(angle+msrotation[bn]);
		else
			tmp2 = -1*parts[i].pavg[1]*sin(angle+msrotation[bn]);
	}
	if (!tmp && !tmp2 && pv[y/CELL][x/CELL] > 10 || pv[y/CELL][x/CELL] < -10)
	{
		kill_part(i);
		return 1;
	}
	type = pmap[y+1][x]&0xFF;
	//bottom side collision
	if (y+1 < YRES && tmp2 > 0 && type && (type != parts[i].type || (type == parts[i].type && parts[pmap[y+1][x]>>8].tmp2 != bn)))
	{
		parts[i].vy -= tmp2*bounce;
		newmsrotation[bn] -= tmp/50000;
	}
	type = pmap[y-1][x]&0xFF;
	//top side collision
	if (y-1 >= 0 && tmp2 < 0 && type && (type != parts[i].type || (type == parts[i].type && parts[pmap[y-1][x]>>8].tmp2 != bn)))
	{
		parts[i].vy -= tmp2*bounce;
		newmsrotation[bn] -= tmp/50000;
	}
	type = pmap[y][x+1]&0xFF;
	//right side collision
	if (x+1 < XRES && tmp > 0 && type && (type != parts[i].type || (type == parts[i].type && parts[pmap[y][x+1]>>8].tmp2 != bn)))
	{
		parts[i].vx -= tmp*bounce;
		newmsrotation[bn] -= tmp/50000;
	}
	type = pmap[y][x-1]&0xFF;
	//left side collision
	if (x-1 >= 0 && tmp < 0 && type && (type != parts[i].type || (type == parts[i].type && parts[pmap[y][x-1]>>8].tmp2 != bn)))
	{
		parts[i].vx -= tmp*bounce;
		newmsrotation[bn] -= tmp/50000;
	}
	return 0;
}
