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

#include "SDLCompat.h"
#include "simulation/ElementsCommon.h"
#include "simulation/ToolNumbers.h"
#include "simulation/Tool.h"
#include "simulation/elements/FIGH.h"
#include "simulation/elements/PRTI.h"
#include "simulation/elements/STKM.h"

#define INBOND(x, y) ((x)>=0 && (y)>=0 && (x)<XRES && (y)<YRES)

const float dt = 0.9f; // Delta time in square
const float rocketBootsHeadEffect = 0.35f;
const float rocketBootsFeetEffect = 0.15f;
const float rocketBootsHeadEffectV = 0.3f; // stronger acceleration vertically, to counteract gravity
const float rocketBootsFeetEffectV = 0.45f;

void STKM_ElementDataContainer::Simulation_AfterUpdate(Simulation *sim)
{
	//Setting an element for the stick man
	if (sim->elementCount[PT_STKM] <= 0)
	{
		int sr = ((ElementTool*)activeTools[1])->GetID();
		if ((sr > 0 && sr < PT_NUM && sim->elements[sr].Enabled && sim->elements[sr].Falldown > 0) || sr == PT_NEUT || sr == PT_PHOT || sr == PT_LIGH)
			player.elem = sr;
		else
			player.elem = PT_DUST;
	}
	if (sim->elementCount[PT_STKM2] <= 0)
	{
		int sr = ((ElementTool*)activeTools[1])->GetID();
		if ((sr > 0 && sr < PT_NUM && sim->elements[sr].Enabled && sim->elements[sr].Falldown > 0) || sr == PT_NEUT || sr == PT_PHOT || sr == PT_LIGH)
			player2.elem = sr;
		else
			player2.elem = PT_DUST;
	}
}

void STKM_ElementDataContainer::NewStickman1(int i, int elem)
{
	InitLegs(&player, i);
	player.elem = (elem == OLD_SPC_AIR) ? SPC_AIR : elem;
	player.spwn = 1;
}

void STKM_ElementDataContainer::NewStickman2(int i, int elem)
{
	InitLegs(&player2, i);
	player2.elem = (elem == OLD_SPC_AIR) ? SPC_AIR : elem;
	player2.spwn = 1;
}

