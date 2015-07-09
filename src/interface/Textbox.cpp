#include <algorithm>
#include <sstream>
#ifdef SDL_R_INCL
#include <SDL_keysym.h>
#else
#include <SDL/SDL_keysym.h>
#endif
#include "Textbox.h"
#include "graphics/VideoBuffer.h"
#include "misc.h"

Textbox::Textbox(Point position, Point size, std::string text, bool multiline):
	Label(position, size, text, multiline),
	sizeLimit(Point(NOSIZELIMIT, NOSIZELIMIT)),
	characterLimit(10000),
	callback(NULL),
	type(TEXT)
{
}

Textbox::~Textbox()
{
	delete callback;
}

void Textbox::SetCallback(TextboxAction *callback_)
{
	if (callback)
		delete callback;
	callback = callback_;
}

//deletes any highlighted text, returns true if there was something deleted (cancels backspace/delete action)
bool Textbox::DeleteHighlight(bool updateDisplayText)
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
	if (updateDisplayText)
	{
		UpdateDisplayText();
		if (callback)
			callback->TextChangedCallback(this);
	}
	return true;
}

void Textbox::InsertText(std::string inserttext)
{
	inserttext = CleanText(inserttext, true, true, type != MULTILINE);
	if (!inserttext.length())
		return;

	std::string oldText = text;
	int oldCursor = cursor, oldCursorStart = cursorStart;
	size_t len = inserttext.length();

	DeleteHighlight(false);
	//text.insert(cursor, 1, static_cast<char>(key));
	text.insert(cursor, inserttext);

	int oldLines = std::count(text.begin(), text.begin()+cursor+len, '\r');
	UpdateDisplayText();
	// this character doesn't fit, revert changes to string (not really a nice way to do this :|)
	if ((multiline ? (autosizeY ? sizeLimit.Y != NOSIZELIMIT && size.Y > sizeLimit.Y : textHeight > size.Y) : (autosizeX ? sizeLimit.X != NOSIZELIMIT && size.X > sizeLimit.X : textWidth > size.X))
	    || text.length() > characterLimit)
	{
		text = oldText;
		cursor = oldCursor;
		cursorStart = oldCursorStart;
		UpdateDisplayText();
		return;
	}
	int newLines = std::count(text.begin(), text.begin()+cursor+len, '\r'), newLines2 = newLines;
	while ((newLines = std::count(text.begin(), text.begin()+cursor+len+newLines-oldLines, '\r')) != newLines2)
		newLines2 = newLines; //account for any newlines that were inserted (while loop for proper checking if like 1000 newlines were inserted ...)

	//put cursor at end of the new paste, accounting for extra newlines
	cursor += len+newLines-oldLines;
	cursorStart = cursor;

	if (callback)
		callback->TextChangedCallback(this);
}

void Textbox::OnKeyPress(int key, unsigned short character, unsigned short modifiers)
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
			std::string clipboard = clipboard_pull_text();
			InsertText(clipboard);
			break;
		}
		case 'x':
			DeleteHighlight(true);
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
		if (!DeleteHighlight(true) && cursor > 0)
		{
			// move cursor barkward 1 real character (accounts for formatting that also needs to be deleted)
			MoveCursor(&cursor, -1);
			text.erase(cursor, cursorStart-cursor);
			cursorStart = cursor;

			int oldLines = std::count(text.begin(), text.begin()+cursor, '\r');
			UpdateDisplayText();
			int newLines = std::count(text.begin(), text.begin()+cursor, '\r');

			// make sure cursor is correct when deleting the character put us on the previous line or next line (deleting a space)
			if (oldLines != newLines)
			{
				cursor += newLines-oldLines;
				cursorStart = cursor;
			}

			if (callback)
				callback->TextChangedCallback(this);
		}
		break;
	case SDLK_DELETE:
		if (!DeleteHighlight(true) && text.length() && cursor < text.length())
		{
			// move cursor forward 1 real character (accounts for formatting that also needs to be deleted)
			MoveCursor(&cursor, 1);
			text.erase(cursorStart, cursor-cursorStart);
			cursor = cursorStart;

			int oldLines = std::count(text.begin(), text.begin()+cursor, '\r');
			UpdateDisplayText();
			int newLines = std::count(text.begin(), text.begin()+cursor, '\r');

			// make sure cursor is correct when deleting the character put us on the next line (deleting a space)
			if (oldLines != newLines)
			{
				cursor += newLines-oldLines;
				cursorStart = cursor;
			}

			if (callback)
				callback->TextChangedCallback(this);
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
	case SDLK_UP:
		if (cursor != cursorStart)
		{
			if (cursor < cursorStart)
				cursorStart = cursor;
			else
				cursor = cursorStart;
		}
		else
		{
			if (cursorPosReset)
			{
				UpdateDisplayText();
				cursorPosReset = false;
			}
			cursorY -= 12;
			if (cursorY < 0)
				cursorY = 0;
			int cx = cursorX, cy = cursorY;
			UpdateDisplayText(true, true);
			// make sure these are set to what we want
			cursorX = cx;
			cursorY = cy;
		}
		break;
	case SDLK_DOWN:
		if (cursor != cursorStart)
		{
			if (cursor > cursorStart)
				cursorStart = cursor;
			else
				cursor = cursorStart;
		}
		else
		{
			if (cursorPosReset)
			{
				UpdateDisplayText();
				cursorPosReset = true;
			}
			cursorY += 12;
			if (cursorY > size.Y)
				cursorY = size.Y;
			int cx = cursorX, cy = cursorY;
			UpdateDisplayText(true, true);
			// make sure these are set to what we want
			cursorX = cx;
			cursorY = cy;
		}
		break;
	case SDLK_RETURN:
		if (this->type == MULTILINE)
		{
			InsertText("\n");
		}
		break;
	default:
		if (this->type == NUMBER ? character >= '0' && character <= '9' : character >= ' ' && character <= '~')
		{
			std::stringstream convert;
			convert << static_cast<char>(character);
			InsertText(convert.str());
		}
		break;
	}
}

void Textbox::OnDraw(VideoBuffer* vid)
{
	Label::OnDraw(vid);
	if (IsFocused() && enabled)
		vid->DrawRect(position.X, position.Y, size.X, size.Y, 255, 255, 255, 255);
	else
		vid->DrawRect(position.X, position.Y, size.X, size.Y, 150, 150, 150, 255);
}

void Textbox::SetAutoSize(bool X, bool Y, Point limit)
{
	autosizeX = X;
	autosizeY = Y;
	sizeLimit = limit;
}
