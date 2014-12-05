#ifdef NEWINTERFACE
#include "Window.h"
#include "Label.h"
#include "Textbox.h"
#include "graphics.h"
#include "graphics/VideoBuffer.h"

Window_::Window_(Point position, Point size):
	position(position),
	size(size),
	Components(NULL),
	focused(NULL)
{
	videoBuffer = new VideoBuffer(size.X, size.Y);
	AddComponent(new Textbox(Point(5, 5), Point(100, 14), "asdf", false));
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

void Window_::DoTick(float dt)
{
	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		(*iter)->OnTick();
	}
}

void Window_::DoDraw()
{
	videoBuffer->Clear();
	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		(*iter)->OnDraw(videoBuffer);
	}
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
}

void Window_::DoMouseDown(int x, int y, unsigned char button)
{
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
		}
	}
	if (!focusedSomething)
		FocusComponent(NULL);
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
}

void Window_::DoMouseWheel(int x, int y, int d)
{

}

void Window_::DoKeyPress(int key, unsigned short character, unsigned char modifiers)
{
	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		if (IsFocused(*iter))
			(*iter)->OnKeyPress(key, character, modifiers);
	}
}

void Window_::DoKeyRelease(int key, unsigned short character, unsigned char modifiers)
{

}

#endif
