#ifndef BRUSH_H
#define BRUSH_H

#include "common/Point.h"

#define CIRCLE_BRUSH 0
#define SQUARE_BRUSH 1
#define TRI_BRUSH 2
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
	//bool InBrush(int x, int y);
};

extern Brush* currentBrush;
int InCurrentBrush(int i, int j, int rx, int ry);

#endif
