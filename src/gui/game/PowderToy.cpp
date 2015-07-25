#include <sstream>
#include "SDLCompat.h"
#include "json/json.h"

#include "PowderToy.h"
#include "defines.h"
#include "interface.h"
#include "luaconsole.h"
#include "powder.h"
#include "save.h"
#include "update.h"

#include "game/Download.h"
#include "game/ToolTip.h"
#include "interface/Button.h"
#include "interface/Engine.h"
#include "interface/Window.h"
#include "graphics/VideoBuffer.h"

#include "gui/profile/ProfileViewer.h"

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
	numNotifications(0),
	voteDownload(NULL),
	loginCheckTicks(0),
	loginFinished(0)
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
	else
		versionCheck = NULL;

	if (svf_login)
	{
		sessionCheck = new Download("http://" SERVER "/Startup.json");
		sessionCheck->AuthHeaders(svf_user_id, svf_session_id);
		sessionCheck->Start();
	}
	else
		sessionCheck = NULL;

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

	class VoteAction : public ButtonAction
	{
		bool voteType;
	public:
		VoteAction(bool up):
			ButtonAction()
		{
			voteType = up;
		}

		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->DoVote(voteType);
		}
	};
	upvoteButton = new Button(saveButton->Right(Point(1, 0)), Point(40, 15), "\xCB Vote");
	upvoteButton->SetColor(COLRGB(0, 187, 18));
	upvoteButton->SetCallback(new VoteAction(true));
	AddComponent(upvoteButton);

	downvoteButton = new Button(upvoteButton->Right(Point(0, 0)), Point(16, 15), "\xCA");
	downvoteButton->SetColor(COLRGB(187, 40, 0));
	downvoteButton->SetCallback(new VoteAction(false));
	AddComponent(downvoteButton);
	/*reportBugButton;
	optionsButton;
	clearSimButton;*/


	// We now start placing buttons from the right side, because tags button is in the middle and uses whatever space is leftover
	class PauseAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->TogglePause();
		}
	};
	pauseButton = new Button(Point(XRES+BARSIZE-16, yPos), Point(15, 15), "\x90");
	pauseButton->SetCallback(new PauseAction());
	AddComponent(pauseButton);

	class RenderOptionsAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->RenderOptions();
		}
	};
	renderOptionsButton = new Button(pauseButton->Left(Point(18, 0)), Point(17, 15), "\x0F\xFF\x01\x01\xD8\x0F\x01\xFF\x01\xD9\x0F\x01\x01\xFF\xDA");
	renderOptionsButton->SetCallback(new RenderOptionsAction());
	AddComponent(renderOptionsButton);

	class LoginButtonAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->LoginButton();
		}
	};
	loginButton = new Button(renderOptionsButton->Left(Point(91, 0)), Point(90, 15), "\x84 [sign in]");
	loginButton->SetAlign(Button::LEFT);
	loginButton->SetCallback(new LoginButtonAction());
	AddComponent(loginButton);
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
				UpdateToolTip("Error creating save", Point(XCNTR-VideoBuffer::TextSize("Error Saving").X/2, YCNTR-10), INFOTIP, 1000);
			}
			else
			{
				if (DoLocalSave(svf_filename, saveData, saveSize, true))
					UpdateToolTip("Error writing local save", Point(XCNTR-VideoBuffer::TextSize("Error Saving").X/2, YCNTR-10), INFOTIP, 1000);
				else
					UpdateToolTip("Updated successfully", Point(XCNTR-VideoBuffer::TextSize("Saved Successfully").X/2, YCNTR-10), INFOTIP, 1000);
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
					UpdateToolTip("Error Saving", Point(XCNTR-VideoBuffer::TextSize("Error Saving").X/2, YCNTR-10), INFOTIP, 1000);
				}
			}
		}
		// local quick save
		else
		{
			if (execute_save(vid_buf))
			{
				UpdateToolTip("Error Saving", Point(XCNTR-VideoBuffer::TextSize("Error Saving").X/2, YCNTR-10), INFOTIP, 1000);
			}
			else
			{
				UpdateToolTip("Saved Successfully", Point(XCNTR-VideoBuffer::TextSize("Saved Successfully").X/2, YCNTR-10), INFOTIP, 1000);
			}
		}
	}
}

void PowderToy::DoVote(bool up)
{
	voteDownload = new Download("http://" SERVER "/Vote.api");
	voteDownload->AuthHeaders(svf_user_id, svf_session_id);
	std::map<std::string, std::string> postData;
	postData.insert(std::pair<std::string, std::string>("ID", svf_id));
	postData.insert(std::pair<std::string, std::string>("Action", up ? "Up" : "Down"));
	voteDownload->AddPostData(postData);
	voteDownload->Start();
	svf_myvote = up ? 1 : -1; // will be reset later upon error
}

