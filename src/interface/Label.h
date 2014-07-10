#ifndef LABEL_H
#define LABEL_H

#include <string>
#include "common/Point.h"

class Label
{
public:
	std::string text;
	std::string displayText;
	Point position;
	Point size;
	bool multiline;

	bool focus, clicked;
	int cursor, cursorStart;
	int lastClick, numClicks, clickPosition;

	void UpdateDisplayText(bool setHighlight = false, int mx = 0, int my = 0);

//public:
	Label(std::string text, Point position, Point size, bool multiline = false);

	void SetText(std::string text_);
	std::string GetText();

	virtual void OnMouseDown(int x, int y, unsigned char button);
	virtual void OnMouseUp(int x, int y, unsigned char button);
	virtual void OnMouseMoved(int x, int y, Point difference);
	virtual void OnKeyPress(int key, unsigned short character, unsigned char modifiers);
	virtual void OnDraw(unsigned int* vid_buf);
	virtual void OnTick();
};

void InterfaceConvert(Label &label, int x, int y, int b, int bq, int sdl_key, unsigned char sdl_mod);

#endif
