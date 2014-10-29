#include "ToolTip.h"

std::vector<ToolTip*> toolTips;
void UpdateToolTip(std::string toolTip, Point location, int ID, int alpha)
{
	for (unsigned int i = 0; i < toolTips.size(); i++)
		if (toolTips[i]->GetID() == ID)
		{
			toolTips[i]->UpdateToolTip(toolTip, location, alpha);
			return;
		}
	if (ID != INTROTIP || alpha > 255)
		toolTips.push_back(new ToolTip(toolTip, location, ID, alpha));
}

void DrawToolTips()
{
	for (int i = toolTips.size()-1; i >= 0; i--)
	{
		if (!toolTips[i]->DrawToolTip())
		{
			delete toolTips[i];
			toolTips.erase(toolTips.begin()+i);
		}
	}
}

int GetToolTipAlpha(int ID)
{
	for (unsigned int i = 0; i < toolTips.size(); i++)
		if (toolTips[i]->GetID() == ID)
			return toolTips[i]->GetAlpha();
	return 0;
}
