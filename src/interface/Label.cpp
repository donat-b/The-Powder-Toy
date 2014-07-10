
#include "Label.h"
#include "graphics.h"
#include "misc.h"

Label::Label(std::string text, Point position, Point size, bool multiline):
	text(text),
	position(position),
	size(size),
	multiline(multiline),
	cursor(0),
	cursorStart(0),
	lastClick(0),
	numClicks(0)
{
	UpdateDisplayText();
}

//ugly but all-in-one function that updates displayText with \n and \x01 and updates cursor position
void Label::UpdateDisplayText(bool setHighlight, int mx, int my)
{
	int posX = 0, posY = 12, wordStart = 0, sub = 0, cursorPos;
	std::string tempText = text;
	bool updatedCursor = false;
	if (setHighlight)
		tempText.insert(cursorStart, "\x01");
	if (my < 0)
	{
		cursorPos = cursor = 0;
		updatedCursor = true;
	}
	for (int i = 0; i < tempText.length();)
	{
		int wordlen = tempText.substr(i).find_first_of(" .,!?\n");
		if (wordlen == -1)
			wordlen = tempText.length();
		if (wordlen && posX > size.X*2/3)
			wordStart = i;
		else
			wordStart = 0;

		for (; --wordlen>=-1 && i < tempText.length(); i++)
		{
			switch (tempText[i])
			{
			case '\n':
				posX = 0;
				posY += 12;
				wordStart = 0;
				if (!updatedCursor && ((posX >= mx+3 && posY >= my) || (posY >= my+12)))
				{
					cursorPos = i;
					cursor = i-sub;
					updatedCursor = true;
				}
				break;
			case '\x0F':
				i += 3;
				wordlen -= 3;
				break;
			case '\b':
				i++;
				wordlen--;
			case '\x0E':
			case '\x01':
				break;
			default:
				posX += charwidth(tempText[i]);
				if (posX >= size.X)
				{
					int replacePos = i;
					if (wordStart)
					{
						replacePos = wordStart;
						i = wordStart;
						wordStart = 0;
					}
					if (tempText[replacePos] == ' ')
						tempText[replacePos] = '\n';
					else
						tempText.insert(replacePos, "\n");
					sub++;
					wordlen++;
					posX = 0;
					posY += 12;
				}
				if (!updatedCursor && !wordStart && ((posX >= mx+3 && posY >= my) || (posY >= my+12)))
				{
					cursorPos = i;
					cursor = i-sub+(posX==0);
					updatedCursor = true;
				}
				break;
			}
		}
	}
	if (!updatedCursor)
	{
		cursorPos = tempText.length();
		cursor = tempText.length()-sub;
	}
	if (setHighlight)
	{
		if (cursor >= cursorStart)
		{
			cursor--;
		}
		tempText.insert(cursorPos, "\x01");
	}
	if (multiline)
		size.Y = posY+2;
	displayText = tempText;
}

void Label::OnMouseDown(int x, int y, unsigned char button)
{
	if (button == 1)
	{
		UpdateDisplayText(false, x, y);
		cursorStart = cursor;

		numClicks++;
		lastClick = SDL_GetTicks();
	}
}

void Label::OnMouseUp(int x, int y, unsigned char button)
{
	if (button == 1)
	{
		UpdateDisplayText(true, x, y);
	}
}

void Label::OnMouseMoved(int x, int y, Point difference)
{
	if (clicked)
	{
		UpdateDisplayText(true, x, y);
	}
}

int lastx = 0;
int lasty = 0;
void Label::OnKeyPress(int key, unsigned short character, unsigned char modifiers)
{
	if (modifiers&KMOD_CTRL && character == 'c')
	{
		int start = cursor > cursorStart ? cursorStart : cursor;
		int len = abs(cursor-cursorStart);
		clipboard_push_text((char*)text.substr(start, len).c_str());
	}
}

void Label::OnDraw(pixel* vid_buf)
{
	drawtext(vid_buf, position.X, position.Y, displayText.c_str(), 255, 255, 255, 100);//focus?255:100);
}

void Label::OnTick()
{
	if (numClicks && lastClick > SDL_GetTicks()+300)
		numClicks = 0;
	//if (focus)
		//cursor = textposxy(text.c_str(), size.X, x, y);
}

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
				label.clicked = true;
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
		label.clicked = false;
	}
	if (lastx != x || lasty != y)
		label.OnMouseMoved(posX, posY, Point(x-lastx, y-lasty));
	lastx = x;
	lasty = y;
	if (sdl_key)
		label.OnKeyPress(sdl_key, sdl_key, sdl_mod);
	label.OnTick();
}
