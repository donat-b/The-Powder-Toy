#ifndef BUTTON_H
#define BUTTON_H

#include <string>
#include "common/Point.h"
#include "Component.h"

class VideoBuffer;
class Button;
class ButtonAction
{
public:
	ButtonAction() { }
	virtual void ButtionActionCallback(Button *button) { }
};

class Button : public Component
{
private:
	std::string text;
	ButtonAction *callback;

public:
	Button(Point position, Point size, std::string text_);

	void SetCallback(ButtonAction *callback_);

	virtual void OnMouseDown(int x, int y, unsigned char button);
	virtual void OnMouseUp(int x, int y, unsigned char button);
	virtual void OnMouseMoved(int x, int y, Point difference);
	virtual void OnKeyPress(int key, unsigned short character, unsigned char modifiers);
	virtual void OnDraw(VideoBuffer* vid);
	virtual void OnTick();
};

#endif
