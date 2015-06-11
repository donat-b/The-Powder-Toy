#ifndef EMP_H
#define EMP_H

#include "simulation/ElementDataContainer.h"
#include "simulation/Simulation.h"

class EMP_ElementDataContainer : public ElementDataContainer
{
public:
	unsigned int emp_decor;
	EMP_ElementDataContainer()
	{
		emp_decor = 0;
	}

	virtual void Simulation_Cleared(Simulation *sim)
	{
		emp_decor = 0;
	}

	void Activate()
	{
		emp_decor += 3;
		if (emp_decor > 40)
			emp_decor = 40;
	}

	void Deactivate()
	{
		if (emp_decor)
			emp_decor -= emp_decor/25+2;

		if (emp_decor > 40)
			emp_decor = 40;
	}
};

#endif