int STKM_ElementDataContainer::Run(Stickman *playerp, UPDATE_FUNC_ARGS)
{
	int t = parts[i].type;

	if ((parts[i].ctype>0 && parts[i].ctype<PT_NUM && ptypes[parts[i].ctype].enabled && ptypes[parts[i].ctype].falldown>0) || parts[i].ctype==SPC_AIR || parts[i].ctype == PT_NEUT || parts[i].ctype == PT_PHOT || parts[i].ctype == PT_LIGH)
		playerp->elem = parts[i].ctype;
	playerp->frames++;

	// Temperature handling
	if (parts[i].temp<243)
		parts[i].life -= 1;
	if ((parts[i].temp<309.6f) && (parts[i].temp>=243))
		parts[i].temp += 1;

	// If his HP is less than 0 or there is very big wind...
	if (parts[i].life < 1 || (pv[y/CELL][x/CELL] >= 4.5f && playerp->elem != SPC_AIR))
	{
		for (int r = -2; r <= 1; r++)
		{
			sim->part_create(-1, x+r, y-2, playerp->elem);
			sim->part_create(-1, x+r+1, y+2, playerp->elem);
			sim->part_create(-1, x-2, y+r+1, playerp->elem);
			sim->part_create(-1, x+2, y+r, playerp->elem);
		}
		sim->part_kill(i); // Kill him
		return 1;
	}

	// Follow gravity
	float gvx = 0.0f, gvy = 0.0f;
	switch (gravityMode)
	{
		default:
		case 0:
			gvy = 1;
			break;
		case 1:
			gvy = gvx = 0.0f;
			break;
		case 2:
		{
			float gravd;
			gravd = 0.01f - hypotf((parts[i].x - XCNTR), (parts[i].y - YCNTR));
			gvx = ((float)(parts[i].x - XCNTR) / gravd);
			gvy = ((float)(parts[i].y - YCNTR) / gravd);
		}
		break;
	}

	gvx += gravx[((int)parts[i].y/CELL)*(XRES/CELL)+((int)parts[i].x/CELL)];
	gvy += gravy[((int)parts[i].y/CELL)*(XRES/CELL)+((int)parts[i].x/CELL)];

	float rbx = gvx;
	float rby = gvy;
	bool rbLowGrav = false;
	float tmp = (fabsf(rbx) > fabsf(rby)) ? fabsf(rbx) : fabsf(rby);
	if (tmp < 0.001f)
	{
		rbLowGrav = true;
		rbx = -parts[i].vx;
		rby = -parts[i].vy;
		tmp = fabsf(rbx) > fabsf(rby)?fabsf(rbx):fabsf(rby);
	}
	if (tmp < 0.001f)
	{
		rbx = 0;
		rby = 1.0f;
		tmp = 1.0f;
	}
	tmp = 1.0f/sqrtf(rbx*rbx+rby*rby);
	rbx *= tmp; // scale to a unit vector
	rby *= tmp;

	parts[i].vx -= gvx*dt; //Head up!
	parts[i].vy -= gvy*dt;

	// Verlet integration
	float pp = 2*playerp->legs[0]-playerp->legs[2]+playerp->accs[0]*dt*dt;
	playerp->legs[2] = playerp->legs[0];
	playerp->legs[0] = pp;
	pp = 2*playerp->legs[1]-playerp->legs[3]+playerp->accs[1]*dt*dt;
	playerp->legs[3] = playerp->legs[1];
	playerp->legs[1] = pp;

	pp = 2*playerp->legs[4]-playerp->legs[6]+(playerp->accs[2]+gvx)*dt*dt;
	playerp->legs[6] = playerp->legs[4];
	playerp->legs[4] = pp;
	pp = 2*playerp->legs[5]-playerp->legs[7]+(playerp->accs[3]+gvy)*dt*dt;
	playerp->legs[7] = playerp->legs[5];
	playerp->legs[5] = pp;

	pp = 2*playerp->legs[8]-playerp->legs[10]+playerp->accs[4]*dt*dt;
	playerp->legs[10] = playerp->legs[8];
	playerp->legs[8] = pp;
	pp = 2*playerp->legs[9]-playerp->legs[11]+playerp->accs[5]*dt*dt;
	playerp->legs[11] = playerp->legs[9];
	playerp->legs[9] = pp;

	pp = 2*playerp->legs[12]-playerp->legs[14]+(playerp->accs[6]+gvx)*dt*dt;
	playerp->legs[14] = playerp->legs[12];
	playerp->legs[12] = pp;
	pp = 2*playerp->legs[13]-playerp->legs[15]+(playerp->accs[7]+gvy)*dt*dt;
	playerp->legs[15] = playerp->legs[13];
	playerp->legs[13] = pp;

	// Setting acceleration to 0
	playerp->accs[0] = 0;
	playerp->accs[1] = 0;

	playerp->accs[2] = 0;
	playerp->accs[3] = 0;

	playerp->accs[4] = 0;
	playerp->accs[5] = 0;

	playerp->accs[6] = 0;
	playerp->accs[7] = 0;

	float gx = (playerp->legs[4] + playerp->legs[12])/2 - gvy;
	float gy = (playerp->legs[5] + playerp->legs[13])/2 + gvx;
	float dl = pow(gx - playerp->legs[4], 2) + pow(gy - playerp->legs[5], 2);
	float dr = pow(gx - playerp->legs[12], 2) + pow(gy - playerp->legs[13], 2);
	
	// Go left
	if (((int)(playerp->comm)&0x01) == 0x01)
	{
		bool moved = false;
		if (dl > dr)
		{
			if (INBOND(playerp->legs[4], playerp->legs[5]) && !sim->EvalMove(t, (int)playerp->legs[4], (int)playerp->legs[5]))
			{
				playerp->accs[2] = -3*gvy-3*gvx;
				playerp->accs[3] = 3*gvx-3*gvy;
				playerp->accs[0] = -gvy;
				playerp->accs[1] = gvx;
				moved = true;
			}
		}
		else
		{
			if (INBOND(playerp->legs[12], playerp->legs[13]) && !sim->EvalMove(t, (int)playerp->legs[12], (int)playerp->legs[13]))
			{
				playerp->accs[6] = -3*gvy-3*gvx;
				playerp->accs[7] = 3*gvx-3*gvy;
				playerp->accs[0] = -gvy;
				playerp->accs[1] = gvx;
				moved = true;
			}
		}
		if (!moved && playerp->rocketBoots)
		{
			parts[i].vx -= rocketBootsHeadEffect*rby;
			parts[i].vy += rocketBootsHeadEffect*rbx;
			playerp->accs[2] -= rocketBootsFeetEffect*rby;
			playerp->accs[6] -= rocketBootsFeetEffect*rby;
			playerp->accs[3] += rocketBootsFeetEffect*rbx;
			playerp->accs[7] += rocketBootsFeetEffect*rbx;
			for (int leg = 0; leg < 2; leg++)
			{
				if (leg == 1 && (((int)(playerp->comm)&0x02) == 0x02))
					continue;
				int footX = (int)playerp->legs[leg*8+4], footY = (int)playerp->legs[leg*8+5];
				int np = sim->part_create(-1, footX, footY, PT_PLSM);
				if (np >= 0)
				{
					parts[np].vx = parts[i].vx+rby*25;
					parts[np].vy = parts[i].vy-rbx*25;
					parts[np].life += 30;
				}
			}
		}
	}

	// Go right
	if (((int)(playerp->comm)&0x02) == 0x02)
	{
		bool moved = false;
		if (dl < dr)
		{
			if (INBOND(playerp->legs[4], playerp->legs[5]) && !sim->EvalMove(t, (int)playerp->legs[4], (int)playerp->legs[5]))
			{
				playerp->accs[2] = 3*gvy-3*gvx;
				playerp->accs[3] = -3*gvx-3*gvy;
				playerp->accs[0] = gvy;
				playerp->accs[1] = -gvx;
				moved = true;
			}
		}
		else
		{
			if (INBOND(playerp->legs[12], playerp->legs[13]) && !sim->EvalMove(t, (int)playerp->legs[12], (int)playerp->legs[13]))
			{
				playerp->accs[6] = 3*gvy-3*gvx;
				playerp->accs[7] = -3*gvx-3*gvy;
				playerp->accs[0] = gvy;
				playerp->accs[1] = -gvx;
				moved = true;
			}
		}
		if (!moved && playerp->rocketBoots)
		{
			parts[i].vx += rocketBootsHeadEffect*rby;
			parts[i].vy -= rocketBootsHeadEffect*rbx;
			playerp->accs[2] += rocketBootsFeetEffect*rby;
			playerp->accs[6] += rocketBootsFeetEffect*rby;
			playerp->accs[3] -= rocketBootsFeetEffect*rbx;
			playerp->accs[7] -= rocketBootsFeetEffect*rbx;
			for (int leg = 0; leg < 2; leg++)
			{
				if (leg == 0 && (((int)(playerp->comm)&0x01) == 0x01))
					continue;
				int footX = (int)playerp->legs[leg*8+4], footY = (int)playerp->legs[leg*8+5];
				int np = sim->part_create(-1, footX, footY, PT_PLSM);
				if (np >= 0)
				{
					parts[np].vx = parts[i].vx-rby*25;
					parts[np].vy = parts[i].vy+rbx*25;
					parts[np].life += 30;
				}
			}
		}
	}

	if (playerp->rocketBoots && ((int)(playerp->comm)&0x03) == 0x03)
	{
		// Pressing left and right simultaneously with rocket boots on slows the stickman down
		// Particularly useful in zero gravity
		parts[i].vx *= 0.5f;
		parts[i].vy *= 0.5f;
		playerp->accs[2] = playerp->accs[6] = 0;
		playerp->accs[3] = playerp->accs[7] = 0;
	}

	//Jump
	if (((int)(playerp->comm)&0x04) == 0x04)
	{
		if (playerp->rocketBoots)
		{
			if (rbLowGrav)
			{
				parts[i].vx -= rocketBootsHeadEffect*rbx;
				parts[i].vy -= rocketBootsHeadEffect*rby;
				playerp->accs[2] -= rocketBootsFeetEffect*rbx;
				playerp->accs[6] -= rocketBootsFeetEffect*rbx;
				playerp->accs[3] -= rocketBootsFeetEffect*rby;
				playerp->accs[7] -= rocketBootsFeetEffect*rby;
			}
			else
			{
				parts[i].vx -= rocketBootsHeadEffectV*rbx;
				parts[i].vy -= rocketBootsHeadEffectV*rby;
				playerp->accs[2] -= rocketBootsFeetEffectV*rbx;
				playerp->accs[6] -= rocketBootsFeetEffectV*rbx;
				playerp->accs[3] -= rocketBootsFeetEffectV*rby;
				playerp->accs[7] -= rocketBootsFeetEffectV*rby;
			}
			for (int leg = 0; leg < 2; leg++)
			{
				int footX = (int)playerp->legs[leg*8+4], footY = (int)playerp->legs[leg*8+5];
				int np = sim->part_create(-1, footX, footY+1, PT_PLSM);
				if (np >= 0)
				{
					parts[np].vx = parts[i].vx+rbx*30;
					parts[np].vy = parts[i].vy+rby*30;
					parts[np].life += 10;
				}
			}
		}
		else if ((INBOND(playerp->legs[4], playerp->legs[5]) && !sim->EvalMove(t, (int)playerp->legs[4], (int)playerp->legs[5])) ||
				 (INBOND(playerp->legs[12], playerp->legs[13]) && !sim->EvalMove(t, (int)playerp->legs[12], (int)playerp->legs[13])))
		{
			parts[i].vx -= 4*gvx;
			parts[i].vy -= 4*gvy;
			playerp->accs[2] -= gvx;
			playerp->accs[6] -= gvx;
			playerp->accs[3] -= gvy;
			playerp->accs[7] -= gvy;
		}
	}

	// Charge detector wall if foot inside
	if (bmap[(int)(playerp->legs[5]+0.5)/CELL][(int)(playerp->legs[4]+0.5)/CELL]==WL_DETECT)
		set_emap((int)playerp->legs[4]/CELL, (int)playerp->legs[5]/CELL);
	if (bmap[(int)(playerp->legs[13]+0.5)/CELL][(int)(playerp->legs[12]+0.5)/CELL]==WL_DETECT)
		set_emap((int)(playerp->legs[12]+0.5)/CELL, (int)(playerp->legs[13]+0.5)/CELL);

	// Searching for particles near head
	for (int rx = -2; rx < 3; rx++)
		for (int ry = -2; ry < 3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				int r = pmap[y+ry][x+rx];
				if (!r)
					r = photons[y+ry][x+rx];

				if (!r && !bmap[(y+ry)/CELL][(x+rx)/CELL])
					continue;
				
				if (ptypes[r&0xFF].falldown != 0 || ptypes[r&0xFF].state == ST_GAS
				    || ptypes[r&0xFF].properties&TYPE_GAS
				    || ptypes[r&0xFF].properties&TYPE_LIQUID
				    || (r&0xFF) == PT_NEUT || (r&0xFF) == PT_PHOT)
				{
					if (!playerp->rocketBoots || (r&0xFF) != PT_PLSM)
						playerp->elem = r&0xFF;  //Current element
				}
				if ((r&0xFF) == PT_TESC || (r&0xFF) == PT_LIGH)
					playerp->elem = PT_LIGH;
				if ((r&0xFF) == PT_PLNT && parts[i].life<100) //Plant gives him 5 HP
				{
					if (parts[i].life<=95)
						parts[i].life += 5;
					else
						parts[i].life = 100;
					sim->part_kill(r>>8);
				}

				if ((r&0xFF) == PT_NEUT)
				{
					if (parts[i].life<=100) parts[i].life -= (102-parts[i].life)/2;
					else parts[i].life = (int)(parts[i].life*0.9f);
					sim->part_kill(r>>8);
				}
				if (bmap[(ry+y)/CELL][(rx+x)/CELL] == WL_FAN)
					playerp->elem = SPC_AIR;
				else if (bmap[(ry+y)/CELL][(rx+x)/CELL] == WL_EHOLE)
					playerp->rocketBoots = false;
				else if (bmap[(ry+y)/CELL][(rx+x)/CELL] == WL_GRAV)
					playerp->rocketBoots = true;
#ifdef NOMOD
				if ((r&0xFF) == PT_PRTI)
#else
				if ((r&0xFF) == PT_PRTI || (r&0xFF) == PT_PPTI)
#endif
					Interact(sim, playerp, i, rx, ry);

				// Interact() may kill STKM
				if (!parts[i].type)
					return 1;
			}

	// Head position
	int rx = x + 3*((((int)playerp->pcomm)&0x02) == 0x02) - 3*((((int)playerp->pcomm)&0x01) == 0x01);
	int ry = y - 3*(playerp->pcomm == 0);

	// Spawn
	if (((int)(playerp->comm)&0x08) == 0x08)
	{
		ry -= 2*(rand()%2)+1;
		int r = pmap[ry][rx];
		if (ptypes[r&0xFF].state == ST_SOLID)
		{
			if (pmap[ry][rx])
				sim->spark_conductive_attempt(pmap[ry][rx]>>8, rx, ry);
			playerp->frames = 0;
		}
		else
		{
			int np = -1;
			if (playerp->elem == SPC_AIR)
			{
				for(int j = -4; j < 5; j++)
					for (int k = -4; k < 5; k++)
					{
						int x = rx + 3*((((int)playerp->pcomm)&0x02) == 0x02) - 3*((((int)playerp->pcomm)&0x01) == 0x01)+j;
						int y = ry+k;
						pv[y/CELL][x/CELL] += 0.03f;
						if (y+CELL<YRES)
							pv[y/CELL+1][x/CELL] += 0.03f;
						if (x+CELL<XRES)
						{
							pv[y/CELL][x/CELL+1] += 0.03f;
							if (y+CELL<YRES)
								pv[y/CELL+1][x/CELL+1] += 0.03f;
						}

						//sim->CreateTool(x, y, TOOL_AIR);
					}
			}
			// limit lightning creation rate
			else if (playerp->elem==PT_LIGH && playerp->frames<30)
				np = -1;
			else
				np = sim->part_create(-1, rx, ry, playerp->elem);
			if ((np < NPART) && np >= 0)
			{
				if (playerp->elem == PT_PHOT)
				{
					int random = abs(rand()%3-1)*3;
					if (random == 0)
					{
						sim->part_kill(np);
					}
					else
					{
						parts[np].vy = 0;
						if (((int)playerp->pcomm)&(0x01|0x02))
							parts[np].vx = (float)(((((int)playerp->pcomm)&0x02) == 0x02) - (((int)(playerp->pcomm)&0x01) == 0x01))*random;
						else
							parts[np].vx = (float)random;
					}
				}
				else if (playerp->elem == PT_LIGH)
				{
					float angle;
					int power = 100;
					if (gvx != 0 || gvy != 0)
						angle = atan2(gvx, gvy)*180.0f/M_PI;
					else
						angle = (float)(rand()%360);
					if (((int)playerp->pcomm)&0x01)
						angle += 180;
					if (angle > 360)
						angle -= 360;
					if (angle < 0)
						angle += 360;
					parts[np].tmp = (int)angle;
					parts[np].life = rand()%(2+power/15) + power/7;
					parts[np].temp = parts[np].life * power/2.5f;
					parts[np].tmp2 = 1;
				}
				else if (playerp->elem != SPC_AIR)
				{
					parts[np].vx -= -gvy*(5*((((int)playerp->pcomm)&0x02) == 0x02) - 5*(((int)(playerp->pcomm)&0x01) == 0x01));
					parts[np].vy -= gvx*(5*((((int)playerp->pcomm)&0x02) == 0x02) - 5*(((int)(playerp->pcomm)&0x01) == 0x01));
					parts[i].vx -= (ptypes[(int)playerp->elem].weight*parts[np].vx)/1000;
				}
				playerp->frames = 0;
			}

		}
	}

	// Simulation of joints
	float d = 25/(pow((playerp->legs[0]-playerp->legs[4]), 2) + pow((playerp->legs[1]-playerp->legs[5]), 2)+25) - 0.5f;  //Fast distance
	playerp->legs[4] -= (playerp->legs[0]-playerp->legs[4])*d;
	playerp->legs[5] -= (playerp->legs[1]-playerp->legs[5])*d;
	playerp->legs[0] += (playerp->legs[0]-playerp->legs[4])*d;
	playerp->legs[1] += (playerp->legs[1]-playerp->legs[5])*d;

	d = 25/(pow((playerp->legs[8]-playerp->legs[12]), 2) + pow((playerp->legs[9]-playerp->legs[13]), 2)+25) - 0.5f;
	playerp->legs[12] -= (playerp->legs[8]-playerp->legs[12])*d;
	playerp->legs[13] -= (playerp->legs[9]-playerp->legs[13])*d;
	playerp->legs[8] += (playerp->legs[8]-playerp->legs[12])*d;
	playerp->legs[9] += (playerp->legs[9]-playerp->legs[13])*d;

	d = 36/(pow((playerp->legs[0]-parts[i].x), 2) + pow((playerp->legs[1]-parts[i].y), 2)+36) - 0.5f;
	parts[i].vx -= (playerp->legs[0]-parts[i].x)*d;
	parts[i].vy -= (playerp->legs[1]-parts[i].y)*d;
	playerp->legs[0] += (playerp->legs[0]-parts[i].x)*d;
	playerp->legs[1] += (playerp->legs[1]-parts[i].y)*d;

	d = 36/(pow((playerp->legs[8]-parts[i].x), 2) + pow((playerp->legs[9]-parts[i].y), 2)+36) - 0.5f;
	parts[i].vx -= (playerp->legs[8]-parts[i].x)*d;
	parts[i].vy -= (playerp->legs[9]-parts[i].y)*d;
	playerp->legs[8] += (playerp->legs[8]-parts[i].x)*d;
	playerp->legs[9] += (playerp->legs[9]-parts[i].y)*d;

	if (INBOND(playerp->legs[4], playerp->legs[5]) && !sim->EvalMove(t, (int)playerp->legs[4], (int)playerp->legs[5]))
	{
		playerp->legs[4] = playerp->legs[6];
		playerp->legs[5] = playerp->legs[7];
	}

	if (INBOND(playerp->legs[12], playerp->legs[13]) && !sim->EvalMove(t, (int)playerp->legs[12], (int)playerp->legs[13]))
	{
		playerp->legs[12] = playerp->legs[14];
		playerp->legs[13] = playerp->legs[15];
	}

	//This makes stick man "pop" from obstacles
	if (INBOND(playerp->legs[4], playerp->legs[5]) && !sim->EvalMove(t, (int)playerp->legs[4], (int)playerp->legs[5]))
	{
		float t;
		t = playerp->legs[4]; playerp->legs[4] = playerp->legs[6]; playerp->legs[6] = t;
		t = playerp->legs[5]; playerp->legs[5] = playerp->legs[7]; playerp->legs[7] = t;
	}

	if (INBOND(playerp->legs[12], playerp->legs[13]) && !sim->EvalMove(t, (int)playerp->legs[12], (int)playerp->legs[13]))
	{
		float t;
		t = playerp->legs[12]; playerp->legs[12] = playerp->legs[14]; playerp->legs[14] = t;
		t = playerp->legs[13]; playerp->legs[13] = playerp->legs[15]; playerp->legs[15] = t;
	}

	// Keeping legs distance
	if ((pow((playerp->legs[4] - playerp->legs[12]), 2) + pow((playerp->legs[5]-playerp->legs[13]), 2)) < 16)
	{
		float tvx = -gvy;
		float tvy = gvx;

		if (tvx || tvy)
		{
			playerp->accs[2] -= 0.2f*tvx/hypot(tvx, tvy);
			playerp->accs[3] -= 0.2f*tvy/hypot(tvx, tvy);

			playerp->accs[6] += 0.2f*tvx/hypot(tvx, tvy);
			playerp->accs[7] += 0.2f*tvy/hypot(tvx, tvy);
		}
	}

	if ((pow((playerp->legs[0] - playerp->legs[8]), 2) + pow((playerp->legs[1]-playerp->legs[9]), 2)) < 16)
	{
		float tvx = -gvy;
		float tvy = gvx;

		if (tvx || tvy)
		{
			playerp->accs[0] -= 0.2f*tvx/hypot(tvx, tvy);
			playerp->accs[1] -= 0.2f*tvy/hypot(tvx, tvy);

			playerp->accs[4] += 0.2f*tvx/hypot(tvx, tvy);
			playerp->accs[5] += 0.2f*tvy/hypot(tvx, tvy);
		}
	}

	// If legs touch something
	Interact(sim, playerp, i, (int)(playerp->legs[4]+0.5), (int)(playerp->legs[5]+0.5));
	Interact(sim, playerp, i, (int)(playerp->legs[12]+0.5), (int)(playerp->legs[13]+0.5));
	Interact(sim, playerp, i, (int)(playerp->legs[4]+0.5), (int)playerp->legs[5]);
	Interact(sim, playerp, i, (int)(playerp->legs[12]+0.5), (int)playerp->legs[13]);
	if (!parts[i].type)
		return 1;

	parts[i].ctype = playerp->elem;
	return 0;
}

