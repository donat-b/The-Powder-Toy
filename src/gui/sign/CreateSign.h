#ifndef CREATESIGN_H
#define CREATESIGN_H
#include "common/Point.h"
#include "interface/Window.h"
#include "game/Sign.h"

class Label;
class Button;
class Textbox;
class CreateSign : public Window_
{
	Label *newSignLabel, *justificationLabel;
	Button *okButton, *leftJuButton, *middleJuButton, *rightJuButton, *noneJuButton, *moveButton, *deleteButton;
	Textbox *signTextbox;

	int signID;
	Sign *theSign;
public:
	CreateSign(int signID, Point pos);

	void OnKeyPress(int key, unsigned short character, unsigned short modifiers);

	void SetJustification(Sign::Justification ju);
	void MoveSign();
	void DeleteSign();
	void SaveSign();
};

#endif
