#include <element.h>

int update_EXPL(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!(r&0xFF))
					continue;
				if ((r&0xFF) != PT_EXPL && (r&0xFF) != PT_BOMB && !(ptypes[r&0xFF].properties&PROP_INDESTRUCTIBLE)) {
					parts[r>>8].flags |= FLAG_EXPLODE;
				}
			}
	return 0;
}

int graphics_EXPL(GRAPHICS_FUNC_ARGS)
{
	*pixel_mode |= PMODE_FLARE;
	return 0;
}
