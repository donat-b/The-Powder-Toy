#include <element.h>

int update_PCLN(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	if (parts[i].ctype<=0 || parts[i].ctype>=PT_NUM || (parts[i].ctype==PT_LIFE && (parts[i].tmp<0 || parts[i].tmp>=NGOLALT)))
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES)
				{
					r = photons[y+ry][x+rx];
					if (!r)
						r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (!(ptypes[r&0xFF].properties&PROP_CLONE) &&
						!(ptypes[r&0xFF].properties&PROP_BREAKABLECLONE) &&
				        (r&0xFF)!=PT_SPRK && (r&0xFF)!=PT_NSCN && 
						(r&0xFF)!=PT_PSCN && (r&0xFF)!=PT_STKM && 
						(r&0xFF)!=PT_STKM2 && (r&0xFF)<PT_NUM)
					{
						parts[i].ctype = r&0xFF;
						if ((r&0xFF)==PT_LIFE)
							parts[i].tmp = parts[r>>8].ctype;
					}
				}
	if (parts[i].ctype>0 && parts[i].ctype<PT_NUM && parts[i].life==10) {
		if (parts[i].ctype==PT_PHOT) {//create photons a different way
			for (rx=-1; rx<2; rx++) {
				for (ry=-1; ry<2; ry++)
				{
					if (rx || ry)
					{
						int r = create_part(-1, x+rx, y+ry, parts[i].ctype);
						if (r!=-1)
						{
							parts[r].vx = rx*3.0f;
							parts[r].vy = ry*3.0f;
							if (r>i)
							{
								// Make sure movement doesn't happen until next frame, to avoid gaps in the beams of photons produced
								parts[r].flags |= FLAG_SKIPMOVE;
							}
						}
					}
				}
			}
		}
		else if (parts[i].ctype==PT_LIFE) {//create life a different way
			for (rx=-1; rx<2; rx++) {
				for (ry=-1; ry<2; ry++) {
					create_part(-1, x+rx, y+ry, parts[i].ctype|(parts[i].tmp<<8));
				}
			}
		} else {
			create_part(-1, x+rand()%3-1, y+rand()%3-1, parts[i].ctype);
		}
	}
	return 0;
}
