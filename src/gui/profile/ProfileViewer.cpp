#include <sstream>
#include "json/json.h"
#include "ProfileViewer.h"
#include "interface/Engine.h"
#include "interface/Label.h"
#include "interface/Textbox.h"
#include "interface/Button.h"
#include "graphics/VideoBuffer.h"
#include "common/Point.h"
#include "game/Download.h"
#include "interface.h"

ProfileViewer::ProfileViewer(std::string profileName):
	ScrollWindow(Point(CENTERED, CENTERED), Point(260, 350)),
	name(profileName),
	avatar(NULL),
	ageLabel(NULL),
	locationLabel(NULL),
	websiteLabel(NULL),
	biographyLabel(NULL),
	saveCountLabel(NULL),
	saveAverageLabel(NULL),
	highestVoteLabel(NULL)
{
	profileInfoDownload = new Download("http://" SERVER "/User.json?Name=" + name);
	//profileInfoDownload->AuthHeaders();
	profileInfoDownload->Start();

	avatarDownload = new Download("http://" STATICSERVER "/avatars/" + name + ".pti");
	avatarDownload->Start();

	usernameLabel = new Label(Point(7, 6), Point(Label::AUTOSIZE, Label::AUTOSIZE), name);
	this->AddComponent(usernameLabel);

	avatarUploadButton = new Button(Point(211, 11), Point(38, 38), "\n\x81");
	// Enable editing when this button is clicked
	class UploadAvatarAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button)
		{
			dynamic_cast<ProfileViewer*>(button->GetParent())->UploadAvatar();
		}
	};
	avatarUploadButton->SetCallback(new UploadAvatarAction());
	this->AddComponent(avatarUploadButton);

	// Enable editing when this button is clicked
	class EnableEditingAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button)
		{
			dynamic_cast<ProfileViewer*>(button->GetParent())->EnableEditing();
		}
	};
	enableEditingButton = new Button(Point(0, size.Y-15), Point(this->size.X/2-1, 15), "Enable Editing");
	enableEditingButton->SetCallback(new EnableEditingAction());
	enableEditingButton->SetEnabled(false);
	this->AddComponent(enableEditingButton);

	// Open profile online when this button is clicked
	class OpenProfileAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button)
		{
			dynamic_cast<ProfileViewer*>(button->GetParent())->OpenProfile();
		}
	};
	openProfileButton = new Button(Point(size.X/2, size.Y-15), Point(this->size.X/2, 15), "Open Profile Online");
	openProfileButton->SetCallback(new OpenProfileAction());
	this->AddComponent(openProfileButton);

	MainLoop();
}

ProfileViewer::~ProfileViewer()
{
	free(avatar);
}

// To be removed later when there is a main engine loop for the entire game
void ProfileViewer::MainLoop()
{
	Engine *asdf = new Engine();
	asdf->ShowWindow(this);
	asdf->MainLoop();
	delete asdf;
}

