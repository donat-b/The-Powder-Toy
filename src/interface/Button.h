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
	virtual void ButtionActionCallback(Button *button, unsigned char b) { }
};

class Button : public Component
{
public:
	enum TextAlign { LEFT, CENTER };

private:
	std::string text;
	ButtonAction *callback;
	bool inverted;
	TextAlign alignment;

public:
	Button(Point position, Point size, std::string text_);
	virtual ~Button();

	void SetCallback(ButtonAction *callback_);
	void SetText(std::string text_);
	void SetAlign(TextAlign align) { alignment = align; }
	void Invert(bool invert) { inverted = invert; }

	virtual void OnMouseDown(int x, int y, unsigned char button);
	virtual void OnMouseUp(int x, int y, unsigned char button);
	virtual void OnMouseMoved(int x, int y, Point difference);
	virtual void OnKeyPress(int key, unsigned short character, unsigned char modifiers);
	virtual void OnDraw(VideoBuffer* vid);
	virtual void OnTick(uint32_t ticks);
};

#endif
