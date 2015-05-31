#ifndef COMPONENT_H
#define COMPONENT_H

#include "common/Point.h"

class VideoBuffer;
class Window_;
class Component
{
	Window_ *parent;

protected:
	Point position;
	Point size;
	bool isMouseInside;

public:
	Component(Point position, Point size);

	void SetParent(Window_ *parentWindow) { parent = parentWindow; }
	Window_* GetParent() { return parent; }
	bool IsFocused();
	bool IsClicked();
	bool IsMouseDown();
	Point GetPosition() { return position; }
	void SetPosition(Point position_) { position = position_; }
	Point GetSize() { return size; }
	void SetMouseInside(bool mouseInside) { isMouseInside = mouseInside; } // used by Window.cpp

	virtual void OnMouseDown(int x, int y, unsigned char button) { }
	virtual void OnMouseUp(int x, int y, unsigned char button) { }
	virtual void OnMouseMoved(int x, int y, Point difference) { }
	virtual void OnKeyPress(int key, unsigned short character, unsigned char modifiers) { }
	virtual void OnDraw(VideoBuffer* vid) { }
	virtual void OnTick() { }

	virtual void OnDefocus() { }
};

#endif
