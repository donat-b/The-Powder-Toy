#ifndef WINDOW_H
#define WINDOW_H

#include <vector>
#include "common/Point.h"
#include "Component.h"

class VideoBuffer;
class Window_
{
public:
	Window_(Point position, Point size);
	~Window_();

	void AddComponent(Component *other);
	void RemoveComponent(Component *other);
	void FocusComponent(Component *toFocus);
	void UpdateComponents();
	bool IsFocused(const Component *other) const { return other == focused; }
	bool IsClicked(const Component *other) const { return other == clicked; }

	void DoTick(float dt);
	void DoDraw();

	virtual void DoMouseMove(int x, int y, int dx, int dy);
	virtual void DoMouseDown(int x, int y, unsigned char button);
	virtual void DoMouseUp(int x, int y, unsigned char button);
	virtual void DoMouseWheel(int x, int y, int d);
	virtual void DoKeyPress(int key, unsigned short character, unsigned char modifiers);
	virtual void DoKeyRelease(int key, unsigned short character, unsigned char modifiers);

	Point GetPosition() { return position; }
	Point GetSize() { return size; }
	VideoBuffer* GetVid() { return videoBuffer; }
	bool IsMouseDown() { return isMouseDown; }

	bool toDelete;

	static const int CENTERED = -1;

protected:
	Point position;
	Point size;
	std::vector<Component*> Components;
	bool isMouseDown; // need to keep track of this for some things like buttons

	virtual void OnTick(float dt) { }
	virtual void OnDraw(VideoBuffer *buf) { }
	virtual void OnMouseMove(int x, int y, Point difference) { }
	virtual void OnMouseDown(int x, int y, unsigned char button) { }
	virtual void OnMouseUp(int x, int y, unsigned char button) { }
	virtual void OnMouseWheel(int x, int y, int d) { }
	virtual void OnKeyPress(int key, unsigned short character, unsigned char modifiers) { }
	virtual void OnKeyRelease(int key, unsigned short character, unsigned char modifiers) { }

private:
	VideoBuffer* videoBuffer;
	Component* focused;
	Component* clicked;
};

#endif
