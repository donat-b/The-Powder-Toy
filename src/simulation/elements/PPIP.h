#ifndef PPIP_H
#define PPIP_H

#include "simulation/ElementDataContainer.h"
#include "simulation/Simulation.h"

class PPIP_ElementDataContainer : public ElementDataContainer
{
public:
	bool ppip_changed;
	PPIP_ElementDataContainer()
	{
		ppip_changed = false;
	}
	virtual void Simulation_BeforeUpdate(Simulation *sim)
	{
		if (ppip_changed)
		{
			for (int i = 0; i <= sim->parts_lastActiveIndex; i++)
			{
				if (parts[i].type == PT_PPIP)
				{
					parts[i].tmp |= (parts[i].tmp&0xE0000000)>>3;
					parts[i].tmp &= ~0xE0000000;
				}
			}
			ppip_changed = false;
		}
	}
};

#endif
