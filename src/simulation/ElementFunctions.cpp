#include "ElementsCommon.h"

// Interactions which only occur when legacy_enable is on
int update_legacy_all(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	switch (parts[i].type)
	{
	case PT_WTRV:
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (((r&0xFF)==PT_WATR||(r&0xFF)==PT_DSTW||(r&0xFF)==PT_SLTW) && !(rand()%1000))
					{
						part_change_type(i,x,y,PT_WATR);
						part_change_type(r>>8,x+rx,y+ry,PT_WATR);
					}
					if (((r&0xFF)==PT_ICEI || (r&0xFF)==PT_SNOW) && !(rand()%1000))
					{
						part_change_type(i,x,y,PT_WATR);
						if (!(rand()%1000))
							part_change_type(r>>8,x+rx,y+ry,PT_WATR);
					}
				}
		if (pv[y/CELL][x/CELL] > 4.0f)
			part_change_type(i,x,y,PT_DSTW);
		break;
	case PT_WATR:
	case PT_DSTW:
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (((r&0xFF)==PT_FIRE || (r&0xFF)==PT_LAVA) && !(rand()%10))
					{
						part_change_type(i,x,y,PT_WTRV);
					}
				}
		break;
	case PT_SLTW:
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (((r&0xFF)==PT_FIRE || (r&0xFF)==PT_LAVA) && !(rand()%10))
					{
						if (rand()%4==0) part_change_type(i,x,y,PT_SALT);
						else part_change_type(i,x,y,PT_WTRV);
					}
				}
		break;
	case PT_ICEI:
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (((r&0xFF)==PT_WATR || (r&0xFF)==PT_DSTW) && !(rand()%1000))
					{
						part_change_type(i,x,y,PT_ICEI);
						part_change_type(r>>8,x+rx,y+ry,PT_ICEI);
					}
				}
		break;
	case PT_SNOW:
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (((r&0xFF)==PT_WATR || (r&0xFF)==PT_DSTW) && !(rand()%1000))
					{
						part_change_type(i,x,y,PT_ICEI);
						part_change_type(r>>8,x+rx,y+ry,PT_ICEI);
					}
					if (((r&0xFF)==PT_WATR || (r&0xFF)==PT_DSTW) && 3>(rand()%200))
						part_change_type(i,x,y,PT_WATR);
				}
		break;
	case PT_OIL:
		if (pv[y/CELL][x/CELL] < -6.0f)
			part_change_type(i,x,y,PT_GAS);
		break;
	case PT_GAS:
		if (pv[y/CELL][x/CELL] > 6.0f)
			part_change_type(i,x,y,PT_OIL);
		break;
	case PT_DESL:
		if (pv[y/CELL][x/CELL] > 12.0f)
		{
			part_change_type(i,x,y,PT_FIRE);
			parts[i].life = rand()%50+120;
		}
	default:
		break;
	}
	return 0;
}

int graphics_DEFAULT(GRAPHICS_FUNC_ARGS)
{
	int t = cpart->type;
	if (ptypes[t].properties & PROP_RADIOACTIVE)
		*pixel_mode |= PMODE_GLOW;
	if (ptypes[t].properties & TYPE_LIQUID)
	{
		*pixel_mode |= PMODE_BLUR;
	}
	if (ptypes[t].properties & TYPE_GAS)
	{
		*pixel_mode &= ~PMODE;
		*pixel_mode |= FIRE_BLEND;
		*firer = *colr/2;
		*fireg = *colg/2;
		*fireb = *colb/2;
		*firea = 125;
		*pixel_mode |= DECO_FIRE;
	}
	return 1;
}

