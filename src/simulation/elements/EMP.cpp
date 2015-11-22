/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "simulation/ElementsCommon.h"
#include "EMP.h"

int EMP_update(UPDATE_FUNC_ARGS)
{
	int r,rx,ry,t,ntype;
	if (parts[i].life)
		return 0;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_SPRK && parts[r>>8].life>0 && parts[r>>8].life<4)
				{
					goto ok;
				}
			}
	return 0;
ok:
	parts[i].life=220;
	((EMP_ElementDataContainer*)sim->elementData[PT_EMP])->Activate();
	for (r = 0; r <= globalSim->parts_lastActiveIndex; r++)
	{
		t=parts[r].type;
		rx=(int)parts[r].x;
		ry=(int)parts[r].y;
		if (t==PT_SPRK || (t==PT_SWCH && parts[r].life!=0 && parts[r].life!=10) || (t==PT_WIRE && parts[r].ctype>0))
		{
			int is_elec=0, n,nx,ny;
			if (parts[r].ctype==PT_PSCN || parts[r].ctype==PT_NSCN || parts[r].ctype==PT_PTCT || parts[r].ctype==PT_NTCT || parts[r].ctype==PT_INST || parts[r].ctype==PT_SWCH || t==PT_WIRE || t==PT_SWCH)
			{
				is_elec=1;
				if (!(rand()%100))
					parts[r].temp = restrict_flt(parts[r].temp+3000.0f, MIN_TEMP, MAX_TEMP);
				if (!(rand()%80))
					part_change_type(r, rx, ry, PT_BREL);
				else if (!(rand()%120))
					part_change_type(r, rx, ry, PT_NTCT);
			}
			for (nx=-2; nx<3; nx++)
				for (ny=-2; ny<3; ny++)
					if (rx+nx>=0 && ry+ny>=0 && rx+nx<XRES && ry+ny<YRES && (rx || ry))
					{
						n = pmap[ry+ny][rx+nx];
						if (!n)
							continue;
						ntype = n&0xFF;

						//Some elements should only be affected by wire/swch, or by a spark on inst/semiconductor
						//So not affected by spark on metl, watr etc
						if (is_elec)
						{
							switch (ntype)
							{
							case PT_METL:
								if (!(rand()%280))
									parts[n>>8].temp = restrict_flt(parts[n>>8].temp+3000.0f, MIN_TEMP, MAX_TEMP);
								if (!(rand()%300))
									part_change_type(n>>8, rx+nx, ry+ny, PT_BMTL);
								continue;
							case PT_BMTL:
								if (!(rand()%280))
									parts[n>>8].temp = restrict_flt(parts[n>>8].temp+3000.0f, MIN_TEMP, MAX_TEMP);
								if (!(rand()%160))
								{
									part_change_type(n>>8, rx+nx, ry+ny, PT_BRMT);
									parts[n>>8].temp = restrict_flt(parts[n>>8].temp+1000.0f, MIN_TEMP, MAX_TEMP);
								}
								continue;
							case PT_WIFI:
								if (!(rand()%8))
								{
									//Randomize channel
									parts[n>>8].temp = (float)(rand()%MAX_TEMP);
								}
								if (!(rand()%16))
								{
									sim->part_create(n>>8, rx+nx, ry+ny, PT_BREL);
									parts[n>>8].temp = restrict_flt(parts[n>>8].temp+1000.0f, MIN_TEMP, MAX_TEMP);
								}
								continue;
							default:
								break;
							}
						}
						switch (ntype)
						{
						case PT_SWCH:
							if (!(rand()%100))
								part_change_type(n>>8, rx+nx, ry+ny, PT_BREL);
							if (!(rand()%100))
								parts[n>>8].temp = restrict_flt(parts[n>>8].temp+2000.0f, MIN_TEMP, MAX_TEMP);
							break;
						case PT_ARAY:
							if (!(rand()%60))
							{
								sim->part_create(n>>8, rx+nx, ry+ny, PT_BREL);
								parts[n>>8].temp = restrict_flt(parts[n>>8].temp+1000.0f, MIN_TEMP, MAX_TEMP);
							}
							break;
						case PT_DLAY:
							if (!(rand()%70))
							{
								//Randomise delay
								parts[n>>8].temp = (rand()%256) + 273.15f;
							}
							break;
						default:
							break;
						}
					}
		}
	}
	return 0;
}

int EMP_graphics(GRAPHICS_FUNC_ARGS)
{
	if(cpart->life)
	{
		*colr = (int)(cpart->life*1.5);
		*colg = (int)(cpart->life*1.5);
		*colb = 200-(cpart->life);
	}
	return 0;
}

void EMP_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_EMP";
	elem->Name = "EMP";
	elem->Colour = COLPACK(0x66AAFF);
	elem->MenuVisible = 1;
	elem->MenuSection = SC_ELEC;
	elem->Enabled = 1;

	elem->Advection = 0.0f;
	elem->AirDrag = 0.00f * CFDS;
	elem->AirLoss = 0.90f;
	elem->Loss = 0.00f;
	elem->Collision = 0.0f;
	elem->Gravity = 0.0f;
	elem->Diffusion = 0.0f;
	elem->HotAir = 0.0f	* CFDS;
	elem->Falldown = 0;

	elem->Flammable = 0;
	elem->Explosive = 0;
	elem->Meltable = 0;
	elem->Hardness = 3;

	elem->Weight = 100;

	elem->DefaultProperties.temp = R_TEMP+0.0f	+273.15f;
	elem->HeatConduct = 121;
	elem->Latent = 0;
	elem->Description = "Electromagnetic pulse. Breaks activated electronics.";

	elem->State = ST_SOLID;
	elem->Properties = TYPE_SOLID|PROP_LIFE_DEC;

	elem->LowPressureTransitionThreshold = IPL;
	elem->LowPressureTransitionElement = NT;
	elem->HighPressureTransitionThreshold = IPH;
	elem->HighPressureTransitionElement = NT;
	elem->LowTemperatureTransitionThreshold = ITL;
	elem->LowTemperatureTransitionElement = NT;
	elem->HighTemperatureTransitionThreshold = ITH;
	elem->HighTemperatureTransitionElement = NT;

	elem->Update = &EMP_update;
	elem->Graphics = &EMP_graphics;
	elem->Init = &EMP_init_element;

	if (sim->elementData[t])
	{
		delete sim->elementData[t];
	}
	sim->elementData[t] = new EMP_ElementDataContainer;
}
