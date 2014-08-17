#ifdef POTATO
#include "Label.h"
#include "graphics.h"
#include "misc.h"
#include <algorithm>

Label::Label(std::string text, Point position, Point size, bool multiline):
	text(text),
	position(position),
	size(size),
	multiline(multiline),
	cursor(0),
	cursorStart(0),
	clickPosition(0),
	lastClick(0),
	numClicks(0)
{
	UpdateDisplayText();
}

void FindWordPosition(const char *s, int position, int *cursorStart, int *cursorEnd, const char* spaces)
{
	int wordLength = 0, totalLength = 0, strLength = strlen(s);
	while (totalLength < strLength)
	{
		wordLength = strcspn(s, spaces);
		if (totalLength + wordLength >= position)
		{
			*cursorStart = totalLength;
			*cursorEnd = totalLength+wordLength;
			return;
		}
		s += wordLength+1;
		totalLength += wordLength+1;
	}
	*cursorStart = totalLength;
	*cursorEnd = totalLength+wordLength;
}

int Label::UpdateCursor(int position)
{
	if (numClicks >= 2)
	{
		int start = 0, end = displayText.length();
		const char *spaces = " .,!?\n";
		if (numClicks == 3)
			spaces = "\n";
		FindWordPosition(displayText.c_str(), position, &start, &end, spaces);
		if (start < clickPosition)
		{
			cursorStart = start;
			FindWordPosition(displayText.c_str(), clickPosition, &start, &end, spaces);
			cursor = end;
		}
		else
		{
			cursorStart = end;
			FindWordPosition(displayText.c_str(), clickPosition, &start, &end, spaces);
			cursor = start;
		}
	}
	return position;
}

//all-in-one function that updates displayText with \r and \x01, updates cursor position, and cuts string where needed
void Label::UpdateDisplayText(int mx, int my, bool firstClick)
{
	int posX = 0, posY = 12, wordStart = 0;
	bool updatedCursor = false;
	displayText = text;

	if (my < 0)
	{
		cursor = 0;
		updatedCursor = true;
	}
	for (int i = 0; i < displayText.length();)
	{
		//find end of word/line chars
		int wordlen = displayText.substr(i).find_first_of(" .,!?\n");
		if (wordlen == -1)
			wordlen = displayText.length();

		//if a word starts in the last 1/3 of the line, it will get put on the next line if it's too long
		if (wordlen && posX > size.X*2/3)
			wordStart = i;
		else
			wordStart = 0;

		//loop through a word
		for (; --wordlen>=-1 && i < displayText.length(); i++)
		{
			switch (displayText[i])
			{
			case '\n':
				if (multiline)
				{
					posX = 0;
					posY += 12;
					wordStart = 0;
					if (!updatedCursor && ((posX >= mx+3 && posY >= my) || (posY >= my+12)))
					{
						cursor = i;
						updatedCursor = true;
					}
				}
				else
				{
					displayText = displayText.substr(0, i);
					displayText.insert(i, "...");
					i += 3;
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
			case '\r':
				break;
			default:
				//normal character, add to the current width and check if it's too long
				posX += charwidth(displayText[i]);
				if (posX >= size.X)
				{
					if (multiline)
					{
						//if it's too long, figure out where to put the newline (either current position or remembered word start position)
						int replacePos = i;
						if (wordStart)
						{
							replacePos = wordStart;
							i = wordStart;
							wordStart = 0;
						}

						//use \r instead of \n, that way it can be easily filtered out when copying without ugly hacks
						displayText.insert(replacePos, "\r");
						wordlen++;
						posX = 0;
						posY += 12;
					}
					else
					{
						//for non multiline strings, cut it off here and delete the rest of the string
						displayText = displayText.substr(0, i-1 > 0 ? i-1 : 0);
						displayText.insert(i-1 > 0 ? i-1 : 0, "...");
						i += 2;
					}
				}
				//update cursor position, in multiple positions
				if (!updatedCursor && !wordStart && ((posX >= mx+3 && posY >= my) || (posY >= my+12)))
				{
					cursor = i+(posX==0);
					updatedCursor = true;
				}
				break;
			}
		}
	}
	//if cursor position wasn't found, probably goes at the end
	if (!updatedCursor)
		cursor = displayText.length();

	//update cursor and cursorStart and adjust for double / triple clicks
	if (firstClick)
		clickPosition = cursorStart = cursor;
	UpdateCursor(cursor);

	//insert \x01 where the cursor start / end are, to mark where to put highlight in drawtext
	if (cursor != cursorStart || numClicks > 1)
	{
		displayText.insert(cursorStart, "\x01");
		displayText.insert(cursor+(cursor>cursorStart), "\x01");
	}

	//multiline labels have their Y size set automatically
	if (multiline)
		size.Y = posY+2;
}

void Label::OnMouseDown(int x, int y, unsigned char button)
{
	if (button == 1)
	{
		clicked = true;
		numClicks++;
		lastClick = SDL_GetTicks();

		UpdateDisplayText(x, y, true);
		//cursorStart = cursor;
	}
}

void Label::OnMouseUp(int x, int y, unsigned char button)
{
	if (button == 1)
	{
		UpdateDisplayText(x, y);
		clicked = false;
	}
}

void Label::OnMouseMoved(int x, int y, Point difference)
{
	if (clicked)
	{
		UpdateDisplayText(x, y);
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
		std::string copyStr = displayText.substr(start+1, len);
		copyStr.erase(std::remove(copyStr.begin(), copyStr.end(), '\r'), copyStr.end());
		clipboard_push_text((char*)copyStr.c_str());
	}
}

void Label::OnDraw(pixel* vid_buf)
{
	drawtext(vid_buf, position.X, position.Y, displayText.c_str(), 255, 255, 255, 100);//focus?255:100);
}

void Label::OnTick()
{
	if (!clicked && numClicks && lastClick+300 < SDL_GetTicks())
		numClicks = 0;
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
}
#endif
