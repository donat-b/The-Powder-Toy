#ifndef COMPONENT_H
#define COMPONENT_H

#include "common/tpt-stdint.h"
#include "common/Point.h"

class VideoBuffer;
class Window_;
class Component
{
	Window_ *parent;

protected:
	Point position;
	Point size;
	bool isMouseInside; // keeps track of if mouse is inside so child classes don't have to
	bool visible; // ignore all events + tick and hide component
	bool enabled; // ignore all events, still call tick and draw function

public:
	Component(Point position, Point size);

	// delay deleting and adding components
	bool toDelete;
	bool toAdd;

	bool IsFocused();
	bool IsClicked();
	bool IsMouseDown();

	void SetParent(Window_ *parentWindow) { parent = parentWindow; }
	Window_* GetParent() { return parent; }
	Point GetPosition() { return position; }
	void SetPosition(Point position_) { position = position_; }
	Point GetSize() { return size; }
	void SetMouseInside(bool mouseInside) { isMouseInside = mouseInside; } // used by Window.cpp
	bool IsVisible() { return visible; }
	void SetVisible(bool visible_) { visible = visible_; toAdd = false; }
	bool IsEnabled() { return enabled; }
	void SetEnabled(bool enabled_) { enabled = enabled_; }

	virtual void OnMouseDown(int x, int y, unsigned char button) { }
	virtual void OnMouseUp(int x, int y, unsigned char button) { }
	virtual void OnMouseMoved(int x, int y, Point difference) { }
	virtual void OnKeyPress(int key, unsigned short character, unsigned char modifiers) { }
	virtual void OnDraw(VideoBuffer* vid) { }
	virtual void OnTick(uint32_t ticks) { }

	virtual void OnDefocus() { }
};

#endif
