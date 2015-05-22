#ifdef NEWINTERFACE
#include "Window.h"
#include "Engine.h"
#include "Label.h"
#include "Textbox.h"
#include "graphics.h"
#include "graphics/VideoBuffer.h"

Window_::Window_(Point position_, Point size_):
	position(position_),
	size(size_),
	Components(NULL),
	focused(NULL),
	clicked(NULL),
	toDelete(false)
{
	if (position.X == CENTERED)
		position.X = (XRES+BARSIZE-size.X)/2;
	if (position.Y == CENTERED)
		position.Y = (YRES+MENUSIZE-size.Y)/2;
	videoBuffer = new VideoBuffer(size.X, size.Y);
	//AddComponent(new Textbox(Point(5, 5), Point(100, 14), "asdf", false));
}

Window_::~Window_()
{
	delete videoBuffer;
	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
		delete *iter;
	Components.clear();
}

void Window_::AddComponent(Component *other)
{
	//Maybe should do something if this component is already part of another window
	Components.push_back(other);
	other->SetParent(this);
}

void Window_::RemoveComponent(Component *other)
{
	for (std::vector<Component*>::iterator iter = Components.end()-1, end = Components.begin(); iter != end; iter--)
	{
		if ((*iter) == other)
		{
			(*iter)->SetParent(NULL);
			Components.erase(iter);
		}
	}
}

void Window_::FocusComponent(Component *toFocus)
{
	if (focused)
		focused->OnDefocus();
	focused = toFocus;
}

void Window_::DoTick(float dt)
{
	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		(*iter)->OnTick();
	}

	OnTick(dt);
}

void Window_::DoDraw()
{
	videoBuffer->Clear();
	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		(*iter)->OnDraw(videoBuffer);
	}

	OnDraw(videoBuffer);

	videoBuffer->CopyVideoBuffer(&vid_buf, position.X, position.Y);
	drawrect(vid_buf, position.X, position.Y, size.X, size.Y, 255, 255, 255, 255);
}

void Window_::DoMouseMove(int x, int y, int dx, int dy)
{
	if (dx || dy)
	{
		for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
		{
			Component *temp = *iter;
			int posX = x-this->position.X-temp->GetPosition().X, posY = y-this->position.Y-temp->GetPosition().Y;
			temp->OnMouseMoved(posX, posY, Point(dx, dy));
		}
	}

	OnMouseMove(x, y, Point(dx, dy));
}

void Window_::DoMouseDown(int x, int y, unsigned char button)
{
	if (x < position.X || x > position.X+size.X || y < position.Y || y > position.Y+size.Y)
	{
		toDelete = true;
	}

	bool focusedSomething = false;
	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		Component *temp = *iter;
		int posX = x-this->position.X-temp->GetPosition().X, posY = y-this->position.Y-temp->GetPosition().Y;
		bool inside = posX >= 0 && posX < temp->GetSize().X && posY >= 0 && posY < temp->GetSize().Y;
		if (inside)
		{
			focusedSomething = true;
			FocusComponent(temp);
			clicked = temp;
			temp->OnMouseDown(posX, posY, button);
			break;
		}
	}
	if (!focusedSomething)
		FocusComponent(NULL);

	OnMouseDown(x, y, button);
}

void Window_::DoMouseUp(int x, int y, unsigned char button)
{
	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		Component *temp = *iter;
		int posX = x-this->position.X-temp->GetPosition().X, posY = y-this->position.Y-temp->GetPosition().Y;
		bool inside = posX >= 0 && posX < temp->GetSize().X && posY >= 0 && posY < temp->GetSize().Y;

		if (inside || IsClicked(temp))
			temp->OnMouseUp(posX, posY, button);
	}
	clicked = NULL;

	OnMouseUp(x, y, button);
}

void Window_::DoMouseWheel(int x, int y, int d)
{
	/*for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		(*iter)->OnMouseWheel(x, y, d);
	}*/

	OnMouseWheel(x, y, d);
}

void Window_::DoKeyPress(int key, unsigned short character, unsigned char modifiers)
{
	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		if (IsFocused(*iter))
			(*iter)->OnKeyPress(key, character, modifiers);
	}

	OnKeyPress(key, character, modifiers);
}

void Window_::DoKeyRelease(int key, unsigned short character, unsigned char modifiers)
{
	/*for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		if (IsFocused(*iter))
			(*iter)->OnKeyRelease(key, character, modifiers);
	}*/

	OnKeyRelease(key, character, modifiers);
}

#endif