void STKM_ElementDataContainer::Interact(Simulation* sim, Stickman *playerp, int i, int x, int y)
{
	if (x<0 || y<0 || x>=XRES || y>=YRES || !parts[i].type)
		return;
	int r = pmap[y][x];
	if (r)
	{
		if ((r&0xFF)==PT_SPRK && playerp->elem!=PT_LIGH) //If on charge
		{
			parts[i].life -= (int)(rand()*20/RAND_MAX)+32;
		}

		if (ptypes[r&0xFF].hconduct && ((r&0xFF)!=PT_HSWC||parts[r>>8].life==10) && ((playerp->elem!=PT_LIGH && parts[r>>8].temp>=323) || parts[r>>8].temp<=243) && (!playerp->rocketBoots || (r&0xFF)!=PT_PLSM))
		{
			parts[i].life -= 2;
			playerp->accs[3] -= 1;
		}
			
		if (ptypes[r&0xFF].properties&PROP_DEADLY)
			switch (r&0xFF)
			{
				case PT_ACID:
					parts[i].life -= 5;
					break;
				default:
					parts[i].life -= 1;
			}

		if (ptypes[r&0xFF].properties&PROP_RADIOACTIVE)
			parts[i].life -= 1;

#ifdef NOMOD
		if ((r&0xFF)==PT_PRTI && parts[i].type)
#else
		if (((r&0xFF)==PT_PRTI || (r&0xFF)==PT_PPTI) && parts[i].type)
#endif
		{
			int t = parts[i].type;
			unsigned char tmp = parts[i].tmp&0xFF;
			PortalChannel *channel = ((PRTI_ElementDataContainer*)sim->elementData[PT_PRTI])->GetParticleChannel(sim, r>>8);
			if (channel->StoreParticle(sim, i, 1))//slot=1 gives rx=0, ry=1 in PRTO_update
			{
				//stop new STKM/fighters being created to replace the ones in the portal:
				if (t==PT_FIGH)
					((FIGH_ElementDataContainer*)sim->elementData[PT_FIGH])->AllocSpecific(tmp);
				else
					playerp->spwn = 1;
			}
		}

		if (((r&0xFF)==PT_BHOL || (r&0xFF)==PT_NBHL) && parts[i].type)
		{
			if (!legacy_enable)
			{
				parts[r>>8].temp = restrict_flt(parts[r>>8].temp+parts[i].temp/2, MIN_TEMP, MAX_TEMP);
			}
			sim->part_kill(i);
		}
		if (((r&0xFF)==PT_VOID || ((r&0xFF)==PT_PVOD && parts[r>>8].life==10)) && (!parts[r>>8].ctype || (parts[r>>8].ctype==parts[i].type)!=(parts[r>>8].tmp&1)) && parts[i].type)
		{
			sim->part_kill(i);
		}
	}
}

