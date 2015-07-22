#include "ScrollWindow.h"

ScrollWindow::ScrollWindow(Point position, Point size):
	Window_(position, size),
	scrollable(false),
	scrollSize(size.Y),
	scrolled(0),
	lastMouseX(0),
	lastMouseY(0)
{
}

void ScrollWindow::DoMouseWheel(int x, int y, int d)
{
	if (scrollable)
	{
		lastMouseX = x;
		lastMouseY = y;
		if (d > 0 && scrolled > 0)
			SetScrollPosition(std::max(scrolled-d*4, 0));
		else if (d < 0 && scrolled < scrollSize)
			SetScrollPosition(std::min(scrolled-d*4, scrollSize));
	}

	/*for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		(*iter)->OnMouseWheel(x, y, d);
	}*/

	OnMouseWheel(x, y, d);
}

void ScrollWindow::SetScrollable(bool scroll, int maxScroll)
{
	if (maxScroll < 0)
		return;
	scrollable = scroll;
	scrollSize = maxScroll;
	if (!scroll)
		SetScrollPosition(0);
	else if (scrolled > scrollSize)
		SetScrollPosition(scrollSize);
}

void ScrollWindow::SetScrollPosition(int pos)
{
	bool alreadyInside = false;
	int oldScrolled = scrolled;
	scrolled = pos;
	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		Component *temp = *iter;
		temp->SetPosition(Point(temp->GetPosition().X, temp->GetPosition().Y-(scrolled-oldScrolled)));

		int posX = lastMouseX-this->position.X-temp->GetPosition().X, posY = lastMouseY-this->position.Y-temp->GetPosition().Y;
		// update isMouseInside for this component
		if (!alreadyInside && posX >= 0 && posX < temp->GetSize().X && posY >= 0 && posY < temp->GetSize().Y)
		{
			temp->SetMouseInside(true);
			alreadyInside = true;
		}
		else
			temp->SetMouseInside(false);
	}
}
