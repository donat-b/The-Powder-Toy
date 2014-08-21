#ifndef BRUSH_H
#define BRUSH_H

#include "common/Point.h"

enum { CIRCLE_BRUSH, SQUARE_BRUSH, TRI_BRUSH};
#define BRUSH_NUM 3

//TODO: maybe use bitmaps and support tpt++ custom brushes?
class Brush
{
	Point radius;
	int shape;

public:
	Brush(Point radius_, int shape_) :
		radius(radius_),
		shape(shape_)
	{

	}

	void SetRadius(Point radius_);
	void ChangeRadius(Point change);
	Point GetRadius() { return radius; }
	void SetShape(int shape_) { shape = shape_; }
	int GetShape() { return shape; }
	bool IsInside(int x, int y);
};

extern Brush* currentBrush;

#endif