void STKM_ElementDataContainer::HandleKeys(int sdl_key, int sdl_rkey)
{
	//  4
	//1 8 2
	if (sdl_key == SDLK_RIGHT)
	{
		player.comm = (int)(player.comm)|0x02;  //Go right command
	}
	if (sdl_key == SDLK_LEFT)
	{
		player.comm = (int)(player.comm)|0x01;  //Go left command
	}
	if (sdl_key == SDLK_DOWN && ((int)(player.comm)&0x08)!=0x08)
	{
		player.comm = (int)(player.comm)|0x08;  //Use element command
	}
	if (sdl_key == SDLK_UP && ((int)(player.comm)&0x04)!=0x04)
	{
		player.comm = (int)(player.comm)|0x04;  //Jump command
	}

	if (sdl_key == SDLK_d)
	{
		player2.comm = (int)(player2.comm)|0x02;  //Go right command
	}
	if (sdl_key == SDLK_a)
	{
		player2.comm = (int)(player2.comm)|0x01;  //Go left command
	}
	if (sdl_key == SDLK_s && ((int)(player2.comm)&0x08)!=0x08)
	{
		player2.comm = (int)(player2.comm)|0x08;  //Use element command
	}
	if (sdl_key == SDLK_w && ((int)(player2.comm)&0x04)!=0x04)
	{
		player2.comm = (int)(player2.comm)|0x04;  //Jump command
	}

	if (sdl_rkey == SDLK_RIGHT || sdl_rkey == SDLK_LEFT)
	{
		player.pcomm = player.comm;  //Saving last movement
		player.comm = (int)(player.comm)&12;  //Stop command
	}
	if (sdl_rkey == SDLK_UP)
	{
		player.comm = (int)(player.comm)&11;
	}
	if (sdl_rkey == SDLK_DOWN)
	{
		player.comm = (int)(player.comm)&7;
	}

	if (sdl_rkey == SDLK_d || sdl_rkey == SDLK_a)
	{
		player2.pcomm = player2.comm;  //Saving last movement
		player2.comm = (int)(player2.comm)&12;  //Stop command
	}
	if (sdl_rkey == SDLK_w)
	{
		player2.comm = (int)(player2.comm)&11;
	}
	if (sdl_rkey == SDLK_s)
	{
		player2.comm = (int)(player2.comm)&7;
	}
}

