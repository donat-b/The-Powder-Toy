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

#ifndef SIMULATION_ELEMENTS_PRTI_H
#define SIMULATION_ELEMENTS_PRTI_H 

#include <cstring>
#include "common/tpt-minmax.h"
#include "simulation/ElementDataContainer.h"
#include "simulation/elements/PPIP.h"
#include "powder.h"

extern const int portal_rx[8];
extern const int portal_ry[8];

class PortalChannel
{
public:
	static const int storageSize = 80;
	int particleCount[8];
	particle portalp[8][80];
	// Store a particle in a given slot (one of the 8 neighbour positions) for this portal channel, then kills the original
	// Does not check whether the particle should be in a portal
	// Returns true on success, or false if the portal is full
	bool StoreParticle(Simulation *sim, int store_i, int slot)
	{
		if (particleCount[slot]>=storageSize)
			return false;
		for (int nnx=0; nnx<80; nnx++)
			if (!portalp[slot][nnx].type)
			{
				if (parts[store_i].type == PT_STOR)
				{
					if (sim->IsElement(parts[store_i].tmp) && (sim->elements[parts[store_i].tmp].Properties & (TYPE_PART | TYPE_LIQUID | TYPE_GAS | TYPE_ENERGY)))
					{
						PIPE_transfer_pipe_to_part(parts+store_i, &portalp[slot][nnx]);
						return true;
					}
					return false;
				}
				else
				{
					portalp[slot][nnx] = parts[store_i];
					particleCount[slot]++;
					if (parts[store_i].type==PT_SPRK)
						sim->part_change_type(store_i,(int)(sim->parts[store_i].x+0.5f),(int)(sim->parts[store_i].y+0.5f),sim->parts[store_i].ctype);
					else
						sim->part_kill(store_i);
					return true;
				}
			}
		return false;
	}
	particle * AllocParticle(int slot)
	{
		if (particleCount[slot]>=storageSize)
			return NULL;
		for (int nnx=0; nnx<80; nnx++)
			if (!portalp[slot][nnx].type)
			{
				particleCount[slot]++;
				return &(portalp[slot][nnx]);
			}
		return NULL;
	}
	void Simulation_Cleared()
	{
		memset(portalp, 0, sizeof(portalp));
		memset(particleCount, 0, sizeof(particleCount));
	}
};

class PRTI_ElementDataContainer : public ElementDataContainer
{
private:
	PortalChannel channels[CHANNELS];
public:	
	PRTI_ElementDataContainer()
	{
		for (int i=0; i<CHANNELS; i++)
			channels[i].Simulation_Cleared();
	}
	PortalChannel* GetChannel(int i)
	{
		return channels+i;
	}
	PortalChannel* GetParticleChannel(Simulation *sim, int i)
	{
		sim->parts[i].tmp = (int)((sim->parts[i].temp-73.15f)/100+1);
		if (sim->parts[i].tmp>=CHANNELS) sim->parts[i].tmp = CHANNELS-1;
		else if (sim->parts[i].tmp<0) sim->parts[i].tmp = 0;
		return channels+parts[i].tmp;
	}
	static int GetSlot(int rx, int ry)
	{
		if (rx>1 || ry>1 || rx<-1 || ry<-1)
		{
			// scale down if larger than +-1
			float rmax = (float)std::max(std::fabs(rx), std::fabs(ry));
			rx = (int)(rx/rmax);
			ry = (int)(ry/rmax);
		}
		for (int i=0; i<8; i++)
		{
			if (rx==portal_rx[i] && ry==portal_ry[i])
				return i;
		}
		return 1; // Dunno, put it in the top of the portal
	}
	virtual void Simulation_Cleared(Simulation *sim)
	{
		for (int i=0; i<CHANNELS; i++)
			channels[i].Simulation_Cleared();
	}
};

#endif
