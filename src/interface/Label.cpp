#ifdef NEWINTERFACE
#include <algorithm>
#include "Label.h"
#include "graphics.h"
#include "misc.h"
#include "graphics/VideoBuffer.h"

Label::Label(std::string text, Point position, Point size, bool multiline):
	text(text),
	position(position),
	size(size),
	multiline(multiline),
	cursor(0),
	cursorStart(0),
	clickPosition(0),
	lastClick(0),
	numClicks(0),
	focus(false)
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
		int start = 0, end = text.length();
		const char *spaces = " .,!?\n";
		if (numClicks == 3)
			spaces = "\n";
		FindWordPosition(text.c_str(), position, &start, &end, spaces);
		if (start < clickPosition)
		{
			cursorStart = start;
			FindWordPosition(text.c_str(), clickPosition, &start, &end, spaces);
			cursor = end;
		}
		else
		{
			cursorStart = end;
			FindWordPosition(text.c_str(), clickPosition, &start, &end, spaces);
			cursor = start;
		}
	}
	return position;
}

//all-in-one function that updates displayText with \r, updates cursor position, and cuts string where needed
void Label::UpdateDisplayText(bool updateCursor, bool firstClick, int mx, int my)
{
	int posX = 0, posY = 12, wordStart = 0, cursorOffset = 0;
	bool updatedCursor = false;

	//get back the original string by removing the inserted newlines
	text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());

	if (updateCursor && my < -2)
	{
		cursor = 0;
		updatedCursor = true;
	}
	for (int i = 0; i < text.length();)
	{
		//find end of word/line chars
		int wordlen = text.substr(i).find_first_of(" .,!?\n");
		if (wordlen == -1)
			wordlen = text.length();

		//if a word starts in the last 1/3 of the line, it will get put on the next line if it's too long
		if (wordlen && posX > size.X*2/3)
			wordStart = i;
		else
			wordStart = 0;

		//loop through a word
		for (; --wordlen>=-1 && i < text.length(); i++)
		{
			switch (text[i])
			{
			case '\n':
				if (multiline)
				{
					posX = 0;
					posY += 12;
					wordStart = 0;
					//update cursor position (newline check)
					if (updateCursor && !updatedCursor && ((posX >= mx+3 && posY >= my+2) || (posY >= my+14)))
					{
						cursor = i;
						updatedCursor = true;
					}
				}
				else
				{
					text = text.substr(0, i);
					text.insert(i, "...");
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
				posX += charwidth(text[i]);
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
						text.insert(replacePos, "\r");
						wordlen++;
						if (!updatedCursor)
							cursorOffset++;
						posX = 0;
						posY += 12;
					}
					else
					{
						//for non multiline strings, cut it off here and delete the rest of the string
						text = text.substr(0, i-1 > 0 ? i-1 : 0);
						text.insert(i-1 > 0 ? i-1 : 0, "...");
						i += 2;
					}
				}
				//update cursor position
				if (updateCursor && !updatedCursor && ((posX >= mx+3 && posY >= my+2) || (posY >= my+14)))
				{
					cursor = i+(posX==0);
					updatedCursor = true;
				}
				break;
			}
		}
	}
	if (updateCursor)
	{
		//if cursor position wasn't found, probably goes at the end
		if (!updatedCursor)
			cursor = text.length();

		//update cursor and cursorStart and adjust for double / triple clicks
		if (firstClick)
			clickPosition = cursorStart = cursor;
		UpdateCursor(cursor);
	}
	//make sure cursor isn't out of bounds (for ctrl+a and shift+right)
	if (cursor > text.length())
		cursor = text.length();
	if (cursorStart > text.length())
		cursorStart = text.length();

	//multiline labels have their Y size set automatically
	if (multiline)
		size.Y = posY+2;
}

//this function moves the cursor a certain amount, and checks at the end for special characters to move past like \r, \b, and \0F
void Label::MoveCursor(int *cursor, int amount)
{
	int sign = isign(amount), offset = 0;
	if (!amount)
		return;

	offset += amount;
	//adjust for strange characters
	if (text[*cursor+offset-1] == '\b' || text[*cursor+offset-1] == '\r')
		offset += sign;
	else if (*cursor+offset-3+(sign>0)*2 > 0 && text[*cursor+offset-3+(sign>0)*2] == '\x0F') //when moving right, check 1 behind, when moving left, check 3 behind
		offset += sign*3;

	//make sure it's in bounds
	if (*cursor+offset < 0)
		offset = -*cursor;
	if (*cursor+offset > text.length())
		offset = text.length()-*cursor;

	*cursor += offset;
}

void Label::OnMouseDown(int x, int y, unsigned char button)
{
	if (button == 1)
	{
		focus = true;
		numClicks++;
		lastClick = SDL_GetTicks();

		UpdateDisplayText(true, true, x, y);
	}
}

void Label::OnMouseUp(int x, int y, unsigned char button)
{
	if (button == 1)
	{
		UpdateDisplayText(true, false, x, y);
		focus = false;
	}
}

void Label::OnMouseMoved(int x, int y, Point difference)
{
	if (focus)
	{
		UpdateDisplayText(true, false, x, y);
	}
}

int lastx = 0;
int lasty = 0;
void Label::OnKeyPress(int key, unsigned short character, unsigned char modifiers)
{
	if (modifiers&KMOD_CTRL)
	{
		switch (key)
		{
		case 'c':
		{
			int start = (cursor > cursorStart) ? cursorStart : cursor;
			int len = abs(cursor-cursorStart);
			std::string copyStr = text.substr(start, len);
			copyStr.erase(std::remove(copyStr.begin(), copyStr.end(), '\r'), copyStr.end()); //strip special newlines
			clipboard_push_text((char*)copyStr.c_str());
			break;
		}
		case 'a':
			//if (clicked)
			{
				cursorStart = 0;
				cursor = clickPosition = text.length();
			}
			break;
		case SDLK_LEFT:
		{
			int start, end;
			FindWordPosition(text.c_str(), cursor-1, &start, &end, " .,!?\n");
			MoveCursor(&cursor, start-cursor-(cursor > cursorStart));
			break;
		}
		case SDLK_RIGHT:
		{
			int start, end;
			FindWordPosition(text.c_str(), cursor+1, &start, &end, " .,!?\n");
			MoveCursor(&cursor, end-cursor+!(cursor > cursorStart));
			break;
		}
		}
	}
	if (modifiers&KMOD_SHIFT)
	{
		switch (key)
		{
		case SDLK_LEFT:
			if (cursor)
				MoveCursor(&cursor, -1);
			break;
		case SDLK_RIGHT:
			if (cursor < text.length())
				MoveCursor(&cursor, 1);
			break;
		}
	}
}

void Label::OnDraw(VideoBuffer* vid)
{
	//at some point there will be a redraw variable so this isn't done every frame
	std::string mootext = text;
	if (cursor != cursorStart || numClicks > 1)
	{
		mootext.insert(cursorStart, "\x01");
		mootext.insert(cursor+(cursor > cursorStart), "\x01");
	}
	if (ShowCursor())
	{
		mootext.insert(cursor+(cursor > cursorStart)*2/*+(cursor < cursorStart)*/, "\x02");
	}
	vid->DrawText(position.X+2, position.Y+3, mootext.c_str(), 255, 255, 255, 100);//focus?255:100);
}

void Label::OnTick()
{
	if (!focus && numClicks && lastClick+300 < SDL_GetTicks())
		numClicks = 0;
}

#endif