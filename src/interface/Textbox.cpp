#include <algorithm>
#include "Textbox.h"
#include "graphics/VideoBuffer.h"
#include "misc.h"

Textbox::Textbox(Point position, Point size, std::string text, bool multiline) :
	Label(position, size, text, multiline)
{
}

Textbox::~Textbox()
{
}

//deletes any highlighted text, returns true if there was something deleted (cancels backspace/delete action)
bool Textbox::DeleteHighlight()
{
	if (cursor == cursorStart)
		return false;
	if (cursor > cursorStart)
	{
		text.erase(cursorStart, cursor-cursorStart);
		cursor = cursorStart;
	}
	else
	{
		text.erase(cursor, cursorStart-cursor);
		cursorStart = cursor;
	}
	UpdateDisplayText();
	return true;
}

void Textbox::OnKeyPress(int key, unsigned short character, unsigned char modifiers)
{
	Label::OnKeyPress(key, character, modifiers);
	if (modifiers & (KMOD_CTRL|KMOD_META))
	{
		switch (key)
		{
		case 'a':
		case 'c':
		case SDLK_LEFT:
		case SDLK_RIGHT:
			//these are handled by Labels and should be ignored
			break;
		case 'v':
		{
			char* clipboard = clipboard_pull_text();
			int len = strlen(clipboard), width = textwidth(clipboard);
			DeleteHighlight();
			if (width+textWidth+2 >= size.X)
				return;
			text.insert(cursor, clipboard);

			int oldLines = std::count(text.begin(), text.begin()+cursor+len, '\r');
			UpdateDisplayText();
			int newLines = std::count(text.begin(), text.begin()+cursor+len, '\r'), newLines2 = newLines;
			while ((newLines = std::count(text.begin(), text.begin()+cursor+len+newLines-oldLines, '\r')) != newLines2)
				newLines2 = newLines; //account for any newlines that were inserted (while loop for proper checking if like 1000 newlines were inserted ...)

			//put cursor at end of the new paste, accounting for extra newlines
			cursor += len+newLines-oldLines;
			cursorStart = cursor;
			break;
		}
		case 'x':
			DeleteHighlight();
			break;
		}
		return;
	}
	if (modifiers&KMOD_SHIFT)
	{
		if (key == SDLK_LEFT || key == SDLK_RIGHT)
			return;
	}
	switch (key) //all of these do different things if any text is highlighted
	{
	case SDLK_BACKSPACE:
		if (!DeleteHighlight() && cursor > 0)
		{
			MoveCursor(&cursor, -1);
			text.erase(cursor, cursorStart-cursor);
			cursorStart = cursor;
			UpdateDisplayText();
		}
		break;
	case SDLK_DELETE:
		if (!DeleteHighlight() && cursor < text.length()-1)
		{
			MoveCursor(&cursor, 1);
			text.erase(cursorStart, cursor-cursorStart);
			cursor = cursorStart;
			UpdateDisplayText();
		}
		break;
	case SDLK_LEFT:
		if (cursor != cursorStart)
		{
			if (cursor < cursorStart)
				cursorStart = cursor;
			else
				cursor = cursorStart;
		}
		else
		{
			MoveCursor(&cursor, -1);
			cursorStart = cursor;
			UpdateDisplayText();
		}
		break;
	case SDLK_RIGHT:
		if (cursor != cursorStart)
		{
			if (cursor > cursorStart)
				cursorStart = cursor;
			else
				cursor = cursorStart;
		}
		else
		{
			MoveCursor(&cursor, 1);
			cursorStart = cursor;
		}
		break;
	default:
		if (key >= ' ' && key <= '~')
		{
			int width = charwidth(key);
			DeleteHighlight();
			if (width+textWidth+1 >= size.X)
				return;
			text.insert(cursor++, 1, (char)key);
			cursorStart = cursor;
			int oldLines = std::count(text.begin(), text.begin()+cursor, '\r');
			UpdateDisplayText();
			int newLines = std::count(text.begin(), text.begin()+cursor, '\r');

			//hack to make sure cursor is correct when a newline gets inserted
			if (oldLines < newLines)
			{
				cursor += newLines-oldLines;
				cursorStart = cursor;
				UpdateDisplayText();
			}
		}
		break;
	}
}

void Textbox::OnDraw(VideoBuffer* vid)
{
	Label::OnDraw(vid);
	if (IsFocused() && enabled)
		vid->DrawRect(position.X-1, position.Y-1, size.X+2, size.Y+2, 255, 255, 255, 255);
	else
		vid->DrawRect(position.X-1, position.Y-1, size.X+2, size.Y+2, 150, 150, 150, 255);
}
