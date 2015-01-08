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

int DRAY_update(UPDATE_FUNC_ARGS)
{
	int ctype = parts[i].ctype&0xFF, ctypeExtra = parts[i].ctype>>8, copyLength = parts[i].tmp, copySpaces = parts[i].tmp2;
	if (copySpaces < 0)
		copySpaces = parts[i].tmp2  = 0;
	if (copyLength < 0)
		copyLength = parts[i].tmp  = 0;
	else
		copySpaces++; //strange hack
	if (!parts[i].life) // only fire when life is 0, but nothing sets the life right now
	{
		for (int rx = -1; rx <= 1; rx++)
			for (int ry = -1; ry <= 1; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					int r = pmap[y+ry][x+rx];
					if ((r&0xFF) == PT_SPRK && parts[r>>8].life == 3) //spark found, start creating
					{
						bool overwrite = parts[r>>8].ctype == PT_PSCN;
						int partsRemaining = copyLength, xCopyTo, yCopyTo; //positions where the line will start being copied at

						if (parts[r>>8].ctype == PT_INWR && rx && ry) // INWR doesn't spark from diagonals
							continue;

						//figure out where the copying will start/end
						for (int xStep = rx*-1, yStep = ry*-1, xCurrent = x+xStep, yCurrent = y+yStep; ; xCurrent+=xStep, yCurrent+=yStep)
						{
							int rr = pmap[yCurrent][xCurrent];
							if ((!copyLength && (rr&0xFF) == ctype && (ctype != PT_LIFE || parts[rr>>8].ctype == ctypeExtra))
									|| !(--partsRemaining && sim->InBounds(xCurrent+xStep, yCurrent+yStep)))
							{
								copyLength -= partsRemaining;
								xCopyTo = xCurrent + xStep*copySpaces;
								yCopyTo = yCurrent + yStep*copySpaces;
								break;
							}
						}
						
						//now, actually copy the particles
						partsRemaining = copyLength + 1;
						for (int xStep = rx*-1, yStep = ry*-1, xCurrent = x+xStep, yCurrent = y+yStep; sim->InBounds(xCopyTo, yCopyTo) && --partsRemaining; xCurrent+=xStep, yCurrent+=yStep, xCopyTo+=xStep, yCopyTo+=yStep)
						{
							int type = pmap[yCurrent][xCurrent]&0xFF, p;
							if (overwrite)
								delete_part(xCopyTo, yCopyTo, 0);
							if (type == PT_SPRK)
								p = sim->part_create(-1, xCopyTo, yCopyTo, PT_METL);
							else
								p = sim->part_create(-1, xCopyTo, yCopyTo, type);
							if (p >= 0)
							{
								if (type == PT_SPRK)
									sim->part_change_type(p, xCopyTo, yCopyTo, PT_SPRK);
								parts[p] = parts[pmap[yCurrent][xCurrent]>>8];
								parts[p].x = (float)xCopyTo;
								parts[p].y = (float)yCopyTo;
								parts[p].animations = NULL;
							}
						}
					}
				}
	}
	return 0;
}

void DRAY_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_DRAY";
	elem->Name = "DRAY";
	elem->Colour = COLPACK(0xFFAA22);
	elem->MenuVisible = 1;
	elem->MenuSection = SC_ELEC;
	elem->Enabled = 1;

	elem->Advection = 0.0f;
	elem->AirDrag = 0.00f * CFDS;
	elem->AirLoss = 0.90f;
	elem->Loss = 0.00f;
	elem->Collision = 0.0f;
	elem->Gravity = 0.0f;
	elem->Diffusion = 0.00f;
	elem->HotAir = 0.000f	* CFDS;
	elem->Falldown = 0;

	elem->Flammable = 0;
	elem->Explosive = 0;
	elem->Meltable = 0;
	elem->Hardness = 1;

	elem->Weight = 100;

	elem->DefaultProperties.temp = R_TEMP + 273.15f;
	elem->HeatConduct = 0;
	elem->Latent = 0;
	elem->Description = "Duplicator ray. Replicates a line of particles in front of it.";

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

	elem->Update = &DRAY_update;
	elem->Graphics = NULL;
	elem->Init = &DRAY_init_element;
}

