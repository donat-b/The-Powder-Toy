#include <sstream>
#include "SDLCompat.h"
#include "json/json.h"

#include "PowderToy.h"
#include "defines.h"
#include "interface.h"
#include "gravity.h"
#include "luaconsole.h"
#include "powder.h"
#include "save.h"
#include "update.h"

#include "game/Download.h"
#include "game/Menus.h"
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
	releasedKey(0),
	heldModifier(0),
	mouseWheel(0),
	numNotifications(0),
	voteDownload(NULL),
	placingZoom(false),
	zoomEnabled(false),
	zoomedOnPosition(0, 0),
	zoomWindowPosition(0, 0),
	zoomSize(32),
	zoomFactor(8),
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

	// start placing the bottom row of buttons, starting from the left
#ifdef TOUCHUI
	const int ySize = 16;
	const int xOffset = 0;
	const int tooltipAlpha = 255;
#else
	const int ySize = 16;
	const int xOffset = 1;
	const int tooltipAlpha = -2;
#endif
	class OpenBrowserAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->OpenBrowser();
		}
	};
	openBrowserButton = new Button(Point(xOffset, YRES+MENUSIZE-16), Point(18-xOffset, ySize), "\x81");
	openBrowserButton->SetCallback(new OpenBrowserAction());
	openBrowserButton->SetTooltip(new ToolTip("Find & open a simulation", Point(16, YRES-24), TOOLTIP, tooltipAlpha));
	AddComponent(openBrowserButton);

	class ReloadAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->ReloadSave(b);
		}
	};
	reloadButton = new Button(openBrowserButton->Right(Point(1, 0)), Point(17, ySize), "\x91");
	reloadButton->SetCallback(new ReloadAction());
	reloadButton->SetEnabled(false);
#ifdef TOUCHUI
	reloadButton->SetState(Button::HOLD);
