#ifdef NEWINTERFACE
#ifndef PROFILEVIEWER_H
#define PROFILEVIEWER_H

#include <string>
#include "interface/Window.h"

class Download;
class Label;
class ProfileViewer : public Window_
{
	std::string name;
	Download *profileInfoDownload;

	Label *usernameLabel, *ageLabel, *websiteLabel, *biographyLabel;
	void MainLoop();
public:
	ProfileViewer(std::string profileName);

	void OnTick(float dt);
	void OnDraw(VideoBuffer *buf);
};

#endif
#endif
