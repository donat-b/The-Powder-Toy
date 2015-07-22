#include "Button.h"
#include "graphics/VideoBuffer.h"

Button::Button(Point position, Point size, std::string text_):
	Component(position, size),
	text(text_),
	callback(NULL),
	inverted(false),
	alignment(CENTER)
{

}

Button::~Button()
{
	delete callback;
}

void Button::SetCallback(ButtonAction *callback_)
{
	delete callback;
	callback = callback_;
}

void Button::OnMouseDown(int x, int y, unsigned char button)
{

}

void Button::OnMouseUp(int x, int y, unsigned char button)
{
	if (IsClicked() && isMouseInside && enabled && callback)
		callback->ButtionActionCallback(this, button);
}

void Button::OnMouseMoved(int x, int y, Point difference)
{

}

void Button::OnKeyPress(int key, unsigned short character, unsigned char modifiers)
{
	
}

void Button::OnDraw(VideoBuffer* vid)
{
	// border
	if (enabled)
		vid->DrawRect(position.X, position.Y, size.X, size.Y, COLR(color), COLG(color), COLB(color), 255);
	else
		vid->DrawRect(position.X, position.Y, size.X, size.Y, COLR(color)*.55f, COLG(color)*.55f, COLB(color)*.55f, 255);

	ARGBColour textColor;
	if (!enabled)
	{
		textColor = COLRGB((int)(COLR(color)*.55f), (int)(COLG(color)*.55f), (int)(COLB(color)*.55f));
	}
	// Mouse not inside button, or over button but click did not start on button
	else if (!isMouseInside || (IsMouseDown() && !IsClicked()))
	{
		if (inverted)
		{
			textColor = COLRGB(255-COLR(color), 255-COLG(color), 255-COLB(color));
			vid->FillRect(position.X, position.Y, size.X, size.Y, COLR(color), COLG(color), COLB(color), 255);
		}
		else
			textColor = color;
	}
	else
	{
		// Button held down
		if (IsClicked())
		{
			if (inverted)
			{
				textColor = color;
			}
			else
			{
				vid->FillRect(position.X, position.Y, size.X, size.Y, COLR(color), COLG(color), COLB(color), 255);
				textColor = COLPACK(0x000000);
			}
		}
		// Mouse over button, not held down
		else
		{
			if (inverted)
			{
				vid->FillRect(position.X, position.Y, size.X, size.Y, COLR(color), COLG(color), COLB(color), 155);
				textColor = color;
			}
			else
			{
				vid->FillRect(position.X, position.Y, size.X, size.Y, COLR(color), COLG(color), COLB(color), 100);
				textColor = color;
			}
		}
	}
	if (alignment == LEFT)
		vid->DrawText(position.X+2, position.Y+4, text, COLR(textColor), COLG(textColor), COLB(textColor), COLA(textColor));
	else
	{
		int textWidth = VideoBuffer::TextSize(text).X;
		vid->DrawText(position.X+(size.X-textWidth)/2+1, position.Y+4, text, COLR(textColor), COLG(textColor), COLB(textColor), COLA(textColor));
	}
}

void Button::OnTick(uint32_t ticks)
{

}
