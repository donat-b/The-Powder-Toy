#ifndef WINDOW_H
#define WINDOW_H

#include "common/Point.h"

class VideoBuffer;
class Window_
{
private:
	Point position;
	Point size;
	VideoBuffer* videoBuffer;

public:
	Window_(Point position, Point size);
	~Window_();

	virtual void DoTick(float dt);
	virtual void DoDraw();

	virtual void DoMouseMove(int x, int y, int dx, int dy);
	virtual void DoMouseDown(int x, int y, unsigned char button);
	virtual void DoMouseUp(int x, int y, unsigned char button);
	virtual void DoMouseWheel(int x, int y, int d);
	virtual void DoKeyPress(int key, unsigned short character, unsigned char modifiers);
	virtual void DoKeyRelease(int key, unsigned short character, unsigned char modifiers);

	VideoBuffer* GetVid() { return videoBuffer; }
};

#endif
