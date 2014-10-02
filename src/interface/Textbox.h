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
	Textbox(std::string text, Point position, Point size, bool multiline);
	~Textbox();

	virtual void OnKeyPress(int key, unsigned short character, unsigned char modifiers);
	virtual void OnDraw(VideoBuffer* vid);
};

#endif