int STKM_update(UPDATE_FUNC_ARGS)
{
	((STKM_ElementDataContainer*)sim->elementData[PT_STKM])->Run(((STKM_ElementDataContainer*)sim->elementData[PT_STKM])->GetStickman1(), UPDATE_FUNC_SUBCALL_ARGS);
	return 0;
}

int STKM_graphics(GRAPHICS_FUNC_ARGS)
{
	*colr = *colg = *colb = *cola = 0;
	*pixel_mode = PSPEC_STICKMAN;
	return 0;
}

bool STKM_create_allowed(ELEMENT_CREATE_ALLOWED_FUNC_ARGS)
{
	return sim->elementCount[PT_STKM]<=0 && !((STKM_ElementDataContainer*)sim->elementData[PT_STKM])->GetStickman1()->spwn;
}

void STKM_create(ELEMENT_CREATE_FUNC_ARGS)
{
	int id = sim->part_create(-3, x, y, PT_SPAWN);
	if (id >= 0)
		((STKM_ElementDataContainer*)sim->elementData[PT_STKM])->GetStickman1()->spawnID = id;
}

void STKM_ChangeType(ELEMENT_CHANGETYPE_FUNC_ARGS)
{
	if (to == PT_STKM)
	{
		((STKM_ElementDataContainer*)sim->elementData[PT_STKM])->InitLegs(((STKM_ElementDataContainer*)sim->elementData[PT_STKM])->GetStickman1(), i);
	}
	else
		((STKM_ElementDataContainer*)sim->elementData[PT_STKM])->GetStickman1()->spwn = 0;
}

