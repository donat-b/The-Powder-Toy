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

#define PFLAG_NORMALSPEED 0x00010000

// parts[].tmp flags
// trigger flags to be processed this frame (trigger flags for next frame are shifted 3 bits to the left):
#define PPIP_TMPFLAG_TRIGGER_ON 0x10000000
#define PPIP_TMPFLAG_TRIGGER_OFF 0x08000000
#define PPIP_TMPFLAG_TRIGGER_REVERSE 0x04000000
#define PPIP_TMPFLAG_TRIGGERS 0x1C000000
// current status of the pipe
#define PPIP_TMPFLAG_PAUSED 0x02000000
#define PPIP_TMPFLAG_REVERSED 0x01000000
// 0x000000FF element
// 0x00000100 is single pixel pipe
// 0x00000200 will transfer like a single pixel pipe when in forward mode
// 0x00001C00 forward single pixel pipe direction
// 0x00002000 will transfer like a single pixel pipe when in reverse mode
// 0x0001C000 reverse single pixel pipe direction

const signed char pos_1_rx[] = {-1,-1,-1, 0, 0, 1, 1, 1};
const signed char pos_1_ry[] = {-1, 0, 1,-1, 1,-1, 0, 1};

int ppip_changed = 0;

void PPIP_flood_trigger(int x, int y, int sparkedBy)
{
	int coord_stack_limit = XRES*YRES;
	unsigned short (*coord_stack)[2];
	int coord_stack_size = 0;
	int x1, x2;

	// Separate flags for on and off in case PPIP is sparked by PSCN and NSCN on the same frame
	// - then PSCN can override NSCN and behaviour is not dependent on particle order
	int prop = 0;
	if (sparkedBy==PT_PSCN) prop = PPIP_TMPFLAG_TRIGGER_ON << 3;
	else if (sparkedBy==PT_NSCN) prop = PPIP_TMPFLAG_TRIGGER_OFF << 3;
	else if (sparkedBy==PT_INST) prop = PPIP_TMPFLAG_TRIGGER_REVERSE << 3;

	if (prop==0 || (pmap[y][x]&0xFF)!=PT_PPIP || (parts[pmap[y][x]>>8].tmp & prop))
		return;

	coord_stack = (unsigned short(*)[2])malloc(sizeof(unsigned short)*2*coord_stack_limit);
	coord_stack[coord_stack_size][0] = x;
	coord_stack[coord_stack_size][1] = y;
	coord_stack_size++;

	do
	{
		coord_stack_size--;
		x = coord_stack[coord_stack_size][0];
		y = coord_stack[coord_stack_size][1];
		x1 = x2 = x;
		// go left as far as possible
		while (x1>=CELL)
		{
			if ((pmap[y][x1-1]&0xFF)!=PT_PPIP)
			{
				break;
			}
			x1--;
		}
		// go right as far as possible
		while (x2<XRES-CELL)
		{
			if ((pmap[y][x2+1]&0xFF)!=PT_PPIP)
			{
				break;
			}
			x2++;
		}
		// fill span
		for (x=x1; x<=x2; x++)
		{
			if (!(parts[pmap[y][x]>>8].tmp & prop))
				ppip_changed = 1;
			parts[pmap[y][x]>>8].tmp |= prop;
		}

		// add adjacent pixels to stack
		// +-1 to x limits to include diagonally adjacent pixels
		// Don't need to check x bounds here, because already limited to [CELL, XRES-CELL]
		if (y>=CELL+1)
			for (x=x1-1; x<=x2+1; x++)
				if ((pmap[y-1][x]&0xFF)==PT_PPIP && !(parts[pmap[y-1][x]>>8].tmp & prop))
				{
					coord_stack[coord_stack_size][0] = x;
					coord_stack[coord_stack_size][1] = y-1;
					coord_stack_size++;
					if (coord_stack_size>=coord_stack_limit)
					{
						free(coord_stack);
						return;
					}
				}
		if (y<YRES-CELL-1)
			for (x=x1-1; x<=x2+1; x++)
				if ((pmap[y+1][x]&0xFF)==PT_PPIP && !(parts[pmap[y+1][x]>>8].tmp & prop))
				{
					coord_stack[coord_stack_size][0] = x;
					coord_stack[coord_stack_size][1] = y+1;
					coord_stack_size++;
					if (coord_stack_size>=coord_stack_limit)
					{
						free(coord_stack);
						return;
					}
				}
	} while (coord_stack_size>0);
	free(coord_stack);
}

