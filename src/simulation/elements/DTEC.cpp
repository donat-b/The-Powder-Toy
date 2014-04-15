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

int DTEC_update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, rt, rd = parts[i].tmp2;
	if (rd > 25) parts[i].tmp2 = rd = 25;
	if (parts[i].life)
	{
		parts[i].life = 0;
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					rt = r&0xFF;
					if (parts_avg(i,r>>8,PT_INSL) != PT_INSL)
					{
						if ((ptypes[rt].properties&PROP_CONDUCTS) && !(rt==PT_WATR||rt==PT_SLTW||rt==PT_NTCT||rt==PT_PTCT||rt==PT_INWR) && parts[r>>8].life==0)
						{
							sim->spark_conductive(r>>8, x+rx, y+ry);
						}
					}
				}
	}
	bool setFilt = false;
	int photonWl = 0;
	for (rx=-rd; rx<rd+1; rx++)
		for (ry=-rd; ry<rd+1; ry++)
			if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					r = photons[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF) == parts[i].ctype && (parts[i].ctype != PT_LIFE || parts[i].tmp == parts[r>>8].ctype || !parts[i].tmp))
					parts[i].life = 1;
				if ((r&0xFF) == PT_PHOT || ((r&0xFF) == PT_BRAY && parts[r>>8].tmp!=2))
				{
					setFilt = true;
					photonWl = parts[r>>8].ctype;
				}
			}
	if (setFilt)
	{
		int nx, ny;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					nx = x+rx;
					ny = y+ry;
					while ((r&0xFF)==PT_FILT)
					{
						parts[r>>8].ctype = photonWl;
						nx += rx;
						ny += ry;
						if (nx<0 || ny<0 || nx>=XRES || ny>=YRES)
							break;
						r = pmap[ny][nx];
					}
				}
	}
	return 0;
}

int DTEC_graphics(GRAPHICS_FUNC_ARGS)
{
	if (cpart->life)
		*pixel_mode |=  PMODE_GLOW;
	return 0;
}

void DTEC_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_DTEC";
	elem->Name = "DTEC";
	elem->Colour = COLPACK(0xFD9D18);
	elem->MenuVisible = 1;
	elem->MenuSection = SC_SENSOR;
	elem->Enabled = 1;

	elem->Advection = 0.0f;
	elem->AirDrag = 0.00f * CFDS;
	elem->AirLoss = 0.96f;
	elem->Loss = 0.00f;
	elem->Collision = 0.0f;
	elem->Gravity = 0.0f;
	elem->Diffusion = 0.00f;
	elem->PressureAdd_NoAmbHeat = 0.000f  * CFDS;
	elem->Falldown = 0;

	elem->Flammable = 0;
	elem->Explosive = 0;
	elem->Meltable = 0;
	elem->Hardness = 1;

	elem->Weight = 100;

	elem->DefaultProperties.temp = R_TEMP+273.15f;
	elem->HeatConduct = 0;
	elem->Latent = 0;
	elem->Description = "Detector, creates a spark when something with its ctype is nearby.";

	elem->State = ST_SOLID;
	elem->Properties = TYPE_SOLID;

	elem->LowPressureTransitionThreshold = IPL;
	elem->LowPressureTransitionElement = NT;
	elem->HighPressureTransitionThreshold = IPH;
	elem->HighPressureTransitionElement = NT;
	elem->LowTemperatureTransitionThreshold = ITL;
	elem->LowTemperatureTransitionElement = NT;
	elem->HighTemperatureTransitionThreshold = ITH;
	elem->HighTemperatureTransitionElement = NT;

	elem->DefaultProperties.tmp2 = 2;

	elem->Update = &DTEC_update;
	elem->Graphics = &DTEC_graphics;
	elem->Init = &DTEC_init_element;
}