void STKM_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_STKM";
	elem->Name = "STKM";
	elem->Colour = COLPACK(0xFFE0A0);
	elem->MenuVisible = 1;
	elem->MenuSection = SC_SPECIAL;
	elem->Enabled = 1;

	elem->Advection = 0.5f;
	elem->AirDrag = 0.00f * CFDS;
	elem->AirLoss = 0.2f;
	elem->Loss = 1.0f;
	elem->Collision = 0.0f;
	elem->Gravity = 0.0f;
	elem->Diffusion = 0.0f;
	elem->HotAir = 0.00f	* CFDS;
	elem->Falldown = 0;

	elem->Flammable = 0;
	elem->Explosive = 0;
	elem->Meltable = 0;
	elem->Hardness = 0;

	elem->Weight = 50;

	elem->DefaultProperties.temp = R_TEMP+14.6f+273.15f;
	elem->HeatConduct = 0;
	elem->Latent = 0;
	elem->Description = "Stickman. Don't kill him! Control with the arrow keys.";

	elem->State = ST_NONE;
	elem->Properties = PROP_NOCTYPEDRAW;

	elem->LowPressureTransitionThreshold = IPL;
	elem->LowPressureTransitionElement = NT;
	elem->HighPressureTransitionThreshold = IPH;
	elem->HighPressureTransitionElement = NT;
	elem->LowTemperatureTransitionThreshold = ITL;
	elem->LowTemperatureTransitionElement = NT;
	elem->HighTemperatureTransitionThreshold = 620.0f;
	elem->HighTemperatureTransitionElement = PT_FIRE;

	elem->DefaultProperties.life = 100;

	elem->Update = &STKM_update;
	elem->Graphics = &STKM_graphics;
	elem->Func_Create_Allowed = &STKM_create_allowed;
	elem->Func_Create = &STKM_create;
	elem->Func_ChangeType = &STKM_ChangeType;
	elem->Init = &STKM_init_element;

	if (sim->elementData[t])
	{
		delete sim->elementData[t];
	}
	sim->elementData[t] = new STKM_ElementDataContainer;
}
