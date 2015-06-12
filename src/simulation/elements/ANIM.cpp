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
#include "interface.h"

int ANIM_update(UPDATE_FUNC_ARGS)
{
	if (!parts[i].animations)
	{
		kill_part(i);
		return 1;
	}
	if (parts[i].life == 10)
	{
		parts[i].tmp--;
		if (parts[i].tmp <= 0)
		{
			parts[i].tmp = (int)(parts[i].temp-273.15);
			parts[i].tmp2++;
		}
	}
	if (parts[i].tmp2 > parts[i].ctype)
	{
		parts[i].tmp2 = 0;
	}
	parts[i].dcolour = parts[i].animations[parts[i].tmp2];
	return 0;
}

int ANIM_graphics(GRAPHICS_FUNC_ARGS)
{
	//invalid ANIM
	if (!cpart->animations || cpart->tmp2 < 0 || cpart->tmp2 >= sim->maxFrames)
		return 0;

	//if decorations are even set (black deco has alpha set)
	if (cpart->animations[cpart->tmp2])
	{
		*cola = COLA(cpart->animations[cpart->tmp2]);
		*colr = COLR(cpart->animations[cpart->tmp2]);
		*colg = COLG(cpart->animations[cpart->tmp2]);
		*colb = COLB(cpart->animations[cpart->tmp2]);
	}

	if (cpart->life < 10)
	{
		*colr = (int)(*colr / (5.5f - cpart->life / 2.0f));
		*colg = (int)(*colg / (5.5f - cpart->life / 2.0f));
		*colb = (int)(*colb / (5.5f - cpart->life / 2.0f));
	}

	*pixel_mode |= NO_DECO;
	return 0;
}

void ANIM_create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].animations = (ARGBColour*)calloc(sim->maxFrames, sizeof(ARGBColour));
	if (!sim->parts[i].animations)
		return;
	memset(sim->parts[i].animations, 0, sizeof(*sim->parts[i].animations));
}

void ANIM_ChangeType(ELEMENT_CHANGETYPE_FUNC_ARGS)
{
	if (to != PT_ANIM && parts[i].animations)
    {
            free(parts[i].animations);
            parts[i].animations = NULL;
    }
}

void ANIM_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_ANIM";
	elem->Name = "ANIM";
	elem->Colour = COLPACK(0x505050);
	elem->MenuVisible = 1;
	elem->MenuSection = SC_POWERED;
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
	elem->DefaultProperties.life = 10;
	elem->DefaultProperties.ctype = 0;
	elem->DefaultProperties.tmp = 1;
	elem->HeatConduct = 0;
	elem->Latent = 0;
	elem->Description = "Animated Liquid Crystal. Can show multiple frames, use left/right in the deco editor.";

	elem->State = ST_SOLID;
	elem->Properties = TYPE_SOLID|PROP_POWERED;

	elem->LowPressureTransitionThreshold = IPL;
	elem->LowPressureTransitionElement = NT;
	elem->HighPressureTransitionThreshold = IPH;
	elem->HighPressureTransitionElement = NT;
	elem->LowTemperatureTransitionThreshold = ITL;
	elem->LowTemperatureTransitionElement = NT;
	elem->HighTemperatureTransitionThreshold = ITH;
	elem->HighTemperatureTransitionElement = NT;

	elem->Update = &ANIM_update;
	elem->Graphics = &ANIM_graphics;
	elem->Func_Create = &ANIM_create;
	elem->Func_ChangeType = &ANIM_ChangeType;
	elem->Init = &ANIM_init_element;
}
