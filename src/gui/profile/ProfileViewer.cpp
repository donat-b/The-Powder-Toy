#include <sstream>
#include "ProfileViewer.h"
#include "interface/Engine.h"
#include "interface/Label.h"
#include "graphics/VideoBuffer.h"
#include "common/Point.h"
#include "game/Download.h"
#include "json/json.h"

ProfileViewer::ProfileViewer(std::string profileName):
	ScrollWindow(Point(CENTERED, CENTERED), Point(260, 350)),
	name(profileName),
	avatar(NULL),
	ageLabel(NULL),
	websiteLabel(NULL),
	locationLabel(NULL),
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

	usernameLabel = new Label(Point(8, 7), Point(Label::AUTOSIZE, Label::AUTOSIZE), name);
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

void ProfileViewer::OnTick(float dt)
{
	if (profileInfoDownload && profileInfoDownload->CheckDone())
	{
		int length, status;
		char *data = profileInfoDownload->Finish(&length, &status);

		std::istringstream datastream(data);
		Json::Value root;

		try
		{
			datastream >> root;

			// We can't use default values here because the profile api actually puts null in the json
			if (root["User"]["Age"].isInt())
				ageLabel = new Label(Point(30, 19), Point(Label::AUTOSIZE, Label::AUTOSIZE), root["User"]["Age"].asString());
			else
				ageLabel = new Label(Point(30, 19), Point(Label::AUTOSIZE, Label::AUTOSIZE), "\bgNot Provided");
			if (root["User"]["Location"].isString())
				locationLabel = new Label(Point(54, 31), Point(Label::AUTOSIZE, Label::AUTOSIZE), root["User"]["Location"].asString());
			else
				locationLabel = new Label(Point(54, 31), Point(Label::AUTOSIZE, Label::AUTOSIZE), "\bgNot Provided");
			if (root["User"]["Website"].isString())
				websiteLabel = new Label(Point(50, 43), Point(Label::AUTOSIZE, Label::AUTOSIZE), root["User"]["Website"].asString());
			else
				websiteLabel = new Label(Point(50, 43), Point(Label::AUTOSIZE, Label::AUTOSIZE), "\bgNot Provided");
			if (root["User"]["Biography"].isString())
				biographyLabel = new Label(Point(8, 115), Point(240, Label::AUTOSIZE), root["User"]["Biography"].asCString(), true);
			else
				biographyLabel = new Label(Point(8, 115), Point(240, Label::AUTOSIZE), "\bgNot Provided", true);

			this->AddComponent(ageLabel);
			this->AddComponent(locationLabel);
			this->AddComponent(websiteLabel);
			this->AddComponent(biographyLabel);
			if (biographyLabel->GetSize().Y+117 > this->GetSize().Y)
				this->SetScrollable(true, biographyLabel->GetSize().Y+117-this->GetSize().Y*2);

			// If we don't do this average score will have a ton of decimal points, round to 2 here
			float average = root["User"]["Saves"]["AverageScore"].asFloat();
			std::stringstream averageScore;
			averageScore.precision(2);
			averageScore << std::fixed << average;

			saveCountLabel = new Label(Point(43,67), Point(Label::AUTOSIZE, Label::AUTOSIZE), root["User"]["Saves"]["Count"].asString());
			saveAverageLabel = new Label(Point(84,79), Point(Label::AUTOSIZE, Label::AUTOSIZE), averageScore.str());
			highestVoteLabel = new Label(Point(83,91), Point(Label::AUTOSIZE, Label::AUTOSIZE), root["User"]["Saves"]["HighestScore"].asString());
			this->AddComponent(saveCountLabel);
			this->AddComponent(saveAverageLabel);
			this->AddComponent(highestVoteLabel);
		}
		catch (std::exception &e)
		{
			biographyLabel = new Label(Point(8, 115), Point(230, Label::AUTOSIZE), "\brError parsing data from server", true);
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

void ProfileViewer::OnDraw(VideoBuffer *buf)
{
	if (avatar)
		buf->DrawImage(avatar, 210, 10-GetScrollPosition(), 40, 40);
	buf->DrawText(10, 22-GetScrollPosition(), "Age:", 175, 175, 175, 255);
	buf->DrawText(10, 34-GetScrollPosition(), "Location:", 175, 175, 175, 255);
	buf->DrawText(10, 46-GetScrollPosition(), "Website:", 175, 175, 175, 255);
	buf->DrawText(10, 58-GetScrollPosition(), "Saves:", 175, 175, 175, 255);
	buf->DrawText(15, 70-GetScrollPosition(), "Count:", 175, 175, 175, 255);
	buf->DrawText(15, 82-GetScrollPosition(), "Average Score:", 175, 175, 175, 255);
	buf->DrawText(15, 94-GetScrollPosition(), "Highest Score:", 175, 175, 175, 255);
	buf->DrawText(10, 106-GetScrollPosition(), "Biography:", 175, 175, 175, 255);
}
