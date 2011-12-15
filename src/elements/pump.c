#include <element.h>

int update_PUMP(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	if (parts[i].life==10)
	{
		if (parts[i].temp>=256.0+273.15)
			parts[i].temp=256.0+273.15;
		if (parts[i].temp<= -256.0+273.15)
			parts[i].temp = -256.0+273.15;

		if (pv[y/CELL][x/CELL]<(parts[i].temp-273.15))
			pv[y/CELL][x/CELL] += 0.1f*((parts[i].temp-273.15)-pv[y/CELL][x/CELL]);
		if (y+CELL<YRES && pv[y/CELL+1][x/CELL]<(parts[i].temp-273.15))
			pv[y/CELL+1][x/CELL] += 0.1f*((parts[i].temp-273.15)-pv[y/CELL+1][x/CELL]);
		if (x+CELL<XRES)
		{
			pv[y/CELL][x/CELL+1] += 0.1f*((parts[i].temp-273.15)-pv[y/CELL][x/CELL+1]);
			if (y+CELL<YRES)
				pv[y/CELL+1][x/CELL+1] += 0.1f*((parts[i].temp-273.15)-pv[y/CELL+1][x/CELL+1]);
		}
	}
	return 0;
}
