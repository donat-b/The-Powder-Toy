#include <math.h>
#include <algorithm>
#include "Brush.h"

/*bool Brush::InBrush(int x, int y)
{
	switch (shape)
	{
	case CIRCLE_BRUSH:
		return (pow(x, 2.0)*pow(radius.Y, 2.0)+pow(y, 2.0)*pow(radius.X, 2.0) <= pow(radius.X, 2.0)*pow(radius.Y, 2.0));
		break;
	case SQUARE_BRUSH:
		return (abs(x) <= radius.X && abs(y) <= radius.Y);
		break;
	case TRI_BRUSH:
		return ((abs((radius.X+2*x)*radius.Y+radius.X*y) + abs(2*radius.X*(y-radius.Y)) + abs((radius.X-2*x)*radius.Y+radius.X*y)) <= (4*radius.X*radius.Y));
		break;
	default:
		return 0;
		break;
	}
}*/

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

//TODO: remove
int InCurrentBrush(int i, int j, int rx, int ry)
{
	switch (currentBrush->GetShape())
	{
	case CIRCLE_BRUSH:
		return (pow(i, 2.0)*pow(ry, 2.0)+pow(j, 2.0)*pow(rx, 2.0)<=pow(rx, 2.0)*pow(ry, 2.0));
		break;
	case SQUARE_BRUSH:
		return (abs(i) <= rx && abs(j) <= ry);
		break;
	case TRI_BRUSH:
		return ((abs((rx+2*i)*ry+rx*j) + abs(2*rx*(j-ry)) + abs((rx-2*i)*ry+rx*j))<=(4*rx*ry));
		break;
	default:
		return 0;
		break;
	}
}

Brush* currentBrush;