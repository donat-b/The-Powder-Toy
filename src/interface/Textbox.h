#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "Label.h"

class VideoBuffer;
class Textbox : public Label
{
	bool DeleteHighlight();

protected:
	bool ShowCursor() { return true; }

public:
	Textbox(Point position, Point size, std::string text, bool multiline);
	~Textbox();

	virtual void OnKeyPress(int key, unsigned short character, unsigned char modifiers);
	virtual void OnDraw(VideoBuffer* vid);
};

#endif
