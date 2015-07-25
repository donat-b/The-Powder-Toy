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
	Point mouse;
	int lastMouseDown, heldKey, releasedKey;
	unsigned short heldModifier;
	int mouseWheel;

	// notifications
	int numNotifications;
	Button * AddNotification(std::string message);

	// website stuff
	Download *versionCheck;
	std::string changelog;
	Download *sessionCheck; // really a tpt++ version check but it does session too and has nice things
	Download *voteDownload;

	// bottom bar buttons
	Button *openBrowserButton;
	Button *reloadButton;
	Button *saveButton;
	Button *upvoteButton;
	Button *downvoteButton;
	Button *openTagsButton;
	Button *reportBugButton;
	Button *optionsButton;
	Button *clearSimButton;
	unsigned int loginCheckTicks;
	int loginFinished;
	Button *loginButton;
	Button *renderOptionsButton;
	Button *pauseButton;

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

	void OpenBrowser();
	void ReloadSave(unsigned char b);
	void DoSave();
	void DoVote(bool up);
	void OpenTags();
	void ReportBug();
	void OpenOptions();
	void LoginButton();
	void RenderOptions();
	void TogglePause();
};

#endif
