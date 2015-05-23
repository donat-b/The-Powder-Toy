#include "ScrollWindow.h"

ScrollWindow::ScrollWindow(Point position, Point size):
	Window_(position, size),
	scrollable(false),
	scrollSize(size.Y),
	scrolled(0)
{
}

void ScrollWindow::DoMouseWheel(int x, int y, int d)
{
	if (scrollable)
	{
		int oldScrolled = scrolled;
		if (d > 0 && scrolled > 0)
			scrolled = std::max(scrolled-d*4, 0);
		else if (d < 0 && scrolled < size.Y + scrollSize)
			scrolled = std::min(scrolled-d*4, size.Y + scrollSize);

		for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
		{
			(*iter)->position.Y -= (scrolled-oldScrolled);
		}
	}

	/*for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		(*iter)->OnMouseWheel(x, y, d);
	}*/

	OnMouseWheel(x, y, d);
}

void ScrollWindow::SetScrollable(bool scroll, int maxScroll)
{
	scrollable = scroll;
	scrollSize = maxScroll;
}