void PowderToy::LoginButton()
{
	if (svf_login && mouse.X <= loginButton->GetPosition().X+18)
	{
		ProfileViewer *temp = new ProfileViewer(svf_user);
		Engine::Ref().ShowWindow(temp);
	}
	else
	{
		int ret = login_ui(vid_buf);
		if (ret && svf_login)
		{
			save_presets(0);
			if (sessionCheck)
			{
				sessionCheck->Cancel();
				sessionCheck = NULL;
			}
			loginFinished = 1;
		}
	}
}

void PowderToy::RenderOptions()
{
	render_ui(vid_buf, XRES+BARSIZE-(510-491)+1, YRES+22, 3);
}

void PowderToy::TogglePause()
{
	sys_pause = !sys_pause;
}

void PowderToy::ConfirmUpdate()
{
	confirm_update(changelog.c_str());
}

Button * PowderToy::AddNotification(std::string message)
{
	int messageSize = VideoBuffer::TextSize(message).X;
	Button *notificationButton = new Button(Point(XRES-19-messageSize-5, YRES-22-20*numNotifications), Point(messageSize+5, 15), message);
	notificationButton->SetColor(COLRGB(255, 216, 32));
	AddComponent(notificationButton);
	numNotifications++;
	return notificationButton;
}

void PowderToy::OnTick(uint32_t ticks)
{
	int mouseX, mouseY;
	int mouseDown = mouse_get_state(&mouseX, &mouseY);
	main_loop_temp(mouseDown, lastMouseDown, heldKey, heldKeyAscii, heldModifier, mouseX, mouseY, mouseWheel);
	lastMouseDown = mouseDown;
	heldKey = heldKeyAscii = releasedKey = mouseWheel = 0;

	if (!loginFinished)
		loginCheckTicks = (loginCheckTicks+1)%51;

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
					Button *notification = AddNotification("A new version is available - click here!");
					notification->SetCallback(new DoUpdateAction());
					AddComponent(notification);
				}
		}
		else
		{
			const char *temp = "Error, could not find update server. Press Ctrl+u to go check for a newer version manually on the tpt website";
			UpdateToolTip(temp, Point(XCNTR-VideoBuffer::TextSize(temp).X/2, YCNTR-10), INFOTIP, 2500);
			UpdateToolTip("", Point(16, 20), INTROTIP, 0);
		}
		if (ver_data)
			free(ver_data);
		versionCheck = NULL;
	}
	if (sessionCheck && sessionCheck->CheckDone())
	{
		int status = 200;
		char *ret = sessionCheck->Finish(NULL, &status);
		// ignore timeout errors or others, since the user didn't actually click anything
		if (status != 200 || ParseServerReturn(ret, status, true))
		{
			// key icon changes to red
			loginFinished = -1;
		}
		else
		{
			std::istringstream datastream(ret);
			Json::Value root;

			try
			{
				datastream >> root;

				if (!root["Session"].asInt())
				{
					// TODO: better login system, why do we reset all these
					strcpy(svf_user, "");
					strcpy(svf_user_id, "");
					strcpy(svf_session_id, "");
					svf_login = 0;
					svf_own = 0;
					svf_admin = 0;
					svf_mod = 0;
				}

				//std::string motd = root["MessageOfTheDay"].asString();

				class NotificationOpenAction : public ButtonAction
				{
					std::string link;
				public:
					NotificationOpenAction(std::string link_):
						ButtonAction()
					{
						link = link_;
					}

					virtual void ButtionActionCallback(Button *button, unsigned char b)
					{
						if (b == 1)
							open_link(link);
						dynamic_cast<PowderToy*>(button->GetParent())->RemoveComponent(button);
					}
				};
				Json::Value notifications = root["Notifications"];
				for (int i = 0; i < notifications.size(); i++)
				{
					std::string message = notifications[i]["Text"].asString();
					std::string link = notifications[i]["Link"].asString();

					Button *notification = AddNotification(message);
					notification->SetCallback(new NotificationOpenAction(link));
				}
				loginFinished = 1;
			}
			catch (std::exception &e)
			{
				// this shouldn't happen because the server hopefully won't return bad data ...
				loginFinished = -1;
			}
		}
		sessionCheck = NULL;
	}
	if (voteDownload && voteDownload->CheckDone())
	{
		int status;
		char *ret = voteDownload->Finish(NULL, &status);
		if (ParseServerReturn(ret, status, false))
			svf_myvote = 0;
		else
			UpdateToolTip("Voted Successfully", Point(XCNTR-VideoBuffer::TextSize("Voted Successfully").X/2, YCNTR-10), INFOTIP, 1000);
		free(ret);
		voteDownload = NULL;
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
	openBrowserButton->SetState(ctrl ? Button::INVERTED : Button::NORMAL);
	saveButton->SetState((svf_login && ctrl) ? Button::INVERTED : Button::NORMAL);
	std::string saveButtonText = "\x82 ";
	if (!svf_login || ctrl)
	{
		if (svf_fileopen)
			saveButtonText += svf_filename;
		else
			saveButtonText += "[save to disk]";
	}
	else
	{
		if (svf_open)
			saveButtonText += svf_name;
		else
			saveButtonText += "[untitled simulation]";
	}
	saveButton->SetText(saveButtonText);
	bool votesAllowed = svf_login && svf_open && svf_own == 0 && svf_myvote == 0;
	upvoteButton->SetEnabled(votesAllowed);
	downvoteButton->SetEnabled(votesAllowed);
	upvoteButton->SetState(svf_myvote == 1 ? Button::HIGHLIGHTED : Button::NORMAL);
	downvoteButton->SetState(svf_myvote == -1 ? Button::HIGHLIGHTED : Button::NORMAL);
	if (svf_login)
	{
		std::string loginButtonText;
		if (loginFinished == 1)
			loginButtonText = "\x0F\x01\xFF\x01\x84\x0E ";
		else if (loginFinished == -1)
			loginButtonText = "\x0F\xFF\x01\x01\x84\x0E ";
		else
			loginButtonText = "\x84 ";
		loginButton->SetText(loginButtonText + svf_user);
	}
	else
		loginButton->SetText("\x84 [sign in]");
	pauseButton->SetState(sys_pause ? Button::INVERTED : Button::NORMAL);

	VideoBufferHack();
}

