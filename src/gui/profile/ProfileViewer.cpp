#ifdef NEWINTERFACE
#include <sstream>
#include "ProfileViewer.h"
#include "interface/Engine.h"
#include "interface/Label.h"
#include "graphics/VideoBuffer.h"
#include "common/json.h"
#include "common/Point.h"
#include "game/Download.h"

ProfileViewer::ProfileViewer(std::string profileName):
	Window_(Point(CENTERED, CENTERED), Point(250, 300)),
	name(profileName),
	avatar(NULL),
	ageLabel(NULL),
	websiteLabel(NULL),
	biographyLabel(NULL)
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
		char *data = profileInfoDownload->Finish(NULL, NULL);

		json::Object parsed = ParseJSON(data);

		//temp (TODO): convert number to string
		std::stringstream converter;
		converter << ((json::Number)parsed["User"]["Age"]).Value();
		std::string age = converter.str();

		ageLabel = new Label(Point(30, 19), Point(Label::AUTOSIZE, Label::AUTOSIZE), age);
		websiteLabel = new Label(Point(50, 31), Point(Label::AUTOSIZE, Label::AUTOSIZE), ((json::String)parsed["User"]["Website"]).Value());
		biographyLabel = new Label(Point(8, 43), Point(230, Label::AUTOSIZE), ((json::String)parsed["User"]["Biography"]).Value(), true);
		this->AddComponent(ageLabel);
		this->AddComponent(websiteLabel);
		this->AddComponent(biographyLabel);

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
		buf->DrawImage(avatar, 200, 10, 40, 40);
	buf->DrawText(10, 22, "Age:", 175, 175, 175, 255);
	buf->DrawText(10, 34, "Website:", 175, 175, 175, 255);
	buf->DrawText(10, 46, "Biography:", 175, 175, 175, 255);
}

#endif
