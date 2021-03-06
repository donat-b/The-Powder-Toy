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

int FWRK_update(UPDATE_FUNC_ARGS)
{
	if (parts[i].life == 0 && ((surround_space && parts[i].temp > 400 && (9+parts[i].temp/40) > rand()%100000) || parts[i].ctype == PT_DUST))
	{
		float gx, gy, multiplier, gmax;
		int randTmp;
		get_gravity_field(x, y, ptypes[PT_FWRK].gravity, 1.0f, &gx, &gy);
		if (gx*gx+gy*gy < 0.001f)
		{
			float angle = (rand()%6284)*0.001f;//(in radians, between 0 and 2*pi)
			gx += sinf(angle)*ptypes[PT_FWRK].gravity*0.5f;
			gy += cosf(angle)*ptypes[PT_FWRK].gravity*0.5f;
		}
		gmax = std::max(fabsf(gx), fabsf(gy));
		if (sim->EvalMove(PT_FWRK, (int)(x-(gx/gmax)+0.5f), (int)(y-(gy/gmax)+0.5f)))
		{
			multiplier = 15.0f/sqrtf(gx*gx+gy*gy);

			//Some variation in speed parallel to gravity direction
			randTmp = (rand()%200)-100;
			gx += gx*randTmp*0.002f;
			gy += gy*randTmp*0.002f;
			//and a bit more variation in speed perpendicular to gravity direction
			randTmp = (rand()%200)-100;
			gx += -gy*randTmp*0.005f;
			gy += gx*randTmp*0.005f;

			parts[i].life=rand()%10+18;
			parts[i].ctype=0;
			parts[i].vx -= gx*multiplier;
			parts[i].vy -= gy*multiplier;
			parts[i].dcolour = parts[i].dcolour;
			return 0;
		}
	}
	if (parts[i].life<3 && parts[i].life>0)
	{
		int r = (rand()%245+11);
		int g = (rand()%245+11);
		int b = (rand()%245+11);
		int n;
		float angle, magnitude;
		unsigned col = (r<<16) | (g<<8) | b;
		for (n=0; n<40; n++)
		{
			int np = sim->part_create(-3, x, y, PT_EMBR);
			if (np>-1)
			{
				magnitude = ((rand()%60)+40)*0.05f;
				angle = (rand()%6284)*0.001f;//(in radians, between 0 and 2*pi)
				parts[np].vx = parts[i].vx*0.5f + cosf(angle)*magnitude;
				parts[np].vy = parts[i].vy*0.5f + sinf(angle)*magnitude;
				parts[np].ctype = col;
				parts[np].tmp = 1;
				parts[np].life = rand()%40+70;
				parts[np].temp = (rand()%500)+5750.0f;
				parts[np].dcolour = parts[i].dcolour;
			}
		}
		pv[y/CELL][x/CELL] += 8.0f;
		kill_part(i);
		return 1;
	}
	if (parts[i].life >= 45)
		parts[i].life=0;
	return 0;
}

void FWRK_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_FWRK";
	elem->Name = "FWRK";
	elem->Colour = COLPACK(0x666666);
	elem->MenuVisible = 1;
	elem->MenuSection = SC_EXPLOSIVE;
	elem->Enabled = 1;

	elem->Advection = 0.4f;
	elem->AirDrag = 0.01f * CFDS;
	elem->AirLoss = 0.99f;
	elem->Loss = 0.95f;
	elem->Collision = 0.0f;
	elem->Gravity = 0.4f;
	elem->Diffusion = 0.00f;
	elem->HotAir = 0.000f	* CFDS;
	elem->Falldown = 1;

	elem->Flammable = 0;
	elem->Explosive = 0;
	elem->Meltable = 0;
	elem->Hardness = 1;

	elem->Weight = 97;

	elem->DefaultProperties.temp = R_TEMP+0.0f	+273.15f;
	elem->HeatConduct = 100;
	elem->Latent = 0;
	elem->Description = "Original version of fireworks, activated by heat/neutrons.";

	elem->State = ST_SOLID;
	elem->Properties = TYPE_PART|PROP_LIFE_DEC;

	elem->LowPressureTransitionThreshold = IPL;
	elem->LowPressureTransitionElement = NT;
	elem->HighPressureTransitionThreshold = IPH;
	elem->HighPressureTransitionElement = NT;
	elem->LowTemperatureTransitionThreshold = ITL;
	elem->LowTemperatureTransitionElement = NT;
	elem->HighTemperatureTransitionThreshold = ITH;
	elem->HighTemperatureTransitionElement = NT;

	elem->Update = &FWRK_update;
	elem->Graphics = NULL;
	elem->Init = &FWRK_init_element;
}
