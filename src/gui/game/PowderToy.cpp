#include <algorithm>
#include <sstream>
#include "SDLCompat.h"
#include "json/json.h"

#include "PowderToy.h"
#include "defines.h"
#include "interface.h"
#include "gravity.h"
#include "luaconsole.h"
#include "powder.h"
#include "misc.h"
#include "save.h"
#include "update.h"

#include "common/Platform.h"
#include "game/Download.h"
#include "game/Menus.h"
#include "game/Sign.h"
#include "game/ToolTip.h"
#include "graphics/VideoBuffer.h"
#include "interface/Button.h"
#include "interface/Engine.h"
#include "interface/Window.h"
#include "simulation/Simulation.h"

#include "gui/profile/ProfileViewer.h"
#include "gui/sign/CreateSign.h"

PowderToy::~PowderToy()
{
	main_end_hack();
	free(clipboardData);
}

PowderToy::PowderToy():
	Window_(Point(0, 0), Point(XRES+BARSIZE, YRES+MENUSIZE)),
	mouse(Point(0, 0)),
	cursor(Point(0, 0)),
	lastMouseDown(0),
	heldKey(0),
	heldAscii(0),
	releasedKey(0),
	heldModifier(0),
	mouseWheel(0),
	mouseCanceled(false),
	numNotifications(0),
	voteDownload(NULL),
	placingZoom(false),
	placingZoomTouch(false),
	zoomEnabled(false),
	zoomedOnPosition(0, 0),
	zoomWindowPosition(0, 0),
	zoomSize(32),
	zoomFactor(8),
	state(NONE),
	loadPos(Point(0, 0)),
	loadSize(Point(0, 0)),
	savedInitial(false),
	stampData(NULL),
	stampSize(0),
	stampImg(NULL),
	waitToDraw(false),
#ifdef TOUCHUI
	stampClickedPos(Point(0, 0)),
	stampClickedOffset(Point(0, 0)),
	initialLoadPos(Point(0, 0)),
	stampQuadrant(0),
	stampMoving(false),
