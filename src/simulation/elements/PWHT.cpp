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

int PWHT_flood(UPDATE_FUNC_ARGS)
{
	int format = parts[i].tmp2;
	size_t propoffset;

	if (parts[i].ctype == 0)
		propoffset = offsetof(particle, type);
	else if (parts[i].ctype == 1)
		propoffset = offsetof(particle, life);
	else if (parts[i].ctype == 2)
		propoffset = offsetof(particle, ctype);
	else if (parts[i].ctype == 3)
		propoffset = offsetof(particle, temp);
	else if (parts[i].ctype == 4)
		propoffset = offsetof(particle, tmp);
	else if (parts[i].ctype == 5)
		propoffset = offsetof(particle, tmp2);
	else if (parts[i].ctype == 6)
		propoffset = offsetof(particle, vy);
	else if (parts[i].ctype == 7)
		propoffset = offsetof(particle, vx);
	else if (parts[i].ctype == 8)
		propoffset = offsetof(particle, x);
	else if (parts[i].ctype == 9)
		propoffset = offsetof(particle, y);
	else if (parts[i].ctype == 10)
		propoffset = offsetof(particle, dcolour);
	else if (parts[i].ctype == 11)
		propoffset = offsetof(particle, flags);
	else
	{
		parts[i].ctype = 0;
		parts[i].tmp2 = 0;
	}
	
	if (format == 2)
	{
		PropertyValue valuef;
		valuef.Float = parts[i].temp;
		return globalSim->FloodProp(x, y - 1, Float, valuef, propoffset);
	}
	else if (format == 3)
	{
		PropertyValue valueui;
		valueui.UInteger = parts[i].dcolour;
		return globalSim->FloodProp(x, y - 1, UInteger, valueui, propoffset);
	}
	else
	{
		PropertyValue valuei;
		valuei.Integer = parts[i].tmp;
		return globalSim->FloodProp(x, y - 1, Integer, valuei, propoffset);
	}
}

int PWHT_update(UPDATE_FUNC_ARGS)
{
	int r = pmap[y-1][x];
	float temp = parts[i].temp;
	if (parts[i].life < 10)
		return 0;
	if ((r&0xFF) == PT_PWHT)
	{
		for (int rx=-1; rx<2; rx++)
			for (int ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK)
				{
					if ((pmap[y+ry][x+rx]&0xFF) == PT_PWHT)
						kill_part(pmap[y+ry][x+rx]>>8);
				}
		return 1;
	}
	else if (r&0xFF)
	{
		if (!parts[i].ctype && !parts[i].tmp2)
		{
			PropertyValue valuef;
			valuef.Float = parts[i].temp;
			globalSim->FloodProp(x, y - 1, Float, valuef, offsetof(particle, temp));
		}
		else
		{
			PWHT_flood(UPDATE_FUNC_SUBCALL_ARGS);
		}
	}
	return 0;
}

int PWHT_graphics(GRAPHICS_FUNC_ARGS)
{
	int lifemod = ((cpart->life>10?10:cpart->life)*19);
	*colb += lifemod;
	return 0;
}
void PWHT_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_PWHT";
	elem->Name = "PWHT";
	elem->Colour = COLPACK(0x381D7C);
	elem->MenuVisible = 1;
	elem->MenuSection = SC_POWERED;
	elem->Enabled = 1;

	elem->Advection = 0.0f;
	elem->AirDrag = 0.00f * CFDS;
	elem->AirLoss = 0.97f;
	elem->Loss = 0.00f;
	elem->Collision = 0.0f;
	elem->Gravity = 0.0f;
	elem->Diffusion = 0.00f;
	elem->HotAir = 0.000f	* CFDS;
	elem->Falldown = 0;

	elem->Flammable = 0;
	elem->Explosive = 0;
	elem->Meltable = 0;
	elem->Hardness = 0;

	elem->Weight = 100;

	elem->DefaultProperties.temp = R_TEMP + 273.15f;
	elem->HeatConduct = 0;
	elem->Latent = 0;
	elem->Description = "Powered Heater. Flood fill heats particles to it's temp. Use only one.";

	elem->State = ST_NONE;
	elem->Properties = TYPE_SOLID|PROP_POWERED;

	elem->LowPressureTransitionThreshold = IPL;
	elem->LowPressureTransitionElement = NT;
	elem->HighPressureTransitionThreshold = IPH;
	elem->HighPressureTransitionElement = NT;
	elem->LowTemperatureTransitionThreshold = ITL;
	elem->LowTemperatureTransitionElement = NT;
	elem->HighTemperatureTransitionThreshold = ITH;
	elem->HighTemperatureTransitionElement = NT;

	elem->DefaultProperties.life = 10;

	elem->Update = &PWHT_update;
	elem->Graphics = &PWHT_graphics;
	elem->Init = &PWHT_init_element;
}
