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
public:
	static const int NOSIZELIMIT = 0; // Component size, not text size
	enum texttype { TEXT, MULTILINE, NUMBER };

	Textbox(Point position, Point size, std::string text, bool multiline = false);
	~Textbox();

	void SetCallback(TextboxAction *callback_);

	virtual void OnKeyPress(int key, unsigned short character, unsigned short modifiers);
	virtual void OnDraw(VideoBuffer* vid);

	void SetAutoSize(bool X, bool Y, Point limit);
	void SetCharacterLimit(int characterLimit_) { characterLimit = characterLimit_; }
	void SetType(texttype type_) { type = type_; }

private:
	Point sizeLimit;
	unsigned int characterLimit;
	TextboxAction *callback;
	texttype type;

	bool DeleteHighlight(bool updateDisplayText);
	void InsertText(std::string inserttext);

protected:
	bool ShowCursor() { return true; }
};

#endif
