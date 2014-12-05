#ifndef BUTTON_H
#define BUTTON_H

#include <string>
#include "common/Point.h"

class VideoBuffer;
class Button
{
private:

protected:
	Point position;
	Point size;

public:
	Button(Point position, Point size);

	Point GetPosition() { return position; }
	Point GetSize() { return size; }

	virtual void OnMouseDown(int x, int y, unsigned char button);
	virtual void OnMouseUp(int x, int y, unsigned char button);
	virtual void OnMouseMoved(int x, int y, Point difference);
	virtual void OnKeyPress(int key, unsigned short character, unsigned char modifiers);
	virtual void OnDraw(VideoBuffer* vid);
	virtual void OnTick();
};

#endif
