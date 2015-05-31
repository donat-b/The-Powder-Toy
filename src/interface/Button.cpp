#include "Button.h"
#include "Window.h"
#include "graphics/VideoBuffer.h"

Button::Button(Point position, Point size, std::string text_):
	Component(position, size),
	text(text_)
{

}

void Button::OnMouseDown(int x, int y, unsigned char button)
{

}

void Button::OnMouseUp(int x, int y, unsigned char button)
{

}

void Button::OnMouseMoved(int x, int y, Point difference)
{

}

void Button::OnKeyPress(int key, unsigned short character, unsigned char modifiers)
{
	
}

void Button::OnDraw(VideoBuffer* vid)
{
	if (!isMouseInside || (IsMouseDown() && !IsClicked()))
	{
		// Mouse not inside button, or over button but click did not start on button
		vid->DrawText(position.X+2, position.Y+3, text.c_str(), 255, 255, 255, 255);
	}
	else
	{
		if (IsClicked())
		{
			// Button held down
			vid->FillRect(position.X, position.Y, size.X, size.Y, 255, 255, 255, 255);
			vid->DrawText(position.X+2, position.Y+3, text.c_str(), 0, 0, 0, 255);
		}
		else
		{
			// Mouse over button, not held down
			vid->FillRect(position.X, position.Y, size.X, size.Y, 255, 255, 255, 100);
			vid->DrawText(position.X+2, position.Y+3, text.c_str(), 255, 255, 255, 255);
		}
	}

	//if (IsFocused())
		vid->DrawRect(position.X-1, position.Y-1, size.X+2, size.Y+2, 255, 255, 255, 255);
	//else
	//	vid->DrawRect(position.X-1, position.Y-1, size.X+2, size.Y+2, 150, 150, 150, 255);
}

void Button::OnTick()
{

}
