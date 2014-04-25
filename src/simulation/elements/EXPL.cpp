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

int EXPL_update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!(r&0xFF))
					continue;
				if (!(ptypes[r&0xFF].properties&PROP_INDESTRUCTIBLE) && (r&0xFF) != PT_EMBR) {
					parts[r>>8].flags |= FLAG_EXPLODE;
				}
			}
	return 0;
}

int EXPL_graphics(GRAPHICS_FUNC_ARGS)
{
	*pixel_mode |= PMODE_FLARE;
	return 0;
}

void EXPL_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_EXPL";
	elem->Name = "EXPL";
	elem->Colour = COLPACK(0xFEA713);
	elem->MenuVisible = 0;
	elem->MenuSection = SC_EXPLOSIVE;
	elem->Enabled = 0;

	elem->Advection = 0.6f;
	elem->AirDrag = 0.10f * CFDS;
	elem->AirLoss = 0.96f;
	elem->Loss = 0.95f;
	elem->Collision = 0.0f;
	elem->Gravity = 0.1f;
	elem->Diffusion = 0.00f;
	elem->HotAir = 0.000f	* CFDS;
	elem->Falldown = 1;

	elem->Flammable = 0;
	elem->Explosive = 0;
	elem->Meltable = 0;
	elem->Hardness = 0;

	elem->Weight = 99;

	elem->DefaultProperties.temp = R_TEMP -2.0f + 273.15f;
	elem->HeatConduct = 29;
	elem->Latent = 0;
	elem->Description = "Explosion, causes everything it touches to explode.";

	elem->State = ST_SOLID;
	elem->Properties = TYPE_PART|PROP_SPARKSETTLE|PROP_INDESTRUCTIBLE;

	elem->LowPressureTransitionThreshold = IPL;
	elem->LowPressureTransitionElement = NT;
	elem->HighPressureTransitionThreshold = IPH;
	elem->HighPressureTransitionElement = NT;
	elem->LowTemperatureTransitionThreshold = ITL;
	elem->LowTemperatureTransitionElement = NT;
	elem->HighTemperatureTransitionThreshold = ITH;
	elem->HighTemperatureTransitionElement = NT;

	elem->Update = &EXPL_update;
	elem->Graphics = &EXPL_graphics;
	elem->Init = &EXPL_init_element;
}
