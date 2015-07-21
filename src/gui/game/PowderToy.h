#ifndef POWDERTOY_H
#define POWDERTOY_H

#include <string>
#include "interface/Window.h"
#include "graphics/Pixel.h"

class Button;
class VideoBuffer;
class Download;
class PowderToy : public Window_
{
	int lastMouseDown, heldKey, heldKeyAscii, releasedKey;
	unsigned short heldModifier;
	int mouseWheel;

	// bottom bar buttons
	Button *openInBrowserButton;
	Button *reloadButton;
	Button *saveButton;
	Button *upvoteButton;
	Button *downvoteButton;
	Button *reportBugButton;
	Button *optionsButton;
	Button *clearSimButton;
	Button *loginButton;
	Button *renderOptionsButton;
	Button *pauseButton;

	// update stuff
	Download *versionCheck;
	std::string changelog;
public:
	PowderToy();
	~PowderToy();

	void ConfirmUpdate();

	void OnTick(uint32_t ticks);
	void OnDraw(VideoBuffer *buf);
	void OnMouseMove(int x, int y, Point difference);
	void OnMouseDown(int x, int y, unsigned char button);
	void OnMouseUp(int x, int y, unsigned char button);
	void OnMouseWheel(int x, int y, int d);
	void OnKeyPress(int key, unsigned short character, unsigned short modifiers);
	void OnKeyRelease(int key, unsigned short character, unsigned short modifiers);
};

#endif