#endif
	reloadButton->SetTooltip(new ToolTip("Reload the simulation \bg(ctrl+r)", Point(16, YRES-24), TOOLTIP, tooltipAlpha));
	AddComponent(reloadButton);

	class SaveAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->DoSave();
		}
	};
	saveButton = new Button(reloadButton->Right(Point(1, 0)), Point(151, ySize), "\x82 [untitled simulation]");
	saveButton->SetAlign(Button::LEFT);
	saveButton->SetCallback(new SaveAction());
	saveButton->SetTooltip(new ToolTip("Upload a new simulation", Point(16, YRES-24), TOOLTIP, tooltipAlpha));
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
	upvoteButton = new Button(saveButton->Right(Point(1, 0)), Point(40, ySize), "\xCB Vote");
	upvoteButton->SetColor(COLRGB(0, 187, 18));
	upvoteButton->SetCallback(new VoteAction(true));
	upvoteButton->SetTooltip(new ToolTip("Like this save", Point(16, YRES-24), TOOLTIP, tooltipAlpha));
	AddComponent(upvoteButton);

	downvoteButton = new Button(upvoteButton->Right(Point(0, 0)), Point(16, ySize), "\xCA");
	downvoteButton->SetColor(COLRGB(187, 40, 0));
	downvoteButton->SetCallback(new VoteAction(false));
	downvoteButton->SetTooltip(new ToolTip("Disike this save", Point(16, YRES-24), TOOLTIP, tooltipAlpha));
	AddComponent(downvoteButton);


	// We now start placing buttons from the right side, because tags button is in the middle and uses whatever space is leftover
	class PauseAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->TogglePause();
		}
	};
	pauseButton = new Button(Point(XRES+BARSIZE-15-xOffset, openBrowserButton->GetPosition().Y), Point(15-xOffset, ySize), "\x90");
	pauseButton->SetCallback(new PauseAction());
	pauseButton->SetTooltip(new ToolTip("Pause the simulation \bg(space)", Point(16, YRES-24), TOOLTIP, tooltipAlpha));
	AddComponent(pauseButton);

	class RenderOptionsAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->RenderOptions();
		}
	};
	renderOptionsButton = new Button(pauseButton->Left(Point(18, 0)), Point(17, ySize), "\x0F\xFF\x01\x01\xD8\x0F\x01\xFF\x01\xD9\x0F\x01\x01\xFF\xDA");
	renderOptionsButton->SetCallback(new RenderOptionsAction());
	renderOptionsButton->SetTooltip(new ToolTip("Renderer options", Point(16, YRES-24), TOOLTIP, tooltipAlpha));
	AddComponent(renderOptionsButton);

	class LoginButtonAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->LoginButton();
		}
	};
	loginButton = new Button(renderOptionsButton->Left(Point(96, 0)), Point(95, ySize), "\x84 [sign in]");
	loginButton->SetAlign(Button::LEFT);
	loginButton->SetCallback(new LoginButtonAction());
	loginButton->SetTooltip(new ToolTip("Sign into the Simulation Server", Point(16, YRES-24), TOOLTIP, tooltipAlpha));
	AddComponent(loginButton);

	class ClearSimAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			NewSim();
		}
	};
	clearSimButton = new Button(loginButton->Left(Point(18, 0)), Point(17, ySize), "\x92");
	clearSimButton->SetCallback(new ClearSimAction());
	clearSimButton->SetTooltip(new ToolTip("Erase all particles and walls", Point(16, YRES-24), TOOLTIP, tooltipAlpha));
	AddComponent(clearSimButton);

	class OpenOptionsAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->OpenOptions();
		}
	};
	optionsButton = new Button(clearSimButton->Left(Point(16, 0)), Point(15, ySize), "\xCF");
	optionsButton->SetCallback(new OpenOptionsAction());
	optionsButton->SetTooltip(new ToolTip("Simulation options", Point(16, YRES-24), TOOLTIP, tooltipAlpha));
	AddComponent(optionsButton);

	class ReportBugAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->ReportBug();
		}
	};
	reportBugButton = new Button(optionsButton->Left(Point(16, 0)), Point(15, ySize), "\xE7");
	reportBugButton->SetCallback(new ReportBugAction());
	reportBugButton->SetTooltip(new ToolTip("Report bugs and feedback to jacob1", Point(16, YRES-24), TOOLTIP, tooltipAlpha));
	AddComponent(reportBugButton);

	class OpenTagsAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->OpenTags();
		}
	};
	Point tagsPos = downvoteButton->Right(Point(1, 0));
	openTagsButton = new Button(tagsPos, Point((reportBugButton->Left(Point(1, 0))-tagsPos).X, ySize), "\x83 [no tags set]");
	openTagsButton->SetAlign(Button::LEFT);
	openTagsButton->SetCallback(new OpenTagsAction());
	openTagsButton->SetTooltip(new ToolTip("Add simulation tags", Point(16, YRES-24), TOOLTIP, tooltipAlpha));
	AddComponent(openTagsButton);

#ifdef TOUCHUI
	class OpenConsoleAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->OpenConsole(b == 4);
		}
	};
	openConsoleButton = new Button(Point(XRES+1, 0), Point(BARSIZE-1, 25), "C");
	openConsoleButton->SetState(Button::HOLD);
	openConsoleButton->SetCallback(new OpenConsoleAction());
	AddComponent(openConsoleButton);

	class EraseAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->ToggleErase(b == 4);
		}
	};
	eraseButton = new Button(openConsoleButton->Below(Point(0, 1)), Point(BARSIZE-1, 25), "E");
	eraseButton->SetState(Button::HOLD);
	eraseButton->SetCallback(new EraseAction());
	AddComponent(eraseButton);

	class SettingAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->ToggleSetting(b == 4);
		}
	};
	settingsButton = new Button(eraseButton->Below(Point(0, 1)), Point(BARSIZE-1, 25), "N");
	settingsButton->SetState(Button::HOLD);
	settingsButton->SetCallback(new SettingAction());
	AddComponent(settingsButton);

	class ZoomAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->StartZoom(b == 4);
		}
	};
	zoomButton = new Button(settingsButton->Below(Point(0, 1)), Point(BARSIZE-1, 25), "Z");
	zoomButton->SetState(Button::HOLD);
	zoomButton->SetCallback(new ZoomAction());
	AddComponent(zoomButton);

	class StampAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->SaveStamp(b == 4);
		}
	};
	stampButton = new Button(zoomButton->Below(Point(0, 1)), Point(BARSIZE-1, 25), "S");
	stampButton->SetState(Button::HOLD);
	stampButton->SetCallback(new StampAction());
	AddComponent(stampButton);
