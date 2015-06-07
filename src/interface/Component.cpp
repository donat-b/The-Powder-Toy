#include <cstddef> //needed for NULL apparently ...
#include "Component.h"
#include "Window.h"

Component::Component(Point position, Point size) :
	parent(NULL),
	position(position),
	size(size),
	visible(true),
	enabled(true),
	toDelete(false),
	toAdd(false)
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

bool Component::IsMouseDown()
{
	if (parent)
		return parent->IsMouseDown();
	return false;
}
