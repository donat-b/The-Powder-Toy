#ifndef LABEL_H
#define LABEL_H

#include <string>
#include "common/Point.h"

class VideoBuffer;
class Label
{
private:
	void UpdateCursorTemp(int x, int y);
	unsigned int UpdateCursor(unsigned int position);
	bool multiline;

protected:
	Point position;
	Point size;

	std::string text;
	unsigned int cursor, cursorStart;
	unsigned int lastClick, numClicks, clickPosition;

	void UpdateDisplayText(bool updateCursor = false, bool firstClick = false, int mx = 0, int my = 0);
	void MoveCursor(unsigned int *cursor, int amount);
	virtual bool ShowCursor() { return false; }

public:
	Label(std::string text, Point position, Point size, bool multiline = false);

	bool focus;
	Point GetPosition() { return position; }
	Point GetSize() { return size; }
	void SetText(std::string text_);
	std::string GetText();

	virtual void OnMouseDown(int x, int y, unsigned char button);
	virtual void OnMouseUp(int x, int y, unsigned char button);
	virtual void OnMouseMoved(int x, int y, Point difference);
	virtual void OnKeyPress(int key, unsigned short character, unsigned char modifiers);
	virtual void OnDraw(VideoBuffer* vid);
	virtual void OnTick();
};

#endif
