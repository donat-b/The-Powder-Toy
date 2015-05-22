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
