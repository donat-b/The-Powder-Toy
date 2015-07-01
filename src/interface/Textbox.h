#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "Label.h"

class VideoBuffer;
class Textbox;
class TextboxAction
{
public:
	TextboxAction() { }
	virtual void TextChangedCallback(Textbox *textbox) { }
};

class Textbox : public Label
{
	Point sizeLimit;
	TextboxAction *callback;
	bool DeleteHighlight(bool updateDisplayText);
	void InsertText(std::string inserttext);

protected:
	bool ShowCursor() { return true; }

public:
	Textbox(Point position, Point size, std::string text, bool multiline = false);
	~Textbox();

	void SetCallback(TextboxAction *callback_);

	virtual void OnKeyPress(int key, unsigned short character, unsigned char modifiers);
	virtual void OnDraw(VideoBuffer* vid);

	void SetAutoSize(bool X, bool Y, Point limit);

	static const int NOSIZELIMIT = 0;
};

#endif
