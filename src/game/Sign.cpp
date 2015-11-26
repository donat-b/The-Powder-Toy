#include <sstream>
#include <iomanip>
#include "Sign.h"
#include "powder.h"
#include "graphics/VideoBuffer.h"

std::vector<Sign*> signs;
int MSIGN = -1;

void ClearSigns()
{
	for (std::vector<Sign*>::iterator iter = signs.begin(), end = signs.end(); iter != end; ++iter)
		delete *iter;
	signs.clear();

	MSIGN = -1;
}

void DeleteSignsInArea(Point topLeft, Point bottomRight)
{
	for (int i = signs.size()-1; i >= 0; i--)
	{
		Point realPos = signs[i]->GetRealPos();
		if (realPos.X >= topLeft.X && realPos.Y >= topLeft.Y && realPos.X < bottomRight.X && realPos.Y < bottomRight.Y)
		{
			delete signs[i];
			signs.erase(signs.begin()+i);
		}
	}
}

// returns -1 if mouse is not inside a link sign, else returns the sign id
// allsigns argument makes it return whether inside any sign (not just link signs)
int InsideSign(int mx, int my, bool allsigns)
{
	int x, y, w, h;
	for (int i = 0; i < signs.size(); i++)
	{
		signs[i]->GetPos(x, y, w, h);
		if (mx >= x && mx <= x+w && my >= y && my <= y+h)
		{
			if (allsigns || signs[i]->GetType() != Sign::Normal)
				return i;
		}
	}
	return -1;
}

Sign::Sign(std::string text, int x, int y, Justification justification):
	x(x),
	y(y),
	ju(justification),
	type(Normal)
{
	SetText(text);
}

void Sign::SetText(std::string newText)
{
	type = Normal;
	text = displayText = newText;
	linkText = "";

	// split link signs into parts here
	int len = text.length(), splitStart;
	if (len > 2 && text[0] == '{')
	{
		switch(text[1])
		{
		case 'c':
		case 't':
			if (text[2] == ':' && text[3] >= '0' && text[3] <= '9')
			{
				splitStart = 4;
				while (splitStart < len && text[splitStart] >= '0' && text[splitStart] <= '9')
					splitStart++;
				linkText = text.substr(3, splitStart-3);
				type = text[1] == 'c' ? SaveLink : ThreadLink;
			}
			else
				return;
			break;
		case 'b':
			splitStart = 2;
			type = Spark;
			break;
		case 's':
			splitStart = 4;
			while (splitStart < len && text[splitStart] != '|')
				splitStart++;
			linkText = text.substr(3, splitStart-3);
			type = SearchLink;
			break;
		default:
			return;
		}

		if (text[splitStart] == '|' && text[len-1] == '}')
		{
			displayText = text.substr(splitStart+1, len-splitStart-2);
		}
		else
		{
			linkText = text;
			type = Normal;
			return;
		}
	}
}

std::string Sign::GetDisplayText()
{
	if (type == Normal && text.length() && text[0] == '{')
	{
		std::stringstream displayTextStream;
		bool set = true;
		if (text == "{p}")
		{
			float pressure = 0.0f;
			if (x >= 0 && x < XRES && y >= 0 && y < YRES)
				pressure = pv[y/CELL][x/CELL];
			displayTextStream << std::fixed << std::setprecision(2) << "Pressure: " << pressure;
		}
		else if (text == "{aheat}")
		{
			float aheat = 0.0f;
			if (x >= 0 && x < XRES && y >= 0 && y < YRES)
				aheat = hv[y/CELL][x/CELL];
			displayTextStream << std::fixed << std::setprecision(2) << aheat;
		}
		else if (text == "{t}")
		{
			if (x >= 0 && x < XRES && y >= 0 && y < YRES && pmap[y][x])
				displayTextStream << std::fixed << std::setprecision(2) << "Temp: " << parts[pmap[y][x]>>8].temp-273.15f;
			else
				displayTextStream << "Temp: 0.00";
		}
		else
			set = false;

		if (set)
			return displayTextStream.str();
	}
	return displayText;
}

void Sign::GetPos(int & x0, int & y0, int & w, int & h)
{
	w = VideoBuffer::TextSize(GetDisplayText()).X + 5;
	h = 15;
	x0 = (ju == Right) ? x - w :
		  (ju == Left) ? x : x - w/2;
	y0 = (y > 18) ? y - 18 : y + 4;
}

bool Sign::IsSignInArea(Point topLeft, Point bottomRight)
{
	return (x >= topLeft.X && y >= topLeft.Y && x < bottomRight.X && y < bottomRight.Y);
}
