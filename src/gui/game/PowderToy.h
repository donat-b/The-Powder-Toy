#ifndef POWDERTOY_H
#define POWDERTOY_H

#include <string>
#include "interface/Window.h"
#include "graphics/Pixel.h"

class Button;
class VideoBuffer;
class Download;
class ToolTip;
class PowderToy : public Window_
{
public:
	enum StampState { NONE, LOAD, COPY, CUT, SAVE };
	enum DrawState { POINTS, LINE, RECT, FILL };

private:
	Point mouse;
	Point cursor;
	int lastMouseDown, heldKey, heldAscii, releasedKey;
	unsigned short heldModifier;
	int mouseWheel;
	bool mouseCanceled;

	// notifications
	int numNotifications;
	Button * AddNotification(std::string message);

	// website stuff
	Download *versionCheck;
	Download *sessionCheck; // really a tpt++ version check but it does session too and has nice things
	Download *voteDownload;

	// drawing stuff
	DrawState drawState;
	bool isMouseDown;
	bool isStampMouseDown;
	int toolIndex; // 0 = left mouse, 1 = right mouse
	float toolStrength; // tool (heat / cool) strength can be modified with ctrl or shift
	Point lastDrawPoint; // for normal point to point drawing
	Point initialDrawPoint; // for lines and boxes
	bool ctrlHeld;
	bool shiftHeld;
	bool altHeld;
	bool mouseInZoom; // mouse drawing is canceled when moving in / out of the zoom window, need state
	bool skipDraw; // when mouse moves, don't attempt an extra draw the next tick as normal

	// zoom
	bool placingZoom;
	bool placingZoomTouch; // clicked the zoom button, zoom window won't be drawn until user clicks
	bool zoomEnabled;
	Point zoomedOnPosition; // position the zoom window is zooming in on
	Point zoomWindowPosition; // position where zoom is drawn on screen (either on the left or the right)
	Point zoomMousePosition; // position where the mouse was when placing the zoom window, needed so that we can resize it without glitching things
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
	// touch ui stuff for rotating / moving stamps
#ifdef TOUCHUI
	Point stampClickedPos;
	Point stampClickedOffset;
	Point initialLoadPos;
	int stampQuadrant;
	bool stampMoving;
#endif

	// saving stamps
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
	Button *eraseButton;
	Button *openConsoleButton;
	Button *settingsButton;
	Button *zoomButton;
	Button *stampButton;
#endif
	bool ignoreMouseUp;

public:
	PowderToy();
	~PowderToy();

	void ConfirmUpdate(std::string changelog, std::string file);
	bool MouseClicksIgnored();
	Point AdjustCoordinates(Point mouse);
	bool IsMouseInZoom(Point mouse);
	void SetInfoTip(std::string infotip);
	ToolTip *GetQTip(std::string qtip, int y);

	// drawing stuff
	void UpdateDrawMode();
	void UpdateToolStrength();
	DrawState GetDrawState() { return drawState; }
	bool IsMouseDown() { return isMouseDown; }
	float GetToolStrength() { return toolStrength; }
	Point GetInitialDrawPoint() { return initialDrawPoint; }
	Point LineSnapCoords(Point point1, Point point2);
	Point RectSnapCoords(Point point1, Point point2);

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
	void UpdateStampCoordinates(Point cursor, Point offset = Point(0, 0));
	StampState GetStampState() { return state; }
	void ResetStampState();
	Point GetStampPos() { return loadPos; }
	Point GetStampSize() { return loadSize; }
	pixel * GetStampImg() { return waitToDraw ? NULL : stampImg; }
	Point GetSavePos() { return savePos; }
	Point GetSaveSize() { return saveSize; }
	bool PlacedInitialStampCoordinate() { return isStampMouseDown; }

	void OnTick(uint32_t ticks);
	void OnDraw(VideoBuffer *buf);
	void OnMouseMove(int x, int y, Point difference);
	void OnMouseDown(int x, int y, unsigned char button);
	void OnMouseUp(int x, int y, unsigned char button);
	void OnMouseWheel(int x, int y, int d);
	void OnKeyPress(int key, unsigned short character, unsigned short modifiers);
	void OnKeyRelease(int key, unsigned short character, unsigned short modifiers);
	void OnDefocus();

	bool BeforeMouseDown(int x, int y, unsigned char button);
	bool BeforeMouseUp(int x, int y, unsigned char button);
	bool BeforeMouseWheel(int x, int y, int d);
	bool BeforeKeyPress(int key, unsigned short character, unsigned short modifiers);
	bool BeforeKeyRelease(int key, unsigned short character, unsigned short modifiers);

	void OpenBrowser(unsigned char b);
	void ReloadSave(unsigned char b);
	void DoSave(unsigned char b);
	void DoVote(bool up);
	void OpenTags();
	void ReportBug();
	void OpenOptions();
	void LoginButton();
	void RenderOptions();
	void TogglePause();

#ifdef TOUCHUI
	void ToggleErase(bool alt);
	void OpenConsole(bool alt);
	void ToggleSetting(bool alt);
	void StartZoom(bool alt);
	void SaveStamp(bool alt);
#endif
};

#endif
