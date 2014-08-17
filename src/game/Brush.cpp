#include <math.h>
#include <algorithm>
#include "Brush.h"

bool Brush::IsInside(int x, int y)
{
	switch (shape)
	{
	case CIRCLE_BRUSH:
		return (pow(x, 2.0)*pow(radius.Y, 2.0)+pow(y, 2.0)*pow(radius.X, 2.0) <= pow(radius.X, 2.0)*pow(radius.Y, 2.0));
		break;
	case SQUARE_BRUSH:
		return (fabs(x) <= radius.X && fabs(y) <= radius.Y);
		break;
	case TRI_BRUSH:
		return ((fabs((radius.X+2*x)*radius.Y+radius.X*y) + fabs(2*radius.X*(y-radius.Y)) + fabs((radius.X-2*x)*radius.Y+radius.X*y)) <= (4*radius.X*radius.Y));
		break;
	default:
		return 0;
		break;
	}
}

void Brush::SetRadius(Point radius_)
{
	radius = radius_;
	radius.X = std::min(1180, std::max(0, radius.X));
	radius.Y = std::min(1180, std::max(0, radius.Y));
}

void Brush::ChangeRadius(Point change)
{
	radius.X += change.X;
	radius.Y += change.Y;
	radius.X = std::min(1180, std::max(0, radius.X));
	radius.Y = std::min(1180, std::max(0, radius.Y));
}
