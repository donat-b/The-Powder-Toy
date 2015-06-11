#ifndef PROFILEVIEWER_H
#define PROFILEVIEWER_H

#include <string>
#include "interface/ScrollWindow.h"
#include "graphics.h"

class Download;
class Label;
class Button;
class ProfileViewer : public ScrollWindow
{
	std::string name;
	Download *profileInfoDownload;
	Download *avatarDownload;
	pixel *avatar;

	Label *usernameLabel, *ageLabel, *locationLabel, *websiteLabel, *biographyLabel;
	Label *saveCountLabel, *saveAverageLabel, *highestVoteLabel;
	Button *enableEditingButton;
	bool editingMode;

	void MainLoop();
	void LabelToTextbox(Label *label);

public:
	ProfileViewer(std::string profileName);
	~ProfileViewer();

	void OnTick(uint32_t ticks);
	void OnDraw(VideoBuffer *buf);

	void EnableEditing();
};

#endif
