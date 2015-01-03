#include <cstddef> //needed for NULL apparently ...
#include "Component.h"
#include "Window.h"

Component::Component(Point position, Point size) :
	parent(NULL),
	position(position),
	size(size)
{

}

bool Component::IsFocused()
{
	if (parent)
		return parent->IsFocused(this);
	return false;
}

bool Component::IsClicked()
{
	if (parent)
		return parent->IsClicked(this);
	return false;
}

void Component::OnMouseDown(int x, int y, unsigned char button)
{
	//Do nothing
}

void Component::OnMouseUp(int x, int y, unsigned char button)
{
	//Do nothing
}

void Component::OnMouseMoved(int x, int y, Point difference)
{
	//Do nothing
}

void Component::OnKeyPress(int key, unsigned short character, unsigned char modifiers)
{
	//Do nothing
}

void Component::OnDraw(VideoBuffer* vid)
{
	//Do nothing
}

void Component::OnTick()
{
	//Do nothing
}
