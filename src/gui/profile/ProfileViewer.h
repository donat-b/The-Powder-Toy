#ifndef PROFILEVIEWER_H
#define PROFILEVIEWER_H

#include <string>
#include "interface/ScrollWindow.h"
#include "graphics/Pixel.h"

class Download;
class Label;
class Button;
class Textbox;
class ProfileViewer : public ScrollWindow
{
	std::string name;
	Download *profileInfoDownload;
	Download *avatarDownload;
	Download *profileSaveDownload;
	pixel *avatar;

	Label *usernameLabel, *ageLabel, *locationLabel, *websiteLabel, *biographyLabel;
	Label *saveCountLabel, *saveAverageLabel, *highestVoteLabel;
	Button *enableEditingButton, *openProfileButton, *avatarUploadButton;

public:
	ProfileViewer(std::string profileName);
	~ProfileViewer();

	void OnTick(uint32_t ticks);
	void OnDraw(VideoBuffer *buf);

	// callback functions for all the buttons
	void EnableEditing();
	void SaveProfile();
	void OpenProfile();
	void OpenProfileEdit();
	void UploadAvatar();
	void ResizeArea(int biographyLabelHeight);
};

#endif
