#ifndef SIGN_H
#define SIGN_H
#include <string>
#include <vector>
#include "common/Point.h"

class Sign
{
public:
	enum Justification { Left = 0, Middle = 1, Right = 2, NoJustification = 3 };
	enum Type { Normal = 0, SaveLink = 1, ThreadLink = 2, Spark = 3, SearchLink = 4 };

private:
	std::string text, displayText, linkText;
	int x, y;
	Justification ju;
	Type type;

public:
	Sign(std::string text, int x, int y, Justification justification);

	void SetText(std::string newText);
	std::string GetText() { return text; }
	std::string GetLinkText() { return linkText; }
	std::string GetDisplayText();

	void SetJustification(Justification newJustification) { ju = newJustification; }
	Justification GetJustification() { return ju; }
	//void SetType(Type newType);
	Type GetType() { return type; }

	Point GetRealPos() { return Point(x, y); }
	void GetPos(int & x0, int & y0, int & w, int & h);
	void SetPos(Point newPos) { x = newPos.X; y = newPos.Y; }
	bool IsSignInArea(Point topLeft, Point bottomRight);
};

#define MAXSIGNS 16
extern std::vector<Sign*> signs;
extern int MSIGN;

void ClearSigns();
void DeleteSignsInArea(Point topLeft, Point bottomRight);
int InsideSign(int mx, int my, bool allsigns);

#endif
