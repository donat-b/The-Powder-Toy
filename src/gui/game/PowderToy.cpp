#include <sstream>
#include "PowderToy.h"
#include "defines.h"
#include "luaconsole.h"
#include "update.h"
#include "game/Download.h"
#include "game/ToolTip.h"
#include "interface/Button.h"
#include "interface/Window.h"
#include "graphics/VideoBuffer.h"

#include "interface.h"
#ifdef SDL_R_INCL
#include <SDL_keysym.h>
#else
#include <SDL/SDL_keysym.h>
#endif

PowderToy::PowderToy():
	Window_(Point(0, 0), Point(XRES+BARSIZE, YRES+MENUSIZE)),
	lastMouseDown(0),
	heldKey(0),
	heldKeyAscii(0),
	releasedKey(0),
	heldModifier(0),
	mouseWheel(0),
	versionCheck(NULL)
{
	ignoreQuits = true;

	if (doUpdates && strcmp(svf_user, "jacob1"))
	{
		if (doUpdates == 2)
			versionCheck = new Download(changelog_uri_alt);
		else
			versionCheck = new Download(changelog_uri);
		if (svf_login)
			versionCheck->AuthHeaders(svf_user, NULL); //username instead of session
		versionCheck->Start();
	}

	//int yPos = YRES+MENUSIZE-15;
	//openInBrowserButton = new Button(Point(2, yPos), Point(15, 13), "\x81");
	//AddComponent(openInBrowserButton);
	/*reloadButton;
	saveButton;
	upvoteButton;
	downvoteButton;
	reportBugButton;
	optionsButton;
	clearSimButton;
	loginButton;
	renderOptionsButton;
	pauseButton;*/
}

PowderToy::~PowderToy()
{
	main_end_hack();
}

void PowderToy::ConfirmUpdate()
{
	confirm_update(changelog.c_str());
}

void PowderToy::OnTick(uint32_t ticks)
{
	int mouseX, mouseY;
	int mouseDown = mouse_get_state(&mouseX, &mouseY);
	main_loop_temp(mouseDown, lastMouseDown, heldKey, heldKeyAscii, heldModifier, mouseX, mouseY, mouseWheel);
	lastMouseDown = mouseDown;
	heldKey = heldKeyAscii = releasedKey = mouseWheel = 0;

	if (versionCheck && versionCheck->CheckDone())
	{
		int status = 200;
		char *ver_data = versionCheck->Finish(NULL, &status);
		if (status == 200 && ver_data)
		{
			int count, buildnum, major, minor;
			if (sscanf(ver_data, "%d %d %d%n", &buildnum, &major, &minor, &count) == 3)
				//if (buildnum > MOD_BUILD_VERSION)
				{
					std::stringstream changelogStream;
					changelogStream << "\bbYour version: " << MOD_VERSION << "." << MOD_MINOR_VERSION << " (" << MOD_BUILD_VERSION << ")\nNew version: " << major << "." << minor << " (" << buildnum << ")\n\n\bwChangeLog:\n";
					changelogStream << &ver_data[count+2];
					changelog = changelogStream.str();

					class DoUpdateAction : public ButtonAction
					{
					public:
						virtual void ButtionActionCallback(Button *button)
						{
							dynamic_cast<PowderToy*>(button->GetParent())->ConfirmUpdate();
							button->GetParent()->RemoveComponent(button);
						}
					};
					Button *updateButton = new Button(Point(XRES-19-200, YRES-22), Point(200, 13), "A new version is available - click here!");
					updateButton->SetColor(COLRGB(255, 216, 32));
					updateButton->SetCallback(new DoUpdateAction());
					AddComponent(updateButton);
				}
			free(ver_data);
		}
		else
		{
			const char *temp = "Error, could not find update server. Press Ctrl+u to go check for a newer version manually on the tpt website";
			UpdateToolTip(temp, Point(XCNTR-textwidth(temp)/2, YCNTR-10), INFOTIP, 2500);
			UpdateToolTip("", Point(16, 20), INTROTIP, 0);
		}
		versionCheck = NULL;
	}

	VideoBufferHack();
}

void PowderToy::OnDraw(VideoBuffer *buf)
{

}

void PowderToy::OnMouseMove(int x, int y, Point difference)
{

}

void PowderToy::OnMouseDown(int x, int y, unsigned char button)
{

}

void PowderToy::OnMouseUp(int x, int y, unsigned char button)
{

}

void PowderToy::OnMouseWheel(int x, int y, int d)
{
	mouseWheel += d;
}

void PowderToy::OnKeyPress(int key, unsigned short character, unsigned short modifiers)
{
	heldKey = key;
	heldKeyAscii = character;
	heldModifier = static_cast<unsigned short>(modifiers);

#ifdef LUACONSOLE
		if (!deco_disablestuff && !luacon_keyevent(key, modifiers, LUACON_KDOWN))
			key = 0;
#endif

	switch (key)
	{
	case 'q':
	case SDLK_ESCAPE:
		if (confirm_ui(vid_buf, "You are about to quit", "Are you sure you want to quit?", "Quit"))
		{
			this->ignoreQuits = false;
			this->toDelete = true;
		}
	}
}

void PowderToy::OnKeyRelease(int key, unsigned short character, unsigned short modifiers)
{
	releasedKey = key;
	heldModifier = static_cast<unsigned short>(modifiers);

#ifdef LUACONSOLE
	if (!deco_disablestuff && !luacon_keyevent(key, modifiers, LUACON_KUP))
		key = 0;
#endif
}