void PIPE_transfer_pipe_to_part(particle *pipe, particle *part)
{
	part->type = (pipe->tmp & 0xFF);
	part->temp = pipe->temp;
	part->life = pipe->tmp2;
	part->tmp = pipe->pavg[0];
	part->ctype = pipe->pavg[1];
	pipe->tmp &= ~0xFF;

	if (!ptypes[part->type].properties & TYPE_ENERGY)
	{
		part->vx = 0.0f;
		part->vy = 0.0f;
	}
	else if (part->type == PT_PHOT && part->ctype == 0x40000000)
		part->ctype = 0x3FFFFFFF;
	part->tmp2 = 0;
	part->flags = 0;
	part->dcolour = 0;
}

void PIPE_transfer_part_to_pipe(particle *part, particle *pipe)
{
	pipe->tmp = (pipe->tmp&~0xFF) | part->type;
	pipe->temp = part->temp;
	pipe->tmp2 = part->life;
	pipe->pavg[0] = part->tmp;
	pipe->pavg[1] = part->ctype;
}

void PIPE_transfer_pipe_to_pipe(particle *src, particle *dest)
{
	dest->tmp = (dest->tmp&~0xFF) | (src->tmp&0xFF);
	dest->temp = src->temp;
	dest->tmp2 = src->tmp2;
	dest->pavg[0] = src->pavg[0];
	dest->pavg[1] = src->pavg[1];
	src->tmp &= ~0xFF;
}

void pushParticle(int i, int count, int original)
{
	int rndstore, rnd, rx, ry, r, x, y, np, q, notctype=(((parts[i].ctype)%3)+2);
	if ((parts[i].tmp&0xFF) == 0 || count >= 2)//don't push if there is nothing there, max speed of 2 per frame
		return;
	x = (int)(parts[i].x+0.5f);
	y = (int)(parts[i].y+0.5f);
	if( !(parts[i].tmp&0x200) )
	{ 
		//normal random push
		rndstore = rand();
		// RAND_MAX is at least 32767 on all platforms i.e. pow(8,5)-1
		// so can go 5 cycles without regenerating rndstore
		for (q=0; q<3; q++)//try to push 3 times
		{
			rnd = rndstore&7;
			rndstore = rndstore>>3;
			rx = pos_1_rx[rnd];
			ry = pos_1_ry[rnd];
			if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES)
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				else if (((r&0xFF)==PT_PIPE || (r&0xFF) == PT_PPIP) && parts[r>>8].ctype!=notctype && (parts[r>>8].tmp&0xFF)==0)
				{
					PIPE_transfer_pipe_to_pipe(parts+i, parts+(r>>8));
					if (r>>8 > original)
						parts[r>>8].flags |= PFLAG_NORMALSPEED;//skip particle push, normalizes speed
					count++;
					pushParticle(r>>8,count,original);
				}
				else if ((r&0xFF) == PT_PRTI) //Pass particles into PRTI for a pipe speed increase
				{
					int nnx;
					for (nnx=0; nnx<80; nnx++)
						if (!portalp[parts[r>>8].tmp][count][nnx].type)
						{
							PIPE_transfer_pipe_to_part(parts+i, &(portalp[parts[r>>8].tmp][count][nnx]));
							count++;
							break;
						}
				}
			}
		}
	}
	else //predefined 1 pixel thick pipe movement
	{
		int coords = 7 - ((parts[i].tmp>>10)&7);
		r = pmap[y+ pos_1_ry[coords]][x+ pos_1_rx[coords]];
		if (((r&0xFF)==PT_PIPE || (r&0xFF) == PT_PPIP) && parts[r>>8].ctype!=notctype && (parts[r>>8].tmp&0xFF)==0)
		{
			PIPE_transfer_pipe_to_pipe(parts+i, parts+(r>>8));
			if (r>>8 > original)
				parts[r>>8].flags |= PFLAG_NORMALSPEED;//skip particle push, normalizes speed
			count++;
			pushParticle(r>>8,count,original);
		}
		else if ((r&0xFF) == PT_PRTI) //Pass particles into PRTI for a pipe speed increase
		{
			int nnx;
			for (nnx=0; nnx<80; nnx++)
				if (!portalp[parts[r>>8].tmp][count][nnx].type)
				{
					PIPE_transfer_pipe_to_part(parts+i, &(portalp[parts[r>>8].tmp][count][nnx]));
					count++;
					break;
				}
		}
		else if ((r&0xFF) == PT_NONE) //Move particles out of pipe automatically, much faster at ends
		{
			rx = pos_1_rx[coords];
			ry = pos_1_ry[coords];
			np = create_part(-1,x+rx,y+ry,parts[i].tmp&0xFF);
			if (np!=-1)
			{
				PIPE_transfer_pipe_to_part(parts+i, parts+np);
			}
		}
	}
	return;
}