//very ugly function that somehow makes powered elements floodfill update and also maintain compatibility if not
int update_POWERED(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
#ifndef NOMOD
	if ((parts[i].type == PT_PPTI || parts[i].type == PT_PPTO) && parts[i].tmp2>0 && parts[i].tmp2!=10)
		parts[i].tmp2--;
	else if (parts[i].life>0 && parts[i].life!=10)
		parts[i].life--;
#else
	if (parts[i].life>0 && parts[i].life!=10)
			parts[i].life--;

#endif
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
#ifdef NOMOD
				if ((parts[i].type != PT_SWCH) || parts_avg(i,r>>8,PT_INSL)!=PT_INSL)
#else
				if ((parts[i].type != PT_SWCH && parts[i].type != PT_BUTN) || parts_avg(i,r>>8,PT_INSL)!=PT_INSL)
#endif
				{
					if ((r&0xFF)==parts[i].type && parts[i].type == PT_SWCH)
					{
						if (parts[i].life>=10&&parts[r>>8].life>0&&parts[r>>8].life<10)
							parts[i].life = 9;
						else if (parts[i].life==0&&parts[r>>8].life>=10)
						{
							//Set to other particle's life instead of 10, otherwise spark loops form when SWCH is sparked while turning on
							parts[i].life = parts[r>>8].life;
						}
					}
					if ((r&0xFF)==PT_SPRK && parts[r>>8].life>0 && (parts[r>>8].life<4 || parts[i].type == PT_SWCH))
					{
#ifndef NOMOD
						//Mod powered elements are always instantly activated
						if ((parts[i].type == PT_PPTI || parts[i].type == PT_PPTO))
						{
							if (parts[r>>8].life>2)
							{
								if (parts[r>>8].ctype==PT_PSCN && parts[i].tmp2 < 10)
								{
									PropertyValue tempValue;
									tempValue.Integer = 10;
									globalSim->FloodProp(x, y, Integer, tempValue, offsetof(particle, tmp2));
									tempValue.Integer = parts[i].flags|FLAG_SKIPMOVE;
									globalSim->FloodProp(x, y, Integer, tempValue, offsetof(particle, flags));
								}
								else if (parts[r>>8].ctype==PT_NSCN && parts[i].tmp2 >= 10)
								{
									PropertyValue tempValue;
									tempValue.Integer = 9;
									globalSim->FloodProp(x, y, Integer, tempValue, offsetof(particle, tmp2));
									tempValue.Integer = parts[i].flags|FLAG_SKIPMOVE;
									globalSim->FloodProp(x, y, Integer, tempValue, offsetof(particle, flags));
								}
							}
						}
						else if (parts[i].type == PT_ANIM)
						{
							if (parts[r>>8].life>2)
							{
								if (parts[r>>8].ctype==PT_PSCN && parts[i].life < 10)
								{
									PropertyValue tempValue;
									tempValue.Integer = 10;
									globalSim->FloodProp(x, y, Integer, tempValue, offsetof(particle, life));
									tempValue.Integer = parts[i].flags|FLAG_SKIPMOVE;
									globalSim->FloodProp(x, y, Integer, tempValue, offsetof(particle, flags));
								}
								else if (parts[r>>8].ctype==PT_NSCN && (parts[i].life >= 10 || parts[i].tmp != (int)(parts[i].temp-273.15) || parts[i].tmp2 > 1))
								{
									PropertyValue tempValue;
									tempValue.Integer = 9;
									globalSim->FloodProp(x, y, Integer, tempValue, offsetof(particle, life));
									tempValue.Integer = parts[i].flags|FLAG_SKIPMOVE;
									globalSim->FloodProp(x, y, Integer, tempValue, offsetof(particle, flags));
									tempValue.Integer = 0;
									globalSim->FloodProp(x, y, Integer, tempValue, offsetof(particle, tmp));
									globalSim->FloodProp(x, y, Integer, tempValue, offsetof(particle, tmp2));
								}
								else if (parts[r>>8].ctype==PT_METL)
								{
									if (parts[i].life == 10)
									{
										PropertyValue tempValue;
										tempValue.Integer = 9;
										globalSim->FloodProp(x, y, Integer, tempValue, offsetof(particle, life));
									}
									else if (parts[i].life == 0)
									{
										PropertyValue tempValue;
										tempValue.Integer = 14;
										globalSim->FloodProp(x, y, Integer, tempValue, offsetof(particle, life));
										return 0;
									}
								}
							}
						}
						//element is instantly activated (mod elements are always instantly activated)
						else if (sim->instantActivation || parts[i].type == PT_BUTN || parts[i].type == PT_PINV || parts[i].type == PT_PWHT)
						{
							if (parts[r >> 8].ctype == PT_PSCN && parts[i].life < 10)
							{
								PropertyValue tempValue;
								tempValue.Integer = 10;
								globalSim->FloodProp(x, y, Integer, tempValue, offsetof(particle, life));
								tempValue.Integer = parts[i].flags | FLAG_SKIPMOVE;
								globalSim->FloodProp(x, y, Integer, tempValue, offsetof(particle, flags));
							}
							else if (parts[r >> 8].ctype == PT_NSCN && parts[i].life >= 10)
							{
								PropertyValue tempValue;
								tempValue.Integer = 9;
								globalSim->FloodProp(x, y, Integer, tempValue, offsetof(particle, life));
								tempValue.Integer = parts[i].flags | FLAG_SKIPMOVE;
								globalSim->FloodProp(x, y, Integer, tempValue, offsetof(particle, flags));
							}
							else if ((parts[i].type == PT_SWCH || parts[i].type == PT_BUTN) && parts[r >> 8].ctype != PT_PSCN && parts[r >> 8].ctype != PT_NSCN && !(parts[r >> 8].ctype == PT_INWR && parts[r >> 8].tmp == 1) && parts[i].life == 10)
							{
								sim->spark_conductive(i, x, y);
								return 1;
							}
						}
#else
						if (false) { }
#endif
						else
						{
							if ((parts[i].type == PT_PUMP || parts[i].type == PT_GPMP || parts[i].type == PT_HSWC || parts[i].type == PT_PBCN))
								continue;
							if (parts[i].type != PT_SWCH && parts[r >> 8].ctype == PT_PSCN && parts[i].life < 10)
								parts[i].life = 10;
							else if (parts[i].type != PT_SWCH && parts[r >> 8].ctype == PT_NSCN)
								parts[i].life = 9;
							else if (parts[i].type == PT_SWCH && parts[r >> 8].ctype != PT_PSCN && parts[r >> 8].ctype != PT_NSCN && !(parts[r >> 8].ctype == PT_INWR && parts[r >> 8].tmp == 1) && parts[i].life == 10)
							{
								sim->spark_conductive(i, x, y);
								return 1;
							}
						}
					}
					if ((r&0xFF)==parts[i].type && parts[i].type != PT_SWCH)
					{
						if (parts[i].life==10&&parts[r>>8].life>0&&parts[r>>8].life<10)
							parts[i].life = 9;
						else if (parts[i].life==0&&parts[r>>8].life==10)
							parts[i].life = 10;
					}
				}
			}
	return 0;
}

