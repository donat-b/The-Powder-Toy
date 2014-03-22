#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <string>
#include <vector>
#include "graphics.h"
#include "common/Point.h"

enum { TOOLTIP, ELEMENTTIP, INFOTIP, QTIP, INTROTIP, LUATIP };

class ToolTip
{
	std::string tip;
	Point location;
	int alpha, ID;

public:
	ToolTip(std::string tip_, Point location_, int ID_, int alpha_):
		tip(tip_),
		location(location_),
		ID(ID_)
	{
		if (alpha_ == -1)
			alpha = 15;
		else
			alpha = alpha_;
	}

	void UpdateToolTip(std::string toolTip, Point location_, int alpha_)
	{
		tip = toolTip;
		location = location_;
		//alpha_ == -1 is used for fading in tooltips for some reason (gets reduced by 5 every frame in draw still)
		if (alpha_ == -1 && alpha < 255)
		{
			alpha += 15;
			if (alpha > 255)
				alpha = 255;
		}
		//else, we want to directly set alpha
		else if (ID != INTROTIP || alpha > 255)
			alpha = alpha_;
	}

	bool DrawToolTip()
	{
		if (alpha)
		{
			if (ID == INFOTIP || ID == ELEMENTTIP)
				drawtext_outline(vid_buf, location.X, location.Y, tip.c_str(), 255, 255, 255, std::min(alpha, 255), 0, 0, 0, std::min(alpha, 255));
			else
				drawtext(vid_buf, location.X, location.Y, tip.c_str(), 255, 255, 255, std::min(alpha, 255));
			alpha -= 5;
		}
		return alpha > 0;
	}

	int GetID() { return ID; }
	int GetAlpha() { return alpha; }
};

extern std::vector<ToolTip*> toolTips;
void UpdateToolTip(std::string toolTip, Point location, int ID, int alpha);
void DrawToolTips();
int GetToolTipAlpha(int ID);

#endif
