#include "Button.h"
#include "game/ToolTip.h"
#include "graphics/VideoBuffer.h"

Button::Button(Point position, Point size, std::string text_):
	Component(position, size),
	text(text_),
	tooltip(NULL),
	callback(NULL),
	alignment(CENTER),
	state(NORMAL),
	timeHeldDown(0)
{

}

Button::~Button()
{
	delete callback;
	delete tooltip;
}

void Button::SetText(std::string text_)
{
	text = text_;
	// ensure text isn't too big for button, maybe not too efficient
	if (VideoBuffer::TextSize(text)+Point(2, 0) >= size)
	{
		text += "...";
		while (text.length() > 3 && VideoBuffer::TextSize(text)+Point(2, 0) >= size)
		{
			text.erase(text.length()-4, 1);
		}
	}
}

void Button::SetTooltipText(std::string newTooltip)
{
	if (tooltip)
		tooltip->SetTip(newTooltip);
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
	{
		if (state == HOLD)
			callback->ButtionActionCallback(this, IsHeld() ? 4 : button);
		else
			callback->ButtionActionCallback(this, button);
	}
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
	ARGBColour backgroundColor = 0;
	if (!enabled)
	{
		if (state == INVERTED)
			backgroundColor = color;
		else if (state == HIGHLIGHTED)
			backgroundColor = COLARGB(100, COLR(color), COLG(color), COLB(color));
		textColor = COLRGB((int)(COLR(color)*.55f), (int)(COLG(color)*.55f), (int)(COLB(color)*.55f));
	}
#ifdef TOUCHUI
	// Mouse not inside button, Mouse not down, or over button but click did not start on button
	else if (!isMouseInside || !IsMouseDown() || (IsMouseDown() && !IsClicked()))
#else
	// Mouse not inside button, or over button but click did not start on button
	else if (!isMouseInside || (IsMouseDown() && !IsClicked()))
#endif
	{
		if (state == INVERTED)
		{
			textColor = COLRGB(255-COLR(color), 255-COLG(color), 255-COLB(color));
			backgroundColor = color;
		}
		else if (state == HIGHLIGHTED)
		{
			backgroundColor = COLARGB(100, COLR(color), COLG(color), COLB(color));
			textColor = color;
		}
		else
			textColor = color;
	}
	else
	{
		// button clicked and held down
		if (IsClicked())
		{
			if (state == INVERTED)
				textColor = color;
			else if (state == HOLD)
			{
				if (IsHeld())
				{
					textColor = COLPACK(0x000000);
					backgroundColor = COLARGB(255, COLR(color), COLG(color), COLB(color));
				}
				else
				{
					unsigned int heldAmount = std::min((int)(timeHeldDown/20), 100);
					textColor = color;
					backgroundColor = COLARGB(100+heldAmount, COLR(color), COLG(color), COLB(color));
				}
			}
			else
			{
				backgroundColor = color;
				textColor = COLPACK(0x000000);
			}
		}
		// Mouse over button, not held down
		else
		{
			if (state == INVERTED)
			{
				backgroundColor = COLARGB(200, COLR(color), COLG(color), COLB(color));
				textColor = COLRGB(255-COLR(color), 255-COLG(color), 255-COLB(color));
			}
			else if (state == HIGHLIGHTED)
			{
				backgroundColor = COLARGB(155, COLR(color), COLG(color), COLB(color));
				textColor = color;
			}
			else
			{
				backgroundColor = COLARGB(100, COLR(color), COLG(color), COLB(color));
				textColor = color;
			}
		}
	}
	// background color (if required)
	if (backgroundColor)
		vid->FillRect(position.X, position.Y, size.X, size.Y, COLR(backgroundColor), COLG(backgroundColor), COLB(backgroundColor), COLA(backgroundColor));

	if (alignment == LEFT)
	{
		Point textSize = VideoBuffer::TextSize(text);
		vid->DrawText(position.X+2, position.Y+(size.Y-textSize.Y)/2+1, text, COLR(textColor), COLG(textColor), COLB(textColor), COLA(textColor));
	}
	else
	{
		Point textSize = VideoBuffer::TextSize(text);
		vid->DrawText(position.X+(size.X-textSize.X)/2+1, position.Y+(size.Y-textSize.Y)/2+1, text, COLR(textColor), COLG(textColor), COLB(textColor), COLA(textColor));
	}
}

void Button::OnTick(uint32_t ticks)
{
#ifdef TOUCHUI
	if (isMouseInside && IsMouseDown() && tooltip)
#else
	if (isMouseInside && tooltip)
#endif
		tooltip->AddToScreen();

	if (state == HOLD)
	{
		if (IsClicked())
			timeHeldDown += ticks;
		else
			timeHeldDown = 0;
	}
}
