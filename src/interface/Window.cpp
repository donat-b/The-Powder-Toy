#include "Window.h"
#include "Engine.h"
#include "Label.h"
#include "Textbox.h"
#include "graphics.h"
#include "graphics/VideoBuffer.h"

Window_::Window_(Point position_, Point size_):
	toDelete(false),
	position(position_),
	size(size_),
	Components(NULL),
	isMouseDown(false),
	ignoreQuits(false),
	mouseDownOutside(false),
	focused(NULL),
	clicked(NULL)
{
	if (position.X == CENTERED)
		position.X = (XRES+BARSIZE-size.X)/2;
	if (position.Y == CENTERED)
		position.Y = (YRES+MENUSIZE-size.Y)/2;
	videoBuffer = new VideoBuffer(size.X, size.Y);
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
	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		// this component is already part of the window
		if ((*iter) == other)
		{
			return;
		}
	}
	Components.push_back(other);
	other->SetParent(this);
	other->SetVisible(false);
	other->toAdd = true;
}

void Window_::RemoveComponent(Component *other)
{
	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		if ((*iter) == other)
		{
			(*iter)->toDelete = true;
		}
	}
	if (other == focused)
		FocusComponent(NULL);
	if (other == clicked)
		clicked = NULL;
}

void Window_::FocusComponent(Component *toFocus)
{
	if (focused != toFocus)
	{
		if (focused)
			focused->OnDefocus();
		focused = toFocus;
		if (focused)
			focused->OnFocus();
	}
}

void Window_::UpdateComponents()
{
	for (std::vector<Component*>::iterator iter = Components.end()-1, end = Components.begin()-1; iter != end; iter--)
	{
		Component *c = *iter;
		if (c->toDelete)
		{
			c->SetParent(NULL);
			Components.erase(iter);
			delete c;
		}
		else if (c->toAdd)
		{
			c->SetVisible(true);
		}
	}
}

void Window_::DoExit()
{
	OnExit();
}

void Window_::DoFocus()
{
	OnFocus();
}

void Window_::DoDefocus()
{
	OnDefocus();
}

void Window_::DoTick(uint32_t ticks)
{
	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		if ((*iter)->IsVisible())
			(*iter)->OnTick(ticks);
	}

	OnTick(ticks);
}

void Window_::DoDraw()
{
	// too lazy to create another variable which is temporary anyway
	if (!ignoreQuits)
		videoBuffer->Clear();
	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		if ((*iter)->IsVisible() && !IsFocused(*iter))
			(*iter)->OnDraw(videoBuffer);
	}
	// draw the focused component on top
	if (focused)
		focused->OnDraw(videoBuffer);

	OnDraw(videoBuffer);

	videoBuffer->CopyVideoBuffer(vid_buf, XRES+BARSIZE, position.X, position.Y);
	// TODO: use videobuffer function for this ...
	if (position.X > 0 || position.Y > 0 || size.X < XRES+BARSIZE-1 || size.Y < YRES+MENUSIZE-1)
		drawrect(vid_buf, position.X, position.Y, size.X, size.Y, 255, 255, 255, 255);
}

void Window_::DoMouseMove(int x, int y, int dx, int dy)
{
	if (!BeforeMouseMove(x, y, Point(dx, dy)))
		return;

	bool alreadyInside = false;
	if (dx || dy)
	{
		for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
		{
			Component *temp = *iter;
			if (temp->IsVisible() && temp->IsEnabled())
			{
				Component *temp = *iter;
				int posX = x-this->position.X-temp->GetPosition().X, posY = y-this->position.Y-temp->GetPosition().Y;
				// update isMouseInside for this component
				if (!alreadyInside && posX >= 0 && posX < temp->GetSize().X && posY >= 0 && posY < temp->GetSize().Y)
				{
					temp->SetMouseInside(true);
					alreadyInside = true;
				}
				else
				{
					temp->SetMouseInside(false);
					if (temp == clicked)
						clicked = NULL;
				}
				temp->OnMouseMoved(posX, posY, Point(dx, dy));
			}
		}
	}

	OnMouseMove(x, y, Point(dx, dy));
}

void Window_::DoMouseDown(int x, int y, unsigned char button)
{
	if (!BeforeMouseDown(x, y, button))
		return;

#ifndef TOUCHUI
	if (x < position.X || x > position.X+size.X || y < position.Y || y > position.Y+size.Y)
	{
		mouseDownOutside = true;
	}
#endif
	isMouseDown = true;

	bool focusedSomething = false;
	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		Component *temp = *iter;
		if (temp->IsVisible() && temp->IsEnabled())
		{
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
	}
	if (!focusedSomething)
		FocusComponent(NULL);

	OnMouseDown(x, y, button);
}

void Window_::DoMouseUp(int x, int y, unsigned char button)
{
	if (!BeforeMouseUp(x, y, button))
		return;

#ifndef TOUCHUI
	if (mouseDownOutside)
	{
		mouseDownOutside = false;
		if (x < position.X || x > position.X+size.X || y < position.Y || y > position.Y+size.Y)
		{
			toDelete = true;
		}
	}
#endif
	isMouseDown = false;
	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		Component *temp = *iter;
		if (temp->IsVisible() && temp->IsEnabled())
		{
			int posX = x-this->position.X-temp->GetPosition().X, posY = y-this->position.Y-temp->GetPosition().Y;
			bool inside = posX >= 0 && posX < temp->GetSize().X && posY >= 0 && posY < temp->GetSize().Y;

			if (inside || IsClicked(temp))
				temp->OnMouseUp(posX, posY, button);
		}
	}
	clicked = NULL;

	OnMouseUp(x, y, button);
}

void Window_::DoMouseWheel(int x, int y, int d)
{
	if (!BeforeMouseWheel(x, y, d))
		return;

	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		if ((*iter)->IsVisible() && (*iter)->IsEnabled())
			(*iter)->OnMouseWheel(x, y, d);
	}

	OnMouseWheel(x, y, d);
}

void Window_::DoKeyPress(int key, unsigned short character, unsigned short modifiers)
{
	if (!BeforeKeyPress(key, character, modifiers))
		return;

	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		if (IsFocused(*iter) && (*iter)->IsVisible() && (*iter)->IsEnabled())
			(*iter)->OnKeyPress(key, character, modifiers);
	}

	OnKeyPress(key, character, modifiers);
}

void Window_::DoKeyRelease(int key, unsigned short character, unsigned short modifiers)
{
	if (!BeforeKeyRelease(key, character, modifiers))
		return;

	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		if (IsFocused(*iter) && (*iter)->IsVisible() && (*iter)->IsEnabled())
			(*iter)->OnKeyRelease(key, character, modifiers);
	}

	OnKeyRelease(key, character, modifiers);
}

void Window_::VideoBufferHack()
{
	std::copy(&vid_buf[0], &vid_buf[(XRES+BARSIZE)*(YRES+MENUSIZE)], &videoBuffer->GetVid()[0]);
}
