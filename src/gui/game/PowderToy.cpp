#include <sstream>
#include "SDLCompat.h"

#include "PowderToy.h"
#include "defines.h"
#include "graphics.h"
#include "interface.h"
#include "luaconsole.h"
#include "powder.h"
#include "save.h"
#include "update.h"

#include "game/Download.h"
#include "game/ToolTip.h"
#include "interface/Button.h"
#include "interface/Window.h"
#include "graphics/VideoBuffer.h"

PowderToy::~PowderToy()
{
	main_end_hack();
}

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

	int yPos = YRES+MENUSIZE-16;

	class OpenBrowserAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->OpenBrowser();
		}
	};
	openBrowserButton = new Button(Point(1, yPos), Point(17, 15), "\x81");
	openBrowserButton->SetCallback(new OpenBrowserAction());
	AddComponent(openBrowserButton);

	class ReloadAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->ReloadSave(b);
		}
	};
	reloadButton = new Button(openBrowserButton->Right(Point(1, 0)), Point(17, 15), "\x91");
	reloadButton->SetCallback(new ReloadAction());
	reloadButton->SetEnabled(false);
	AddComponent(reloadButton);

	class SaveAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->DoSave();
		}
	};
	saveButton = new Button(reloadButton->Right(Point(1, 0)), Point(151, 15), "\x82 [untitled simulation]");
	saveButton->SetAlign(Button::LEFT);
	saveButton->SetCallback(new SaveAction());
	AddComponent(saveButton);
	/*upvoteButton;
	downvoteButton;
	reportBugButton;
	optionsButton;
	clearSimButton;
	loginButton;
	renderOptionsButton;
	pauseButton;*/
}

void PowderToy::OpenBrowser()
{
	if (heldModifier & (KMOD_CTRL|KMOD_META))
		catalogue_ui(vid_buf);
	else
		search_ui(vid_buf);
}

void PowderToy::ReloadSave(unsigned char b)
{
	if (b == 1 || !strncmp(svf_id, "", 8))
	{
		parse_save(svf_last, svf_lsize, 1, 0, 0, bmap, vx, vy, pv, fvx, fvy, signs, parts, pmap);
		ctrlzSnapshot();
	}
	else
		open_ui(vid_buf, svf_id, NULL, 0);
}