#endif
	savePos(Point(0, 0)),
	saveSize(Point(0, 0)),
	clipboardData(NULL),
	clipboardSize(0),
	loginCheckTicks(0),
	loginFinished(0),
	ignoreMouseUp(false)
{
	ignoreQuits = true;

#ifndef NOMOD
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
#endif
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
	const int ySize = 15;
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
	pauseButton = new Button(Point(XRES+BARSIZE-15-xOffset, openBrowserButton->GetPosition().Y), Point(15, ySize), "\x90");
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
	class EraseAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->ToggleErase(b == 4);
		}
	};
	eraseButton = new Button(Point(XRES+1, 0), Point(BARSIZE-1, 25), "\xE8");
	eraseButton->SetState(Button::HOLD);
	eraseButton->SetCallback(new EraseAction());
	eraseButton->SetTooltip(GetQTip("Swap to erase tool (hold to clear the sim)", eraseButton->GetPosition().Y+10));
	AddComponent(eraseButton);

	class OpenConsoleAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->OpenConsole(b == 4);
		}
	};
	openConsoleButton = new Button(eraseButton->Below(Point(0, 1)), Point(BARSIZE-1, 25), "\xE9");
	openConsoleButton->SetState(Button::HOLD);
	openConsoleButton->SetCallback(new OpenConsoleAction());
	openConsoleButton->SetTooltip(GetQTip("Open console (hold to show on screen keyboard)", openConsoleButton->GetPosition().Y+10));
	AddComponent(openConsoleButton);

	class SettingAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->ToggleSetting(b == 4);
		}
	};
	settingsButton = new Button(openConsoleButton->Below(Point(0, 1)), Point(BARSIZE-1, 25), "\xEB");
	settingsButton->SetState(Button::HOLD);
	settingsButton->SetCallback(new SettingAction());
	settingsButton->SetTooltip(GetQTip("Toggle Decorations (hold to open options)", settingsButton->GetPosition().Y+10));
	AddComponent(settingsButton);

	class ZoomAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->StartZoom(b == 4);
		}
	};
	zoomButton = new Button(settingsButton->Below(Point(0, 1)), Point(BARSIZE-1, 25), "\xEC");
	zoomButton->SetState(Button::HOLD);
	zoomButton->SetCallback(new ZoomAction());
	zoomButton->SetTooltip(GetQTip("Start placing the zoom window", zoomButton->GetPosition().Y+10));
	AddComponent(zoomButton);

	class StampAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<PowderToy*>(button->GetParent())->SaveStamp(b == 4);
		}
	};
	stampButton = new Button(zoomButton->Below(Point(0, 1)), Point(BARSIZE-1, 25), "\xEA");
	stampButton->SetState(Button::HOLD);
	stampButton->SetCallback(new StampAction());
	stampButton->SetTooltip(GetQTip("Save a stamp (hold to load a stamp)", stampButton->GetPosition().Y+10));
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
				SetInfoTip("Error creating save");
			}
			else
			{
				if (DoLocalSave(svf_filename, saveData, saveSize, true))
					SetInfoTip("Error writing local save");
				else
					SetInfoTip("Updated successfully");
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
					SetInfoTip("Error saving");
				}
			}
		}
		// local quick save
		else
		{
			if (execute_save(vid_buf))
			{
				SetInfoTip("Error saving");
			}
			else
			{
				SetInfoTip("Saved successfully");
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

// functions called by touch interface buttons are here
#ifdef TOUCHUI
void PowderToy::ToggleErase(bool alt)
{
	if (alt)
	{
		NewSim();
		SetInfoTip("Cleared the simulation");
	}
	else
	{
		Tool *erase = GetToolFromIdentifier("DEFAULT_PT_NONE");
		if (activeTools[0] == erase)
		{
			activeTools[0] = activeTools[1];
			activeTools[1] = erase;
			SetInfoTip("Erase tool deselected");
		}
		else
		{
			activeTools[1] = activeTools[0];
			activeTools[0] = erase;
			SetInfoTip("Erase tool selected");
		}
	}
}

void PowderToy::OpenConsole(bool alt)
{
	if (alt)
		Platform::ShowOnScreenKeyboard("");
	else
		console_mode = 1;
}

void PowderToy::ToggleSetting(bool alt)
{
	if (alt)
		simulation_ui(vid_buf);
	else
	{
		//if (active_menu == SC_DECO)
		{
			decorations_enable = !decorations_enable;
			if (decorations_enable)
				SetInfoTip("Decorations enabled");
			else
				SetInfoTip("Decorations disabled");
		}
		/*else
		{
			if (!ngrav_enable)
			{
				start_grav_async();
				SetInfoTip("Newtonian Gravity enabled");
			}
			else
			{
				stop_grav_async();
				SetInfoTip("Newtonian Gravity disabled");
			}
		}*/
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
	if (alt)
	{
		// if we are loading a stamp, cancel that and free all memory
		if (state == LOAD)
		{
			free(stampData);
			free(stampImg);
			stampImg = NULL;
			state = NONE;
			stampMoving = false;
		}

		int reorder = 1;
		int stampID = stamp_ui(vid_buf, &reorder);
		if (stampID >= 0)
			stampData = stamp_load(stampID, &stampSize, reorder);
		else
			stampData = NULL;

		if (stampData)
		{
			stampImg = prerender_save(stampData, stampSize, &loadSize.X, &loadSize.Y);
			if (stampImg)
			{
				state = LOAD;
				loadPos.X = CELL*((XRES-loadSize.X+CELL)/2/CELL);
				loadPos.Y = CELL*((YRES-loadSize.Y+CELL)/2/CELL);
				stampClickedPos = Point(XRES, YRES)/2;
				ignoreMouseUp = true;
				waitToDraw = true;
			}
			else
			{
				free(stampData);
				stampData = NULL;
			}
		}
	}
	else
	{
		// if we are loading a stamp, cancel that and free all memory
		if (state == LOAD)
		{
			free(stampData);
			stampData = NULL;
			free(stampImg);
			stampImg = NULL;
			state = NONE;
		}
		else if (state == SAVE || state == COPY || state == CUT)
		{
			state = NONE;
		}
		else
		{
			state = SAVE;
			savedInitial = false;
			ignoreMouseUp = true;
		}
	}
}

#endif

// misc main gui functions
void PowderToy::ConfirmUpdate()
{
	confirm_update(changelog.c_str());
}

bool PowderToy::MouseClicksIgnored()
{
	return PlacingZoomWindow() || state != NONE;
}

Point PowderToy::AdjustCoordinates(Point mouse)
{
	//adjust coords into the simulation area
	mouse.Clamp(Point(0, 0), Point(XRES-1, YRES-1));

	//Change mouse coords to take zoom window into account
	if (ZoomWindowShown())
	{
		if (mouse >= zoomWindowPosition && mouse < Point(zoomWindowPosition.X+zoomFactor*zoomSize, zoomWindowPosition.Y+zoomFactor*zoomSize))
		{
			mouse.X = ((mouse.X-zoomWindowPosition.X)/zoomFactor) + zoomedOnPosition.X;
			mouse.Y = ((mouse.Y-zoomWindowPosition.Y)/zoomFactor) + zoomedOnPosition.Y;
		}
	}
	return mouse;
}

void PowderToy::SetInfoTip(std::string infotip)
{
	UpdateToolTip(infotip, Point(XCNTR-VideoBuffer::TextSize(infotip).X/2, YCNTR-10), INFOTIP, 1000);
}

ToolTip * PowderToy::GetQTip(std::string qtip, int y)
{
	return new ToolTip(qtip, Point(XRES-5-VideoBuffer::TextSize(qtip).X, y), QTIP, -2);
}

void PowderToy::UpdateZoomCoordinates(Point mouse)
{
	zoomedOnPosition = mouse-Point(zoomSize/2, zoomSize/2);
	zoomedOnPosition.Clamp(Point(0, 0), Point(XRES-zoomSize, YRES-zoomSize));

	if (mouse.X < XRES/2)
		zoomWindowPosition = Point(XRES-zoomSize*zoomFactor-1, 1);
	else
		zoomWindowPosition = Point(1, 1);
}

void PowderToy::UpdateStampCoordinates(Point cursor, Point offset)
{
	loadPos.X = CELL*((cursor.X-loadSize.X/2+CELL/2)/CELL);
	loadPos.Y = CELL*((cursor.Y-loadSize.Y/2+CELL/2)/CELL);
	loadPos -= offset;
	loadPos.Clamp(Point(0, 0), Point(XRES, YRES)-loadSize);
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

// Engine events
void PowderToy::OnTick(uint32_t ticks)
{
	int mouseX, mouseY;
	int mouseDown = mouse_get_state(&mouseX, &mouseY);
#ifdef LUACONSOLE
	// lua mouse "tick", call the function every frame. When drawing is rewritten, this needs to be changed to cancel drawing.
	if (mouseDown && !luacon_mouseevent(mouseX, mouseY, mouseDown, LUACON_MPRESS, 0))
		mouseCanceled = true;
	if (mouseCanceled)
		mouseDown = 0;
#endif
	sdl_key = heldKey; // ui_edit_process in deco editor uses these two globals so we have to set them ):
	sdl_ascii = heldAscii;
	main_loop_temp(mouseDown, lastMouseDown, heldKey, releasedKey, heldModifier, mouseX, mouseY, mouseWheel);
	lastMouseDown = mouseDown;
	heldKey = heldAscii = releasedKey = mouseWheel = 0;

	if (!loginFinished)
		loginCheckTicks = (loginCheckTicks+1)%51;
	waitToDraw = false;

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
							Platform::OpenLink(link);
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
		free(ret);
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
		// if currently moving a sign, stop doing so
		if (MSIGN != -1)
			MSIGN = -1;
		else
		{
			Point cursor = AdjustCoordinates(Point(mouseX, mouseY));
			int signID = InsideSign(cursor.X, cursor.Y, true);
			if (signID == -1 && signs.size() >= MAXSIGNS)
				SetInfoTip("Sign limit reached");
			else
				Engine::Ref().ShowWindow(new CreateSign(signID, cursor));
		}
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
		openTagsButton->SetText("\x83 " + std::string(svf_tags));
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
		UpdateToolTip("\x0F\xEF\xEF\020Tap any location to place a zoom window (volume keys to resize, click zoom button to cancel)", Point(16, YRES-24), TOOLTIP, 255);
#ifdef TOUCHUI
	if (state == SAVE || state == COPY)
		UpdateToolTip("\x0F\xEF\xEF\020Click-and-drag to specify a rectangle to copy (click save button to cancel)", Point(16, YRES-24), TOOLTIP, 255);
	else if (state == CUT)
		UpdateToolTip("\x0F\xEF\xEF\020Click-and-drag to specify a rectangle to copy and then cut (click save button to cancel)", Point(16, YRES-24), TOOLTIP, 255);
	else if (state == LOAD)
		UpdateToolTip("\x0F\xEF\xEF\020Drag the stamp around to move it, and tap it to place. Tap or drag outside the stamp to shift and rotate.", Point(16, YRES-24), TOOLTIP, 255);
#else
	if (state == SAVE || state == COPY)
		UpdateToolTip("\x0F\xEF\xEF\020Click-and-drag to specify a rectangle to copy (right click = cancel)", Point(16, YRES-24), TOOLTIP, 255);
	else if (state == CUT)
		UpdateToolTip("\x0F\xEF\xEF\020Click-and-drag to specify a rectangle to copy and then cut (right click = cancel)", Point(16, YRES-24), TOOLTIP, 255);
#endif
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
#ifdef LUACONSOLE
	luacon_step(mouse.X, mouse.Y);
	ExecuteEmbededLuaCode();
#endif
}

void PowderToy::OnMouseMove(int x, int y, Point difference)
{
	mouse = Point(x, y);
	cursor = AdjustCoordinates(mouse);
	if (placingZoom)
		UpdateZoomCoordinates(mouse);
	if (state == LOAD)
	{
#ifdef TOUCHUI
		if (stampMoving)
			UpdateStampCoordinates(cursor, stampClickedOffset);
#else
		UpdateStampCoordinates(cursor);
#endif
	}
	else if (state == SAVE || state == COPY || state == CUT)
	{
		if (savedInitial)
		{
			saveSize.X = cursor.X + 1 - savePos.X;
			saveSize.Y = cursor.Y + 1 - savePos.Y;
			if (savePos.X + saveSize.X < 0)
				saveSize.X = 0;
			else if (savePos.X + saveSize.X > XRES)
				saveSize.X = XRES - savePos.X;
			if (savePos.Y + saveSize.Y < 0)
				saveSize.Y = 0;
			else if (savePos.Y + saveSize.Y > YRES)
				saveSize.Y = YRES - savePos.Y;
		}
	}

	// moving sign, update coordinates here
	if (MSIGN >= 0 && MSIGN < signs.size())
	{
		signs[MSIGN]->SetPos(cursor);
	}
}

bool PowderToy::BeforeMouseDown(int x, int y, unsigned char button)
{
#ifdef LUACONSOLE
	// lua mouse event, cancel mouse action if the function returns false
	if (!luacon_mouseevent(x, y, button, LUACON_MDOWN, 0))
		return false;
#endif
	return true;
}

void PowderToy::OnMouseDown(int x, int y, unsigned char button)
{
	mouse = Point(x, y);
	cursor = AdjustCoordinates(mouse);
	if (placingZoomTouch)
	{
		if (x < XRES && y < YRES)
		{
			placingZoomTouch = false;
			placingZoom = true;
			UpdateZoomCoordinates(mouse);
		}
	}
#ifdef TOUCHUI
	else if (state == LOAD)
	{
		stampClickedPos = cursor;
		initialLoadPos = loadPos;
		UpdateStampCoordinates(cursor);
		stampClickedOffset = loadPos-initialLoadPos;
		loadPos -= stampClickedOffset;
		if (cursor.IsInside(loadPos, loadPos+loadSize))
			stampMoving = true;
		// calculate which side of the stamp this touch is on
		else
		{
			int xOffset = (loadSize.X-loadSize.Y)/2;
			Point diff = cursor-(loadPos+loadSize/2);
			if (std::abs(diff.X)-xOffset > std::abs(diff.Y))
				stampQuadrant = (diff.X > 0) ? 3 : 1; // right : left
			else
				stampQuadrant = (diff.Y > 0) ? 2 : 0; // down : up
		}
	}
#endif
	else if (state == SAVE || state == COPY || state == CUT)
	{
		// right click cancel
		if (button == 4)
		{
			state = NONE;
		}
		// placing initial coordinate
		else if (!savedInitial)
		{
			savePos = cursor;
			saveSize = Point(1, 1);
			savedInitial = true;
		}
	}
}

bool PowderToy::BeforeMouseUp(int x, int y, unsigned char button)
{
	mouseCanceled = false;
#ifdef LUACONSOLE
	// lua mouse event, cancel mouse action if the function returns false
	if (!luacon_mouseevent(x, y, button, LUACON_MUP, 0))
		return false;
#endif
	return true;
}

void PowderToy::OnMouseUp(int x, int y, unsigned char button)
{
	mouse = Point(x, y);
	cursor = AdjustCoordinates(mouse);
	if (placingZoom)
	{
		placingZoom = false;
		zoomEnabled = true;
	}
	else if (ignoreMouseUp)
	{
		// ignore mouse up when some touch ui buttons on the right side are pressed
		ignoreMouseUp = false;
	}
	else if (state == LOAD)
	{
		if (button == 4 || y >= YRES+MENUSIZE-16)
		{
			free(stampData);
			stampData = NULL;
			free(stampImg);
			stampImg = NULL;
			state = NONE;
#ifdef TOUCHUI
			stampMoving = false;
#endif
			return;
		}
#ifdef TOUCHUI
		if (loadPos != initialLoadPos)
		{
			stampMoving = false;
			return;
		}
		else if (cursor.IsInside(Point(0, 0), Point(XRES, YRES)) && !cursor.IsInside(loadPos, loadPos+loadSize))
		{
			// figure out which side this touch started and ended on (arbitrary direction numbers)
			// imagine 4 quadrants coming out of the stamp, with the edges diagonally starting directly from each corner
			// if you tap the screen in one of these corners, it shifts the stamp one pixel in that direction
			// if you move between quadrants you can rotate in that direction
			// or flip horizontally / vertically by dragging accross the stamp without touching the side quadrants
			int quadrant, xOffset = (loadSize.X-loadSize.Y)/2;
			Point diff = cursor-(loadPos+loadSize/2);
			if (std::abs(diff.X)-xOffset > std::abs(diff.Y))
				quadrant = (diff.X > 0) ? 3 : 1; // right : left
			else
				quadrant = (diff.Y > 0) ? 2 : 0; // down : up

			matrix2d transform = m2d_identity;
			vector2d translate = v2d_zero;
			// shift (arrow keys)
			if (quadrant == stampQuadrant)
				translate = v2d_new((quadrant-2)%2, (quadrant-1)%2);
			// rotate 90 degrees
			else if (quadrant%2 != stampQuadrant%2)
				transform = m2d_new(0, (quadrant-stampQuadrant+1)%4 == 0 ? -1 : 1, (quadrant-stampQuadrant+1)%4 == 0 ? 1 : -1, 0);
			// flip 180
			else
				transform = m2d_new((quadrant%2)*-2+1, 0, 0, (quadrant%2)*2-1);

			// actual transformation is done here
			void *newData = transform_save(stampData, &stampSize, transform, translate);
			if (!newData)
				return;
			free(stampData);
			stampData = newData;
			free(stampImg);
			stampImg = prerender_save(stampData, stampSize, &loadSize.X, &loadSize.Y);
			return;
		}
		else if (!stampMoving)
			return;
		stampMoving = false;
#endif
		ctrlzSnapshot();
		parse_save(stampData, stampSize, 0, loadPos.X, loadPos.Y, bmap, vx, vy, pv, fvx, fvy, signs, parts, pmap);
		free(stampData);
		stampData = NULL;
		free(stampImg);
		stampImg = NULL;
		state = NONE;
	}
	else if (state == SAVE || state == COPY || state == CUT)
	{
		// already placed initial coordinate. If they haven't ... no idea what happened here
		// mouse could be 4 if strange stuff with zoom window happened so do nothing and reset state in that case too
		if (savedInitial && button == 1)
		{
			// make sure size isn't negative
			if (saveSize.X < 0)
			{
				savePos.X = savePos.X + saveSize.X - 1;
				saveSize.X = abs(saveSize.X) + 2;
			}
			if (saveSize.Y < 0)
			{
				savePos.Y = savePos.Y + saveSize.Y - 1;
				saveSize.Y = abs(saveSize.Y) + 2;
			}
			if (saveSize.X > 0 && saveSize.Y > 0)
			{
				switch (state)
				{
				case COPY:
					free(clipboardData);
					clipboardData = build_save(&clipboardSize, savePos.X, savePos.Y, saveSize.X, saveSize.Y, bmap, vx, vy, pv, fvx, fvy, signs, parts);
					break;
				case CUT:
					free(clipboardData);
					clipboardData = build_save(&clipboardSize, savePos.X, savePos.Y, saveSize.X, saveSize.Y, bmap, vx, vy, pv, fvx, fvy, signs, parts);
					if (clipboardData)
						clear_area(savePos.X, savePos.Y, saveSize.X, saveSize.Y);
					break;
				case SAVE:
					// function returns the stamp name which we don't want, so free it
					free(stamp_save(savePos.X, savePos.Y, saveSize.X, saveSize.Y));
					break;
				}
			}
		}
		state = NONE;
	}
}

bool PowderToy::BeforeMouseWheel(int x, int y, int d)
{
#ifdef LUACONSOLE
	int mouseX, mouseY;
	// lua mouse event, cancel mouse action if the function returns false
	if (!luacon_mouseevent(x, y, mouse_get_state(&mouseX, &mouseY), 0, d))
		return false;
#endif
	return true;
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

bool PowderToy::BeforeKeyPress(int key, unsigned short character, unsigned short modifiers)
{
	if ((modifiers & (KMOD_CTRL|KMOD_META)) && !(heldModifier & (KMOD_CTRL|KMOD_META)))
		openBrowserButton->SetTooltipText("Open a simulation from your hard drive \bg(ctrl+o)");

	heldModifier = modifiers;
	// key -1 is fake event sent in order to update modifiers when in other interfaces
	if (key == -1)
		return false;
	heldKey = key;
	heldAscii = character;
	// do nothing when deco textboxes are selected
	if (deco_disablestuff)
		return true;

#ifdef LUACONSOLE
	if (!luacon_keyevent(key, modifiers, LUACON_KDOWN))
	{
		heldKey = 0;
		return false;
	}
#endif

	// lua can disable all key shortcuts
	if (!sys_shortcuts)
		return false;

	return true;
}

void PowderToy::OnKeyPress(int key, unsigned short character, unsigned short modifiers)
{
	if (deco_disablestuff)
		return;

	// loading a stamp, special handling here
	// if stamp was transformed, key presses get ignored
	if (state == LOAD)
	{
		matrix2d transform = m2d_identity;
		vector2d translate = v2d_zero;
		bool doTransform = true;

		switch (key)
		{
		case 'r':
			// vertical invert
			if ((modifiers & (KMOD_CTRL|KMOD_META)) && (sdl_mod & KMOD_SHIFT))
			{
				transform = m2d_new(1, 0, 0, -1);
			}
			// horizontal invert
			else if (modifiers & KMOD_SHIFT)
			{
				transform = m2d_new(-1, 0, 0, 1);
			}
			// rotate anticlockwise 90 degrees
			else
			{
				transform = m2d_new(0, 1, -1, 0);
			}
			break;
		case SDLK_LEFT:
			translate = v2d_new(-1, 0);
			break;
		case SDLK_RIGHT:
			translate = v2d_new(1, 0);
			break;
		case SDLK_UP:
			translate = v2d_new(0, -1);
			break;
		case SDLK_DOWN:
			translate = v2d_new(0, 1);
			break;
		default:
			doTransform = false;
		}

		if (doTransform)
		{
			void *newData = transform_save(stampData, &stampSize, transform, translate);
			if (!newData)
				return;
			free(stampData);
			stampData = newData;
			free(stampImg);
			stampImg = prerender_save(stampData, stampSize, &loadSize.X, &loadSize.Y);
			return;
		}
	}

	// handle normal keypresses
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
	case 's':
		//if stkm2 is out, you must be holding left ctrl, else not be holding ctrl at all
		if (globalSim->elementCount[PT_STKM2] > 0 ? (modifiers&(KMOD_LCTRL|KMOD_LMETA)) : !(sdl_mod&(KMOD_CTRL|KMOD_META)))
		{
			// if we are loading a stamp, cancel that and free all memory
			if (state == LOAD)
			{
				free(stampData);
				stampData = NULL;
				free(stampImg);
				stampImg = NULL;
			}
			state = SAVE;
			savedInitial = false;
		}
		break;
	case 'k':
	case 'l':
		// if we are placing a stamp, cancel that to load the new one
		if (state == LOAD)
		{
			free(stampData);
			free(stampImg);
			stampImg = NULL;
			state = NONE;
#ifdef TOUCHUI
			stampMoving = false;
#endif
		}
		// open stamp interface
		if (key == 'k')
		{
			int reorder = 1;
			int stampID = stamp_ui(vid_buf, &reorder);
			if (stampID >= 0)
				stampData = stamp_load(stampID, &stampSize, reorder);
			else
				stampData = NULL;
		}
		// else, open most recent stamp
		else
			stampData = stamp_load(0, &stampSize, 1);

		// if a stamp was actually loaded
		if (stampData)
		{
			int width, height;
			stampImg = prerender_save(stampData, stampSize, &width, &height);
			if (stampImg)
			{
				state = LOAD;
				loadSize = Point(width, height);
				waitToDraw = true;
				UpdateStampCoordinates(cursor);
			}
			else
			{
				free(stampData);
				stampData = NULL;
			}
		}
		else
			stampImg = NULL;
		break;
	case 'x':
		if (modifiers & (KMOD_CTRL|KMOD_META))
		{
			// if we are loading a stamp, cancel that and free all memory
			if (state == LOAD)
			{
				free(stampData);
				stampData = NULL;
				free(stampImg);
				stampImg = NULL;
			}
			state = CUT;
			savedInitial = false;
		}
		break;
	case 'c':
		if (modifiers & (KMOD_CTRL|KMOD_META))
		{
			// if we are loading a stamp, cancel that and free all memory
			if (state == LOAD)
			{
				free(stampData);
				stampData = NULL;
				free(stampImg);
				stampImg = NULL;
			}
			state = COPY;
			savedInitial = false;
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
	case 'v':
		if ((modifiers & (KMOD_CTRL|KMOD_META)) && clipboardData)
		{
			if (stampData)
				free(stampData);
			stampData = malloc(clipboardSize);
			if (stampData)
			{
				memcpy(stampData, clipboardData, clipboardSize);
				stampSize = clipboardSize;
				stampImg = prerender_save(stampData, stampSize, &loadSize.X, &loadSize.Y);
				if (stampImg)
				{
					state = LOAD;
					UpdateStampCoordinates(cursor);
				}
				else
				{
					free(stampData);
					stampData = NULL;
				}
			}
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

bool PowderToy::BeforeKeyRelease(int key, unsigned short character, unsigned short modifiers)
{
	if (!(modifiers & (KMOD_CTRL|KMOD_META)) && (heldModifier & (KMOD_CTRL|KMOD_META)))
		openBrowserButton->SetTooltipText("Find & open a simulation");

	heldModifier = modifiers;
	// key -1 is fake event sent in order to update modifiers when in other interfaces
	if (key == -1)
		return false;
	releasedKey = key;

	if (deco_disablestuff)
		return true;

#ifdef LUACONSOLE
	if (!luacon_keyevent(key, modifiers, LUACON_KUP))
	{
		releasedKey = 0;
		return false;
	}
#endif
	return true;
}

void PowderToy::OnKeyRelease(int key, unsigned short character, unsigned short modifiers)
{
	if (deco_disablestuff)
		return;

	switch (key)
	{
	case 'z':
		if (placingZoom)
			HideZoomWindow();
		break;
	}
}
