#ifdef NEWINTERFACE
#include "Window.h"
#include "Label.h"
#include "Textbox.h"
#include "graphics.h"
#include "graphics/VideoBuffer.h"

Label *moo;

Window_::Window_(Point position, Point size):
	position(position),
	size(size)
{
	videoBuffer = new VideoBuffer(size.X, size.Y);
	//moo = new Textbox("", Point(5,5), Point(100,14), true);
	moo = new Label("1234567890123456\n\n\n78901234567890 1234\n\n5678.90 12345678\bt9012345 6789\x0F\xFF\x03\xFF.01234567890123\bo4567890123456789\x0F.0", Point(5,5), Point(100,14), true);
	//moo = new Label("1234567890123456\n\n\n78901234567890 1234\n\n5678.90 123456789012345 67890.12345678901234567890123456789.0", Point(100,100), Point(100,14), true);
	//moo = new Label("1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", Point(100,100), Point(100,14), true);
}

Window_::~Window_()
{
	delete videoBuffer;
	delete moo;
}

void Window_::DoTick(float dt)
{
	moo->OnTick();
}

void Window_::DoDraw()
{
	videoBuffer->Clear();
	moo->OnDraw(videoBuffer);
	videoBuffer->CopyVideoBuffer(&vid_buf, position.X, position.Y);
	drawrect(vid_buf, position.X, position.Y, size.X, size.Y, 255, 255, 255, 255);
}

void Window_::DoMouseMove(int x, int y, int dx, int dy)
{
	if (dx || dy)
	{
		int posX = x-this->position.X-moo->GetPosition().X, posY = y-this->position.Y-moo->GetPosition().Y;
		moo->OnMouseMoved(posX, posY, Point(dx, dy));
	}
}

void Window_::DoMouseDown(int x, int y, unsigned char button)
{
	int posX = x-this->position.X-moo->GetPosition().X, posY = y-this->position.Y-moo->GetPosition().Y;
	bool inside = posX >= 0 && posX < this->size.X && posY >= 0 && posY < this->size.Y;

	if (inside && !moo->focus)
		moo->OnMouseDown(posX, posY, button);
}

void Window_::DoMouseUp(int x, int y, unsigned char button)
{
	int posX = x-this->position.X-moo->GetPosition().X, posY = y-this->position.Y-moo->GetPosition().Y;
	bool inside = posX >= 0 && posX < this->size.X && posY >= 0 && posY < this->size.Y;

	if (inside || moo->focus)
		moo->OnMouseUp(posX, posY, button);
}

void Window_::DoMouseWheel(int x, int y, int d)
{

}

void Window_::DoKeyPress(int key, unsigned short character, unsigned char modifiers)
{
	moo->OnKeyPress(key, character, modifiers);
}

void Window_::DoKeyRelease(int key, unsigned short character, unsigned char modifiers)
{

}

/*int lastx, lasty;
void InterfaceConvert(Label &label, int x, int y, int b, int bq, int sdl_key, unsigned char sdl_mod)
{
	int posX = x-label.position.X, posY = y-label.position.Y;
	bool inside = posX >= 0 && posX < label.size.X && posY >= 0 && posY < label.size.Y;

	if (b && !bq)
	{
		if (inside)
		{
			label.focus = true;
			if (!label.clicked)
			{
				label.OnMouseDown(posX, posY, b);
			}
		}
		else
			label.focus = false;
	}
	else if (bq && !b)
	{
		if (inside || label.clicked)
			label.OnMouseUp(posX, posY, bq);
	}
	if (lastx != x || lasty != y)
		label.OnMouseMoved(posX, posY, Point(x-lastx, y-lasty));
	lastx = x;
	lasty = y;
	if (sdl_key)
		label.OnKeyPress(sdl_key, sdl_key, sdl_mod);
	label.OnTick();
}*/
#endif
