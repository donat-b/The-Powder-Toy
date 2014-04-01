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

const char* messages[16] = {
	"cmd:1: attempt to index global 'parts' (a nil value)",
	"cmd:1: attempt to index global 'potato' (a troll value)",
	"cmd:1: unexpected symbol near 'april'",
	"cmd:1: no-file: No such file or directory",
	"cmd:1: unable to open 'powder.pref'",
	"cmd:1: malformed number near '1april'",
	"Particle type not recognized",
	"cmd:1: i'm a potato",
	"cmd:1: assertion failed!",
	"cmd:1: attempt to call field 'insert_rootkit' (a nil value)",
	"cmd:1: attempt to call field 'create_part' (a nil value)",
	"cmd:1: attempt to call field 'passwordLogger' (a nil value)",
	"Invalid Page Fault in module Kernel32.dll",
	"A problem has been detected and windows must be shut down to prevent damage to your computer",
	"Only premium members may use this element, upgrade today",
	"An unidentified program wants access to your computer"
};
int CMD_graphics(GRAPHICS_FUNC_ARGS)
{
	if (cpart->life > 0)
	{
		if (cpart->ctype >= 0 && cpart->ctype < 16)
		{
			int alpha = std::min(cpart->life*5, 255);
			drawtext(vid_buf, cpart->tmp, cpart->tmp2, messages[cpart->ctype], 255, 255, 255, alpha);
		}
		cpart->life--;
	}
	return 0;
}

void CMD_create(ELEMENT_CREATE_FUNC_ARGS)
{
	parts[i].ctype = rand()%16;
	parts[i].tmp = rand()%XRES-40;
	parts[i].tmp2 = rand()%YRES;
}

void CMD_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_CMD";
	elem->Name = "CMD";
	elem->Colour = COLPACK(0x9C939C);
	elem->MenuVisible = 1;
	elem->MenuSection = SC_NUCLEAR;
	elem->Enabled = 1;

	elem->Advection = 0.0f;
	elem->AirDrag = 0.00f * CFDS;
	elem->AirLoss = 0.90f;
	elem->Loss = 0.00f;
	elem->Collision = 0.0f;
	elem->Gravity = 0.0f;
	elem->Diffusion = 0.00f;
	elem->PressureAdd_NoAmbHeat = 0.000f	* CFDS;
	elem->Falldown = 0;

	elem->Flammable = 0;
	elem->Explosive = 0;
	elem->Meltable = 0;
	elem->Hardness = 0;

	elem->Weight = 100;

	elem->DefaultProperties.temp = R_TEMP+0.0f	+273.15f;
	elem->DefaultProperties.life = 60;
	elem->HeatConduct = 186;
	elem->Latent = 0;
	elem->Description = "Command element, runs console commands.";

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

	elem->Update = NULL;
	elem->Graphics = &CMD_graphics;
	elem->Func_Create = &CMD_create;
	elem->Init = &CMD_init_element;
}