void PowderToy::DoSave()
{
	if (!svf_login || (sdl_mod & (KMOD_CTRL|KMOD_META)))
	{
		// local quick save
		if (mouse.X <= saveButton->GetPosition().X+18 && svf_fileopen)
		{
			int saveSize;
			void *saveData = build_save(&saveSize, 0, 0, XRES, YRES, bmap, vx, vy, pv, fvx, fvy, signs, parts);
			if (!saveData)
			{
				UpdateToolTip("Error creating save", Point(XCNTR-textwidth("Error Saving")/2, YCNTR-10), INFOTIP, 1000);
			}
			else
			{
				if (DoLocalSave(svf_filename, saveData, saveSize, true))
					UpdateToolTip("Error writing local save", Point(XCNTR-textwidth("Error Saving")/2, YCNTR-10), INFOTIP, 1000);
				else
					UpdateToolTip("Updated successfully", Point(XCNTR-textwidth("Saved Successfully")/2, YCNTR-10), INFOTIP, 1000);
			}
		}
		// local save
		else
			save_filename_ui(vid_buf);
	}
	else
	{
		// local save
		if (!svf_open || !svf_own || mouse.X > saveButton->GetPosition().X+18)
		{
			if (save_name_ui(vid_buf))
			{
				if (!execute_save(vid_buf) && svf_id[0])
				{
					copytext_ui(vid_buf, "Save ID", "Saved successfully!", svf_id);
				}
				else
				{
					UpdateToolTip("Error Saving", Point(XCNTR-textwidth("Error Saving")/2, YCNTR-10), INFOTIP, 1000);
				}
			}
		}
		// local quick save
		else
		{
			if (execute_save(vid_buf))
			{
				UpdateToolTip("Error Saving", Point(XCNTR-textwidth("Error Saving")/2, YCNTR-10), INFOTIP, 1000);
			}
			else
			{
				UpdateToolTip("Saved Successfully", Point(XCNTR-textwidth("Saved Successfully")/2, YCNTR-10), INFOTIP, 1000);
			}
		}
	}
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
				if (buildnum > MOD_BUILD_VERSION)
				{
					std::stringstream changelogStream;
					changelogStream << "\bbYour version: " << MOD_VERSION << "." << MOD_MINOR_VERSION << " (" << MOD_BUILD_VERSION << ")\nNew version: " << major << "." << minor << " (" << buildnum << ")\n\n\bwChangeLog:\n";
					changelogStream << &ver_data[count+2];
					changelog = changelogStream.str();

					class DoUpdateAction : public ButtonAction
					{
					public:
						virtual void ButtionActionCallback(Button *button, unsigned char b)
						{
							if (b == 1)
								dynamic_cast<PowderToy*>(button->GetParent())->ConfirmUpdate();
							button->GetParent()->RemoveComponent(button);
						}
					};
					Button *updateButton = new Button(Point(XRES-19-199, YRES-22), Point(199, 15), "A new version is available - click here!");
					updateButton->SetColor(COLRGB(255, 216, 32));
					updateButton->SetCallback(new DoUpdateAction());
					AddComponent(updateButton);
				}
			free(ver_data);
		}
		else
		{
			const char *temp = "Error, could not find update server. Press Ctrl+u to go check for a newer version manually on the tpt website";
			UpdateToolTip(temp, Point(XCNTR-VideoBuffer::TextSize(temp).X/2, YCNTR-10), INFOTIP, 2500);
			UpdateToolTip("", Point(16, 20), INTROTIP, 0);
		}
		versionCheck = NULL;
	}

	if (openConsole)
	{
		if (console_ui(GetVid()->GetVid()) == -1)
		{
			this->ignoreQuits = false;
			this->toDelete = true;
		}
		openConsole = false;
	}
	if (openSign)
	{
		int mx = mouseX, my = mouseY;
		mouse_coords_window_to_sim(&mx, &my);
		add_sign_ui(GetVid()->GetVid(), mx, my);
		openSign = false;
	}
	if (openProp)
	{
		prop_edit_ui(GetVid()->GetVid());
		openProp = false;
	}

	reloadButton->SetEnabled(svf_last ? true : false);
	bool ctrl = (heldModifier & (KMOD_CTRL|KMOD_META)) ? true : false;
	openBrowserButton->Invert(ctrl);
	saveButton->Invert(svf_login && ctrl);
	std::string saveButtonText = "\x82  ";
	ARGBColour dotColor = 0;
	if (!svf_login || ctrl)
	{
		if (svf_fileopen)
			saveButtonText += svf_filename;
		else
			saveButtonText += "[save to disk]";
		if (svf_fileopen)
		{
			if (svf_login && ctrl)
				dotColor = COLPACK(0x000000);
			else
				dotColor = COLPACK(0xFFFFFF);
		}
	}
	else
	{
		if (svf_open)
			saveButtonText += svf_name;
		else
			saveButtonText += "[untitled simulation]";
		if (svf_open && svf_own)
			dotColor = COLPACK(0xFFFFFF);
	}
	if (dotColor)
	{
		for (int i = 0; i <= 12; i+= 2)
			drawpixel(vid_buf, saveButton->GetPosition().X+18, saveButton->GetPosition().Y+i, COLR(dotColor), COLG(dotColor), COLB(dotColor), 255);
	}
	saveButton->SetText(saveButtonText);

	VideoBufferHack();
}

void PowderToy::OnDraw(VideoBuffer *buf)
{

}

void PowderToy::OnMouseMove(int x, int y, Point difference)
{
	mouse = Point(x, y);
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
