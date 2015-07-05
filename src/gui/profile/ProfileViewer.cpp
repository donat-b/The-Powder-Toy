#include <sstream>
#include "ProfileViewer.h"
#include "interface/Engine.h"
#include "interface/Label.h"
#include "interface/Textbox.h"
#include "interface/Button.h"
#include "graphics/VideoBuffer.h"
#include "common/Point.h"
#include "game/Download.h"
#include "json/json.h"

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
	highestVoteLabel(NULL),
	enableEditingButton(NULL)
{
	profileInfoDownload = new Download("http://" SERVER "/User.json?Name=" + name);
	//profileInfoDownload->AuthHeaders();
	profileInfoDownload->Start();

	avatarDownload = new Download("http://" STATICSERVER "/avatars/" + name + ".pti");
	avatarDownload->Start();

	usernameLabel = new Label(Point(7, 6), Point(Label::AUTOSIZE, Label::AUTOSIZE), name);
	this->AddComponent(usernameLabel);
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
					ageLabel = new Label(Point(29, 20), Point(Label::AUTOSIZE, Label::AUTOSIZE), "\x0F\xC0\xC0\xC0Not Provided");
				if (root["User"]["Location"].isString())
					locationLabel = new Label(Point(53, 34), Point(Label::AUTOSIZE, Label::AUTOSIZE), root["User"]["Location"].asString());
				else
					locationLabel = new Label(Point(53, 34), Point(Label::AUTOSIZE, Label::AUTOSIZE), "\x0F\xC0\xC0\xC0Not Provided");
				if (root["User"]["Website"].isString())
					websiteLabel = new Label(Point(49, 48), Point(Label::AUTOSIZE, Label::AUTOSIZE), root["User"]["Website"].asString());
				else
					websiteLabel = new Label(Point(49, 48), Point(Label::AUTOSIZE, Label::AUTOSIZE), "\x0F\xC0\xC0\xC0Not Provided");
				if (root["User"]["Biography"].isString())
					biographyLabel = new Label(Point(7, 133), Point(246, Label::AUTOSIZE), root["User"]["Biography"].asCString(), true);
				else
					biographyLabel = new Label(Point(7, 133), Point(246, Label::AUTOSIZE), "\x0F\xC0\xC0\xC0Not Provided", true);

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

				// Enable editing when this button is clicked
				class EnableEditingAction : public ButtonAction
				{
				public:
					virtual void ButtionActionCallback(Button *button)
					{
						dynamic_cast<ProfileViewer*>(button->GetParent())->EnableEditing();
					}
				};
				enableEditingButton = new Button(Point(0, 149+biographyLabel->GetSize().Y), Point(this->size.X, 15), "Enable Editing");
				enableEditingButton->SetCallback(new EnableEditingAction());
				this->AddComponent(enableEditingButton);

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

Textbox* ProfileViewer::LabelToTextbox(Label *label)
{
	Textbox *textbox = new Textbox(label->GetPosition(), label->GetSize(), label->GetText(), label->IsMultiline());
	RemoveComponent(label);
	AddComponent(textbox);
	if (textbox->IsMultiline())
		textbox->SetAutoSize(false, true, Point(Textbox::NOSIZELIMIT,Textbox::NOSIZELIMIT));
	return textbox;
}

void ProfileViewer::EnableEditing()
{
	ageLabel = LabelToTextbox(ageLabel);
	locationLabel = LabelToTextbox(locationLabel);
	websiteLabel = LabelToTextbox(websiteLabel);
	biographyLabel = LabelToTextbox(biographyLabel);

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

	// Enable editing when this button is clicked
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
}

void ProfileViewer::SaveProfile()
{
	enableEditingButton->SetText("test");
}

void ProfileViewer::ResizeArea(int biographyLabelHeight)
{
	int yPos = 149+biographyLabelHeight;
	if (yPos < this->size.Y-enableEditingButton->GetSize().Y)
		yPos = this->size.Y-enableEditingButton->GetSize().Y;
	enableEditingButton->SetPosition(Point(0, yPos-GetScrollPosition()));

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
