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
	Window_(Point(CENTERED, CENTERED), Point(200, 300)),
	name(profileName),
	ageLabel(NULL),
	websiteLabel(NULL),
	biographyLabel(NULL)
{
	profileInfoDownload = new Download("http://" SERVER "/User.json?Name=" + profileName);
	//profileInfoDownload->AuthHeaders();
	profileInfoDownload->Start();

	usernameLabel = new Label(Point(8, 7), Point(Label::AUTOSIZE, Label::AUTOSIZE), profileName);
	this->AddComponent(usernameLabel);
	MainLoop();
}

ProfileViewer::~ProfileViewer()
{
	delete usernameLabel;
	delete ageLabel;
	delete websiteLabel;
	delete biographyLabel;
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

		json::Object parsed = ParseJSON(data);

		//temp (TODO): convert number to string
		std::stringstream converter;
		converter << ((json::Number)parsed["User"]["Age"]).Value();
		std::string age = converter.str();

		ageLabel = new Label(Point(30, 19), Point(Label::AUTOSIZE, Label::AUTOSIZE), age, true);
		websiteLabel = new Label(Point(50, 31), Point(Label::AUTOSIZE, Label::AUTOSIZE), ((json::String)parsed["User"]["Website"]).Value(), true);
		biographyLabel = new Label(Point(8, 43), Point(180, Label::AUTOSIZE), ((json::String)parsed["User"]["Biography"]).Value(), true);
		this->AddComponent(ageLabel);
		this->AddComponent(websiteLabel);
		this->AddComponent(biographyLabel);

		delete data;
		profileInfoDownload = NULL;
	}
}

void ProfileViewer::OnDraw(VideoBuffer *buf)
{
	buf->DrawText(10, 22, "Age:", 175, 175, 175, 255);
	buf->DrawText(10, 34, "Website:", 175, 175, 175, 255);
	buf->DrawText(10, 46, "Biography:", 175, 175, 175, 255);
}

#endif
