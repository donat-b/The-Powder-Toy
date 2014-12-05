#ifdef NEWINTERFACE
#include "Button.h"
#include "graphics.h"
#include "graphics/VideoBuffer.h"

Button::Button(Point position, Point size) :
position(position),
size(size)
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
	
}

void Button::OnTick()
{

}

#endif