void PowderToy::OnDraw(VideoBuffer *buf)
{
	ARGBColour dotColor = 0;
	bool ctrl = (heldModifier & (KMOD_CTRL|KMOD_META)) ? true : false;
	if (svf_fileopen && svf_login && ctrl)
		dotColor = COLPACK(0x000000);
	else if ((!svf_login && svf_fileopen) || (svf_open && svf_own && !ctrl))
		dotColor = COLPACK(0xFFFFFF);
	if (dotColor)
	{
		for (int i = 1; i <= 13; i+= 2)
			buf->DrawPixel(saveButton->GetPosition().X+18, saveButton->GetPosition().Y+i, COLR(dotColor), COLG(dotColor), COLB(dotColor), 255);
	}

	if (svf_login)
	{
		for (int i = 1; i <= 13; i+= 2)
			buf->DrawPixel(loginButton->GetPosition().X+18, loginButton->GetPosition().Y+i, 255, 255, 255, 255);

		// login check hasn't finished, key icon is dynamic
		if (loginFinished == 0)
			buf->FillRect(loginButton->GetPosition().X+2+loginCheckTicks/3, loginButton->GetPosition().Y+1, 16-loginCheckTicks/3, 13, 0, 0, 0, 255);

		if (svf_admin)
		{
			Point iconPos = loginButton->Right(Point(-12, 3));
			buf->DrawText(iconPos.X, iconPos.Y, "\xC9", 232, 127, 35, 255);
			buf->DrawText(iconPos.X, iconPos.Y, "\xC7", 255, 255, 255, 255);
			buf->DrawText(iconPos.X, iconPos.Y, "\xC8", 255, 255, 255, 255);
		}
		else if (svf_mod)
		{
			Point iconPos = loginButton->Right(Point(-12, 3));
			buf->DrawText(iconPos.X, iconPos.Y, "\xC9", 35, 127, 232, 255);
			buf->DrawText(iconPos.X, iconPos.Y, "\xC7", 255, 255, 255, 255);
		}
		else if (true)
		{
			Point iconPos = loginButton->Right(Point(-12, 3));
			buf->DrawText(iconPos.X, iconPos.Y, "\x97", 0, 230, 153, 255);
		}
	}
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
	heldModifier = modifiers;
	// key -1 is fake event sent in order to update modifiers when in other interfaces
	if (key == -1)
		return;
	heldKey = key;
	heldKeyAscii = character;

#ifdef LUACONSOLE
	if (key != -1 && !deco_disablestuff && !luacon_keyevent(key, modifiers, LUACON_KDOWN))
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
	heldModifier = modifiers;
	// key -1 is fake event sent in order to update modifiers when in other interfaces
	if (key == -1)
		return;
	releasedKey = key;

#ifdef LUACONSOLE
	if (!deco_disablestuff && !luacon_keyevent(key, modifiers, LUACON_KUP))
		key = 0;
#endif
}