#endif
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
	if (voteDownload != NULL)
		return;
	voteDownload = new Download("http://" SERVER "/Vote.api");
	voteDownload->AuthHeaders(svf_user_id, svf_session_id);
	std::map<std::string, std::string> postData;
	postData.insert(std::pair<std::string, std::string>("ID", svf_id));
	postData.insert(std::pair<std::string, std::string>("Action", up ? "Up" : "Down"));
	voteDownload->AddPostData(postData);
	voteDownload->Start();
	svf_myvote = up ? 1 : -1; // will be reset later upon error
}

void PowderToy::OpenTags()
{
	tag_list_ui(vid_buf);
}

void PowderToy::ReportBug()
{
	report_ui(vid_buf, NULL, true);
}

void PowderToy::OpenOptions()
{
	simulation_ui(vid_buf);
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

#ifdef TOUCHUI
void PowderToy::OpenConsole(bool alt)
{
	if (alt)
		ShowOnScreenKeyboard("");
	else
		console_mode = 1;
}

void PowderToy::ToggleErase(bool alt)
{
	if (alt)
		NewSim();
	else
	{
		Tool *temp = activeTools[1];
		activeTools[1] = activeTools[0];
		activeTools[0] = temp;
	}
}

void PowderToy::ToggleSetting(bool alt)
{
	if (alt)
		simulation_ui(vid_buf);
	else
	{
		if (active_menu == SC_DECO)
			decorations_enable = !decorations_enable;
		else
		{
			if (!ngrav_enable)
				start_grav_async();
			else
				stop_grav_async();
			ngrav_enable = !ngrav_enable;
		}
	}
}

void PowderToy::StartZoom(bool alt)
{
	if (ZoomWindowShown() || placingZoomTouch)
		HideZoomWindow();
	else
	{
		placingZoomTouch = true;
		UpdateZoomCoordinates(mouse);
	}
}

void PowderToy::SaveStamp(bool alt)
{
	/*if (alt)
	{
		if (load_mode)
		{
			free(load_img);
			free(load_data);
			load_mode = 0;
			load_data = NULL;
			load_img = NULL;
			return;
		}
		UpdateToolTip(it_msg, Point(16, 20), INTROTIP, 255);

		int reorder = 1;
		int stampID = stamp_ui(vid_buf, &reorder);
		if (stampID >= 0)
			load_data = stamp_load(stampID, &load_size, reorder);
		else
			load_data = NULL;

		if (load_data)
		{
			load_img = prerender_save(load_data, load_size, &load_w, &load_h);
			if (load_img)
				load_mode = 1;
			else
			{
				free(load_data);
				load_data = NULL;
			}
		}
	}
	else
	{
		UpdateToolTip(it_msg, Point(16, 20), INTROTIP, 255);
		save_mode = 1;
	}*/
}

#endif

void PowderToy::ConfirmUpdate()
{
	confirm_update(changelog.c_str());
}

bool PowderToy::MouseClicksIgnored()
{
	return PlacingZoomWindow();
}

void PowderToy::UpdateZoomCoordinates(Point mouse)
{
	int zoomX = mouse.X-zoomSize/2;
	int zoomY = mouse.Y-zoomSize/2;
	if (zoomX < 0)
		zoomX = 0;
	else if (zoomX > XRES-zoomSize)
		zoomX = XRES-zoomSize;
	if (zoomY < 0)
		zoomY = 0;
	else if (zoomY > YRES-zoomSize)
		zoomY = YRES-zoomSize;
	zoomedOnPosition = Point(zoomX, zoomY);

	if (mouse.X < XRES/2)
		zoomWindowPosition = Point(XRES-zoomSize*zoomFactor-1, 1);
	else
		zoomWindowPosition = Point(1, 1);
}

void PowderToy::HideZoomWindow()
{
	placingZoom = false;
	placingZoomTouch = false;
	zoomEnabled = false;
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
#include <iostream>
void PowderToy::OnTick(uint32_t ticks)
{
	int mouseX, mouseY;
	int mouseDown = mouse_get_state(&mouseX, &mouseY);
	main_loop_temp(mouseDown, lastMouseDown, heldKey, releasedKey, heldModifier, mouseX, mouseY, mouseWheel);
	lastMouseDown = mouseDown;
	heldKey = releasedKey = mouseWheel = 0;

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

	// a ton of stuff with the buttons on the bottom row has to be updated
	// later, this will only be done when an event happens
	reloadButton->SetEnabled(svf_last ? true : false);
	bool ctrl = (heldModifier & (KMOD_CTRL|KMOD_META)) ? true : false;
	openBrowserButton->SetState(ctrl ? Button::INVERTED : Button::NORMAL);
	saveButton->SetState((svf_login && ctrl) ? Button::INVERTED : Button::NORMAL);
	std::string saveButtonText = "\x82 ";
	std::string saveButtonTip;
	if (!svf_login || ctrl)
	{
		// button text
		if (svf_fileopen)
			saveButtonText += svf_filename;
		else
			saveButtonText += "[save to disk]";

		// button tooltip
		if (svf_fileopen && mouse.X <= saveButton->GetPosition().X+18)
			saveButtonTip = "Overwrite the open simulation on your hard drive.";
		else
		{
			if (!svf_login)
				saveButtonTip = "Save the simulation to your hard drive. Login to save online.";
			else
				saveButtonTip = "Save the simulation to your hard drive";
		}
	}
	else
	{
		// button text
		if (svf_open)
			saveButtonText += svf_name;
		else
			saveButtonText += "[untitled simulation]";

		// button tooltip
		if (svf_open && svf_own)
		{
			if (mouse.X <= saveButton->GetPosition().X+18)
				saveButtonTip = "Reupload the current simulation";
			else
				saveButtonTip = "Modify simulation properties";
		}
		else
			saveButtonTip = "Upload a new simulation";
	}
	saveButton->SetText(saveButtonText);
	saveButton->SetTooltipText(saveButtonTip);

	bool votesAllowed = svf_login && svf_open && svf_own == 0 && svf_myvote == 0;
	upvoteButton->SetEnabled(votesAllowed && voteDownload == NULL);
	downvoteButton->SetEnabled(votesAllowed && voteDownload == NULL);
	upvoteButton->SetState(svf_myvote == 1 ? Button::HIGHLIGHTED : Button::NORMAL);
	downvoteButton->SetState(svf_myvote == -1 ? Button::HIGHLIGHTED : Button::NORMAL);
	if (svf_myvote == 1)
	{
		upvoteButton->SetTooltipText("You like this");
		downvoteButton->SetTooltipText("You like this");
	}
	else if (svf_myvote == -1)
	{
		upvoteButton->SetTooltipText("You dislike this");
		downvoteButton->SetTooltipText("You dislike this");
	}
	else
	{
		upvoteButton->SetTooltipText("Like this save");
		downvoteButton->SetTooltipText("Dislike this save");
	}

	if (svf_tags[0])
		openTagsButton->SetText(svf_tags);
	else
		openTagsButton->SetText("\x83 [no tags set]");
	openTagsButton->SetEnabled(svf_open);
	if (svf_own)
		openTagsButton->SetTooltipText("Add and remove simulation tags");
	else
		openTagsButton->SetTooltipText("Add simulation tags");

	// set login button text, key turns green or red depending on whether session check succeeded
	std::string loginButtonText;
	std::string loginButtonTip;
	if (svf_login)
	{
		if (loginFinished == 1)
		{
			loginButtonText = "\x0F\x01\xFF\x01\x84\x0E " + std::string(svf_user);
			if (mouse.X <= loginButton->GetPosition().X+18)
				loginButtonTip = "View and edit your profile";
			else if (svf_mod && mouse.X >= loginButton->Right(Point(-15, 0)).X)
				loginButtonTip = "You're a moderator";
			else if (svf_admin && mouse.X >= loginButton->Right(Point(-15, 0)).X)
				loginButtonTip = "Annuit C\245ptis";
			else
				loginButtonTip = "Sign into the simulation server under a new name";
		}
		else if (loginFinished == -1)
		{
			loginButtonText = "\x0F\xFF\x01\x01\x84\x0E " + std::string(svf_user);
			loginButtonTip = "Could not validate login";
		}
		else
		{
			loginButtonText = "\x84 " + std::string(svf_user);
			loginButtonTip = "Waiting for login server ...";
		}
	}
	else
	{
		loginButtonText = "\x84 [sign in]";
		loginButtonTip = "Sign into the Simulation Server";
	}
	loginButton->SetText(loginButtonText);
	loginButton->SetTooltipText(loginButtonTip);

	pauseButton->SetState(sys_pause ? Button::INVERTED : Button::NORMAL);
	if (sys_pause)
		pauseButton->SetTooltipText("Resume the simulation \bg(space)");
	else
		pauseButton->SetTooltipText("Pause the simulation \bg(space)");

	if (placingZoomTouch)
		UpdateToolTip("\x0F\xEF\xEF\020Click any location to place a zoom window (volume keys to resize, click zoom button to cancel)", Point(16, YRES-24), TOOLTIP, 255);
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
		// amd logo
		/*else if (true)
		{
			Point iconPos = loginButton->Right(Point(-12, 3));
			buf->DrawText(iconPos.X, iconPos.Y, "\x97", 0, 230, 153, 255);
		}*/
	}
}

void PowderToy::OnMouseMove(int x, int y, Point difference)
{
	mouse = Point(x, y);
	if (placingZoom)
		UpdateZoomCoordinates(mouse);
}

void PowderToy::OnMouseDown(int x, int y, unsigned char button)
{
	if (placingZoomTouch)
	{
		if (x < XRES && y < YRES)
		{
			placingZoomTouch = false;
			placingZoom = true;
			UpdateZoomCoordinates(mouse);
		}
	}
}

void PowderToy::OnMouseUp(int x, int y, unsigned char button)
{
	if (placingZoom)
	{
		placingZoom = false;
		zoomEnabled = true;
	}
}

void PowderToy::OnMouseWheel(int x, int y, int d)
{
	mouseWheel += d;
	if (PlacingZoomWindow())
	{
		zoomSize += d;
		zoomSize = std::max(2, std::min(zoomSize, 60));
		zoomFactor = 256/zoomSize;
	}
}

void PowderToy::OnKeyPress(int key, unsigned short character, unsigned short modifiers)
{
	if ((modifiers & (KMOD_CTRL|KMOD_META)) && !(heldModifier & (KMOD_CTRL|KMOD_META)))
		openBrowserButton->SetTooltipText("Open a simulation from your hard drive \bg(ctrl+o)");

	heldModifier = modifiers;
	// key -1 is fake event sent in order to update modifiers when in other interfaces
	if (key == -1)
		return;
	heldKey = key;

#ifdef LUACONSOLE
	if (key != -1 && !deco_disablestuff && !luacon_keyevent(key, modifiers, LUACON_KDOWN))
		key = 0;
#endif

	// lua can disable all key shortcuts
	if (!sys_shortcuts)
		return;

	switch (key)
	{
	case 'q':
	case SDLK_ESCAPE:
		if (confirm_ui(vid_buf, "You are about to quit", "Are you sure you want to quit?", "Quit"))
		{
			this->ignoreQuits = false;
			this->toDelete = true;
		}
		break;
	case 'z':
		// don't do anything if this is a ctrl+z (undo)
		if (modifiers & (KMOD_CTRL|KMOD_META))
			break;
		if (ZoomWindowShown())
		{
			HideZoomWindow();
		}
		else
		{
			placingZoom = true;
			UpdateZoomCoordinates(mouse);
		}
		break;
	case SDLK_LEFTBRACKET:
		if (PlacingZoomWindow())
		{
			zoomSize -= 1;
			zoomSize = std::max(2, std::min(zoomSize, 60));
			zoomFactor = 256/zoomSize;
		}
		break;
	case SDLK_RIGHTBRACKET:
		if (PlacingZoomWindow())
		{
			zoomSize += 1;
			zoomSize = std::max(2, std::min(zoomSize, 60));
			zoomFactor = 256/zoomSize;
		}
		break;
	}
}

void PowderToy::OnKeyRelease(int key, unsigned short character, unsigned short modifiers)
{
	if (!(modifiers & (KMOD_CTRL|KMOD_META)) && (heldModifier & (KMOD_CTRL|KMOD_META)))
		openBrowserButton->SetTooltipText("Find & open a simulation");

	heldModifier = modifiers;
	// key -1 is fake event sent in order to update modifiers when in other interfaces
	if (key == -1)
		return;
	releasedKey = key;

#ifdef LUACONSOLE
	if (!deco_disablestuff && !luacon_keyevent(key, modifiers, LUACON_KUP))
		key = 0;
#endif

	switch (key)
	{
	case 'z':
		if (placingZoom)
			HideZoomWindow();
		break;
	}
}
