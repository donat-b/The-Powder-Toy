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
public:
	enum StampState { NONE, LOAD, COPY, CUT, SAVE };

private:
	Point mouse;
	Point cursor;
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

	// zoom
	bool placingZoom;
	bool placingZoomTouch; // clicked the zoom button, zoom window won't be drawn until user clicks
	bool zoomEnabled;
	Point zoomedOnPosition;
	Point zoomWindowPosition;
	int zoomSize;
	int zoomFactor;

	// loading stamps
	StampState state;
	Point loadPos;
	Point loadSize;
	void *stampData;
	int stampSize;
	pixel *stampImg;
	bool waitToDraw; // wait a frame to draw stamp after load, because it will be in the wrong spot until another mouse event comes in

	// saving stamps
	bool savedInitial;
	Point savePos;
	Point saveSize;
	void *clipboardData;
	int clipboardSize;

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

	// these buttons only appear when using the touchscreen interface
#ifdef TOUCHUI
	Button *openConsoleButton;
	Button *eraseButton;
	Button *settingsButton;
	Button *zoomButton;
	Button *stampButton;
#endif
	bool ignoreMouseUp;

public:
	PowderToy();
	~PowderToy();

	void ConfirmUpdate();
	bool MouseClicksIgnored();
	Point AdjustCoordinates(Point mouse);

	// zoom window stuff
	bool ZoomWindowShown() { return placingZoom || zoomEnabled; }
	bool PlacingZoomWindow() { return placingZoom; }
	void UpdateZoomCoordinates(Point mouse);
	void HideZoomWindow();
	Point GetZoomedOnPosition() { return zoomedOnPosition; }
	Point GetZoomWindowPosition() { return zoomWindowPosition; }
	int GetZoomWindowSize() { return zoomSize; }
	int GetZoomWindowFactor() { return zoomFactor; }

	// stamp stuff (so main() can get needed info)
	void UpdateStampCoordinates(Point cursor);
	StampState GetState() { return state; }
	Point GetStampPos() { return loadPos; }
	Point GetStampSize() { return loadSize; }
	pixel * GetStampImg() { return waitToDraw ? NULL : stampImg; }
	Point GetSavePos() { return savePos; }
	Point GetSaveSize() { return saveSize; }
	bool PlacedInitialStampCoordinate() { return savedInitial; }

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

#ifdef TOUCHUI
	void OpenConsole(bool alt);
	void ToggleErase(bool alt);
	void ToggleSetting(bool alt);
	void StartZoom(bool alt);
	void SaveStamp(bool alt);
#endif
};

#endif
