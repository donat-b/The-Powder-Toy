#ifndef CREATESIGN_H
#define CREATESIGN_H
#include "common/Point.h"
#include "interface/Window.h"

class Label;
class Button;
class Textbox;
class CreateSign : public Window_
{
	Label *newSignLabel, *justificationLabel;
	Button *okButton, *leftJuButton, *middleJuButton, *rightJuButton, *noneJuButton, *moveButton, *deleteButton;
	Textbox *signTextbox;

	int signID;
public:
	CreateSign(int signID, Point pos);

	void OnKeyPress(int key, unsigned short character, unsigned short modifiers);

	void SetJustification(int ju);
	void MoveSign();
	void DeleteSign();
	void SaveSign();
};

#endif
