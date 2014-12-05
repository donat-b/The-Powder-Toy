#ifndef LABEL_H
#define LABEL_H

#include <string>
#include "common/Point.h"
#include "Component.h"

class VideoBuffer;
class Label : public Component
{
private:
	void UpdateCursorTemp(int x, int y);
	unsigned int UpdateCursor(unsigned int position);
	bool multiline;

protected:
	std::string text;
	int textWidth;
	unsigned int cursor, cursorStart;
	unsigned int lastClick, numClicks, clickPosition;

	void UpdateDisplayText(bool updateCursor = false, bool firstClick = false, int mx = 0, int my = 0);
	void MoveCursor(unsigned int *cursor, int amount);
	virtual bool ShowCursor() { return false; }

public:
	Label(Point position, Point size, std::string text, bool multiline = false);

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
