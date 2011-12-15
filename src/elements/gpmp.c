#include <element.h>

int update_GPMP(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	if (parts[i].life==10)
	{
		if (parts[i].temp>=256.0+273.15)
			parts[i].temp=256.0+273.15;
		if (parts[i].temp<= -256.0+273.15)
			parts[i].temp = -256.0+273.15;

		gravmap[y/CELL][x/CELL] = 0.2f*(parts[i].temp-273.15);
		if (y+CELL<YRES && pv[y/CELL+1][x/CELL]<(parts[i].temp-273.15))
			gravmap[y/CELL+1][x/CELL] += 0.1f*((parts[i].temp-273.15)-gravmap[y/CELL+1][x/CELL]);
	}
	return 0;
}
