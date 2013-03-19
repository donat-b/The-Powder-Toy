#include <element.h>

int update_EXPL(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = ((pmap[y+ry][x+rx]&0xFF)==PT_PINV&&parts[pmap[y+ry][x+rx]>>8].life==10)?0:pmap[y+ry][x+rx];
				if (!(r&0xFF))
					continue;
				if (!(ptypes[r&0xFF].properties&PROP_INDESTRUCTIBLE) && (r&0xFF) != PT_EMBR) {
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