void ProfileViewer::OnTick(uint32_t ticks)
{
	if (profileInfoDownload && profileInfoDownload->CheckDone())
	{
		int length, status;
		char *data = profileInfoDownload->Finish(&length, &status);

		if (data)
		{
			std::istringstream datastream(data);
			Json::Value root;

			try
			{
				datastream >> root;

				// We can't use default values here because the profile api actually puts null in the json
				if (root["User"]["Age"].isInt())
					ageLabel = new Label(Point(29, 20), Point(Label::AUTOSIZE, Label::AUTOSIZE), root["User"]["Age"].asString());
				else
				{
					ageLabel = new Label(Point(29, 20), Point(Label::AUTOSIZE, Label::AUTOSIZE), "\x0F\xC0\xC0\xC0Not Provided");
					ageLabel->SetEnabled(false);
				}
				if (root["User"]["Location"].isString())
					locationLabel = new Label(Point(53, 34), Point(Label::AUTOSIZE, Label::AUTOSIZE), root["User"]["Location"].asString());
				else
				{
					locationLabel = new Label(Point(53, 34), Point(Label::AUTOSIZE, Label::AUTOSIZE), "\x0F\xC0\xC0\xC0Not Provided");
					locationLabel->SetEnabled(false);
				}
				if (root["User"]["Website"].isString())
					websiteLabel = new Label(Point(49, 48), Point(Label::AUTOSIZE, Label::AUTOSIZE), root["User"]["Website"].asString());
				else
				{
					websiteLabel = new Label(Point(49, 48), Point(Label::AUTOSIZE, Label::AUTOSIZE), "\x0F\xC0\xC0\xC0Not Provided");
					websiteLabel->SetEnabled(false);
				}
				if (root["User"]["Biography"].isString())
					biographyLabel = new Label(Point(7, 133), Point(246, Label::AUTOSIZE), root["User"]["Biography"].asCString(), true);
				else
				{
					biographyLabel = new Label(Point(7, 133), Point(246, Label::AUTOSIZE), "\x0F\xC0\xC0\xC0Not Provided", true);
					biographyLabel->SetEnabled(false);
				}

				this->AddComponent(ageLabel);
				this->AddComponent(locationLabel);
				this->AddComponent(websiteLabel);
				this->AddComponent(biographyLabel);

				// If we don't do this average score will have a ton of decimal points, round to 2 here
				float average = root["User"]["Saves"]["AverageScore"].asFloat();
				std::stringstream averageScore;
				averageScore.precision(2);
				averageScore << std::fixed << average;

				saveCountLabel = new Label(Point(42,76), Point(Label::AUTOSIZE, Label::AUTOSIZE), root["User"]["Saves"]["Count"].asString());
				saveAverageLabel = new Label(Point(83,90), Point(Label::AUTOSIZE, Label::AUTOSIZE), averageScore.str());
				highestVoteLabel = new Label(Point(82,104), Point(Label::AUTOSIZE, Label::AUTOSIZE), root["User"]["Saves"]["HighestScore"].asString());
				this->AddComponent(saveCountLabel);
				this->AddComponent(saveAverageLabel);
				this->AddComponent(highestVoteLabel);

				enableEditingButton->SetEnabled(true);
				ResizeArea(biographyLabel->GetSize().Y);
			}
			catch (std::exception &e)
			{
				// TODO: make a new version of error_ui because this is bad
				biographyLabel = new Label(Point(7, 133), Point(246, Label::AUTOSIZE), "\brError parsing data from server", true);
				this->AddComponent(biographyLabel);
			}
		}
		else
		{
			// TODO: make a new version of error_ui because this is bad
			biographyLabel = new Label(Point(7, 133), Point(246, Label::AUTOSIZE), "\brServer returned error", true);
			this->AddComponent(biographyLabel);
		}

		free(data);
		profileInfoDownload = NULL;
	}

	if (avatarDownload && avatarDownload->CheckDone())
	{
		int length;
		char *data = avatarDownload->Finish(&length, NULL);
		if (data)
		{
			int w, h;
			avatar = ptif_unpack(data, length, &w, &h);
			if (w != 40 || h != 40)
			{
				free(avatar);
				avatar = NULL;
			}
		}

		free(data);
		avatarDownload = NULL;
	}
}

void ProfileViewer::EnableEditing()
{
	RemoveComponent(locationLabel);
	locationLabel = new Textbox(locationLabel->GetPosition(), Point(size.X-110, locationLabel->GetSize().Y), locationLabel->IsEnabled() ? locationLabel->GetText() : "");
	AddComponent(locationLabel);

	/*RemoveComponent(websiteLabel);
	websiteLabel = new Textbox(websiteLabel->GetPosition(), Point(size.X-110, websiteLabel->GetSize().Y), websiteLabel->IsEnabled() ? websiteLabel->GetText() : "");
	AddComponent(websiteLabel);*/

	RemoveComponent(biographyLabel);
	biographyLabel = new Textbox(biographyLabel->GetPosition(), biographyLabel->GetSize(), biographyLabel->IsEnabled() ? biographyLabel->GetText() : "", true);
	dynamic_cast<Textbox*>(biographyLabel)->SetAutoSize(false, true, Point(Textbox::NOSIZELIMIT,Textbox::NOSIZELIMIT));
	AddComponent(biographyLabel);

	// Enable editing when this button is clicked
	class BiographyChangedAction : public TextboxAction
	{
	public:
		virtual void TextChangedCallback(Textbox *textbox)
		{
			dynamic_cast<ProfileViewer*>(textbox->GetParent())->ResizeArea(textbox->GetSize().Y);
		}
	};
	dynamic_cast<Textbox*>(biographyLabel)->SetCallback(new BiographyChangedAction());
	dynamic_cast<Textbox*>(biographyLabel)->SetType(Textbox::MULTILINE);

	// Save profile when this button is clicked
	class ProfileSaveAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button)
		{
			dynamic_cast<ProfileViewer*>(button->GetParent())->SaveProfile();
		}
	};
	enableEditingButton->SetCallback(new ProfileSaveAction());
	enableEditingButton->SetText("Save");

	// Open profile editor online when this button is clicked
	class ProfileEditAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button)
		{
			dynamic_cast<ProfileViewer*>(button->GetParent())->OpenProfileEdit();
		}
	};
	openProfileButton->SetCallback(new ProfileEditAction());
	openProfileButton->SetText("Edit profile online");
}

