#ifndef LABEL_H
#define LABEL_H

#include <string>
#include "common/Point.h"
#include "Component.h"

class VideoBuffer;
class Label : public Component
{
private:
	void CleanText(bool ascii, bool color, bool newlines);
	void FindWordPosition(unsigned int position, unsigned int *cursorStart, unsigned int *cursorEnd, const char* spaces);
	unsigned int UpdateCursor(unsigned int position);
	uint32_t currentTick;

protected:
	std::string text;
	int textWidth;
	bool multiline;
	unsigned int cursor, cursorStart;
	uint32_t lastClick;
	unsigned int numClicks, clickPosition;

	void UpdateDisplayText(bool updateCursor = false, bool firstClick = false, int mx = 0, int my = 0);
	void MoveCursor(unsigned int *cursor, int amount);
	virtual bool ShowCursor() { return false; }

public:
	Label(Point position, Point size, std::string text, bool multiline = false);

	void SetText(std::string text_);
	std::string GetText();
	bool IsMultiline() { return multiline; }

	virtual void OnMouseDown(int x, int y, unsigned char button);
	virtual void OnMouseUp(int x, int y, unsigned char button);
	virtual void OnDefocus();
	virtual void OnMouseMoved(int x, int y, Point difference);
	virtual void OnKeyPress(int key, unsigned short character, unsigned char modifiers);
	virtual void OnDraw(VideoBuffer* vid);
	virtual void OnTick(uint32_t ticks);

	static const int AUTOSIZE = -1;
};

#endif