int TRON_update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, np;
	if (parts[i].tmp&TRON_WAIT)
	{
		parts[i].tmp &= ~TRON_WAIT;
		return 0;
	}
	if (parts[i].tmp&TRON_HEAD)
	{
		int firstdircheck = 0,seconddir,seconddircheck = 0,lastdir,lastdircheck = 0;
		int direction = (parts[i].tmp>>5 & 0x3);
		int originaldir = direction;

		//random turn
		int random = rand()%340;
		if ((random==1 || random==3) && !(parts[i].tmp & TRON_NORANDOM))
		{
			//randomly turn left(3) or right(1)
			direction = (direction + random)%4;
		}
		
		//check in front
		//do sight check
		firstdircheck = trymovetron(x,y,direction,i,parts[i].tmp2);
		if (firstdircheck < parts[i].tmp2)
		{
			if (parts[i].tmp & TRON_NORANDOM)
			{
				seconddir = (direction + 1)%4;
				lastdir = (direction + 3)%4;
			}
			else if (originaldir != direction) //if we just tried a random turn, don't pick random again
			{
				seconddir = originaldir;
				lastdir = (direction + 2)%4;
			}
			else
			{
				seconddir = (direction + ((rand()%2)*2)+1)% 4;
				lastdir = (seconddir + 2)%4;
			}
			seconddircheck = trymovetron(x,y,seconddir,i,parts[i].tmp2);
			lastdircheck = trymovetron(x,y,lastdir,i,parts[i].tmp2);
		}
		//find the best move
		if (seconddircheck > firstdircheck)
			direction = seconddir;
		if (lastdircheck > seconddircheck && lastdircheck > firstdircheck)
			direction = lastdir;
		//now try making new head, even if it fails
		if (new_tronhead(x + tron_rx[direction],y + tron_ry[direction],i,direction) == -1)
		{
			//ohgod crash
			parts[i].tmp |= TRON_DEATH;
			//trigger tail death for TRON_NODIE, or is that mode even needed? just set a high tail length(but it still won't start dying when it crashes)
		}

		//set own life and clear .tmp (it dies if it can't move anyway)
		parts[i].life = parts[i].tmp2;
		parts[i].tmp &= parts[i].tmp&0xF818;
	}
	else // fade tail deco, or prevent tail from dying
	{
		if (parts[i].tmp&TRON_NODIE)
			parts[i].life++;
		//parts[i].dcolour =  clamp_flt((float)parts[i].life/(float)parts[i].tmp2,0,1.0f) << 24 |  parts[i].dcolour&0x00FFFFFF;
	}
	return 0;
}

int TRON_graphics(GRAPHICS_FUNC_ARGS)
{
	unsigned int col = tron_colours[(cpart->tmp&0xF800)>>11];
	if(cpart->tmp & TRON_HEAD)
		*pixel_mode |= PMODE_GLOW;
	*colr = (col & 0xFF0000)>>16;
	*colg = (col & 0x00FF00)>>8;
	*colb = (col & 0x0000FF);
	if(cpart->tmp & TRON_DEATH)
	{
		*pixel_mode |= FIRE_ADD | PMODE_FLARE;
		*firer = *colr;
		*fireg = *colg;
		*fireb = *colb;
		*firea = 255;
	}
	if(cpart->life < cpart->tmp2 && !(cpart->tmp & TRON_HEAD))
	{
		*pixel_mode |= PMODE_BLEND;
		*pixel_mode &= ~PMODE_FLAT;
		*cola = (int)((((float)cpart->life)/((float)cpart->tmp2))*255.0f);
	}
	return 0;
}

void TRON_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_TRON";
	elem->Name = "TRON";
	elem->Colour = COLPACK(0xA9FF00);
	elem->MenuVisible = 1;
	elem->MenuSection = SC_SPECIAL;
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

	elem->CreationTemperature = 0.0f;
	elem->HeatConduct = 40;
	elem->Latent = 0;
	elem->Description = "Smart particles, Travels in straight lines and avoids obstacles. Grows with time.";

	elem->State = ST_NONE;
	elem->Properties = TYPE_SOLID|PROP_LIFE_DEC|PROP_LIFE_KILL;

	elem->LowPressureTransitionThreshold = IPL;
	elem->LowPressureTransitionElement = NT;
	elem->HighPressureTransitionThreshold = IPH;
	elem->HighPressureTransitionElement = NT;
	elem->LowTemperatureTransitionThreshold = ITL;
	elem->LowTemperatureTransitionElement = NT;
	elem->HighTemperatureTransitionThreshold = ITH;
	elem->HighTemperatureTransitionElement = NT;

	elem->Update = &TRON_update;
	elem->Graphics = &TRON_graphics;
}