void ProfileViewer::SaveProfile()
{
	profileSaveDownload = new Download("http://" SERVER "/Profile.json");
	profileSaveDownload->AuthHeaders(svf_user_id, svf_session_id);
	std::map<std::string, std::string> postData;
	postData.insert(std::pair<std::string, std::string>("Location", locationLabel->GetText()));
	postData.insert(std::pair<std::string, std::string>("Biography", biographyLabel->GetText()));
	//postData.insert(std::pair<std::string, std::string>("Website", websiteLabel->GetText()));
	profileSaveDownload->AddPostData(postData);

	profileSaveDownload->Start();
	int status;
	char * ret = profileSaveDownload->Finish(NULL, &status);
	ParseServerReturn(ret, status);
}

void ProfileViewer::OpenProfile()
{
	open_link("http://powdertoy.co.uk/User.html?Name="+name);
}

void ProfileViewer::OpenProfileEdit()
{
	open_link("http://powdertoy.co.uk/Profile.html");
}

void ProfileViewer::UploadAvatar()
{
	open_link("http://powdertoy.co.uk/Profile/Avatar.html");
}

void ProfileViewer::ResizeArea(int biographyLabelHeight)
{
	int yPos = 149+biographyLabelHeight;
	if (yPos < this->size.Y-enableEditingButton->GetSize().Y)
		yPos = this->size.Y-enableEditingButton->GetSize().Y;
	enableEditingButton->SetPosition(Point(0, yPos-GetScrollPosition()));
	openProfileButton->SetPosition(Point(size.X/2, yPos-GetScrollPosition()));

	int maxScroll = enableEditingButton->GetPosition().Y+GetScrollPosition()+enableEditingButton->GetSize().Y-this->size.Y;
	if (maxScroll >= 0)
	{
		int oldMaxScroll = this->GetMaxScrollSize();
		this->SetScrollable(true, maxScroll);
		if (this->GetScrollPosition() == oldMaxScroll)
			this->SetScrollPosition(maxScroll);
	}
	else
		this->SetScrollable(false, 0);
}

void ProfileViewer::OnDraw(VideoBuffer *buf)
{
	if (avatar)
		buf->DrawImage(avatar, 210, 10-GetScrollPosition(), 40, 40);
	buf->DrawText(10, 24-GetScrollPosition(), "Age:", 175, 175, 175, 255);
	buf->DrawText(10, 38-GetScrollPosition(), "Location:", 175, 175, 175, 255);
	buf->DrawText(10, 52-GetScrollPosition(), "Website:", 175, 175, 175, 255);
	buf->DrawText(10, 66-GetScrollPosition(), "Saves:", 175, 175, 175, 255);
	buf->DrawText(15, 80-GetScrollPosition(), "Count:", 175, 175, 175, 255);
	buf->DrawText(15, 94-GetScrollPosition(), "Average Score:", 175, 175, 175, 255);
	buf->DrawText(15, 108-GetScrollPosition(), "Highest Score:", 175, 175, 175, 255);
	buf->DrawText(10, 122-GetScrollPosition(), "Biography:", 175, 175, 175, 255);
}
