#include <element.h>

int update_MOVS(UPDATE_FUNC_ARGS) {
	int bn = parts[i].life;
	if (y+1 < YRES && (pmap[y+1][x]&0xFF) && ((pmap[y+1][x]&0xFF) != PT_MOVS || parts[pmap[y+1][x]>>8].life != bn))
		parts[i].vy -= parts[i].tmp2;
	if (y-1 >= 0   && (pmap[y-1][x]&0xFF) && ((pmap[y+1][x]&0xFF) != PT_MOVS || parts[pmap[y-1][x]>>8].life != bn))
		parts[i].vy -= parts[i].tmp2;
	if (x+1 < XRES && (pmap[y][x+1]&0xFF) && ((pmap[y][x+1]&0xFF) != PT_MOVS || parts[pmap[y][x+1]>>8].life != bn))
		parts[i].vx -= parts[i].tmp;
	if (y-1 >= 0   && (pmap[y][x-1]&0xFF) && ((pmap[y][x-1]&0xFF) != PT_MOVS || parts[pmap[y][x-1]>>8].life != bn))
		parts[i].vx -= parts[i].tmp;
	return 0;
}
