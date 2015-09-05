#include "SDLCompat.h"
#include "CreateSign.h"
#include "interface/Label.h"
#include "interface/Button.h"
#include "interface/Textbox.h"
#include "defines.h"

CreateSign::CreateSign(int signID, Point pos):
	Window_(Point(CENTERED, CENTERED), Point(250, 100)),
	signID(signID)
{
	if (signs[signID].text[0])
		newSignLabel = new Label(Point(5, 3), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Edit sign:");
	else
		newSignLabel = new Label(Point(5, 3), Point(Label::AUTOSIZE, Label::AUTOSIZE), "New sign:");
	newSignLabel->SetColor(COLRGB(140, 140, 255));
	this->AddComponent(newSignLabel);

	signTextbox = new Textbox(newSignLabel->Below(Point(0, 2)), Point(this->size.X-10, Textbox::AUTOSIZE), "");
	signTextbox->SetCharacterLimit(45);
	this->AddComponent(signTextbox);
#ifndef TOUCHUI
	FocusComponent(signTextbox);
#endif

	justificationLabel = new Label(signTextbox->Below(Point(0, 4)), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Justify:");
	this->AddComponent(justificationLabel);

	class JustificationAction : public ButtonAction
	{
		int ju;
	public:
		JustificationAction(int ju):
				ButtonAction(),
				ju(ju)
		{

		}

		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<CreateSign*>(button->GetParent())->SetJustification(ju);
		}
	};
	leftJuButton = new Button(justificationLabel->Right(Point(8, 0)), Point(Button::AUTOSIZE, Button::AUTOSIZE), "\x9D Left");
	leftJuButton->SetCallback(new JustificationAction(0));
	this->AddComponent(leftJuButton);

	middleJuButton = new Button(leftJuButton->Right(Point(5, 0)), Point(Button::AUTOSIZE, Button::AUTOSIZE), "\x9E Middle");
	middleJuButton->SetCallback(new JustificationAction(1));
	this->AddComponent(middleJuButton);

	rightJuButton = new Button(middleJuButton->Right(Point(5, 0)), Point(Button::AUTOSIZE, Button::AUTOSIZE), "\x9F Right");
	rightJuButton->SetCallback(new JustificationAction(2));
	this->AddComponent(rightJuButton);

	noneJuButton = new Button(rightJuButton->Right(Point(5, 0)), Point(Button::AUTOSIZE, Button::AUTOSIZE), "  None");
	noneJuButton->SetCallback(new JustificationAction(3));
	this->AddComponent(noneJuButton);

	class MoveAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<CreateSign*>(button->GetParent())->MoveSign();
		}
	};
	moveButton = new Button(leftJuButton->Below(Point(0, 4)), leftJuButton->GetSize(), "Move");
	moveButton->SetCallback(new MoveAction());
	this->AddComponent(moveButton);

	class DeleteAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<CreateSign*>(button->GetParent())->DeleteSign();
		}
	};
	deleteButton = new Button(middleJuButton->Below(Point(0, 4)), middleJuButton->GetSize(), "\x86\x85 Delete");
	deleteButton->SetCallback(new DeleteAction());
	this->AddComponent(deleteButton);

	class OkAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<CreateSign*>(button->GetParent())->SaveSign();
		}
	};
	okButton = new Button(Point(0, this->size.Y-14), Point(this->size.X+1, 15), "OK");
	okButton->SetCallback(new OkAction());
	this->AddComponent(okButton);

	if (!signs[signID].text[0])
	{
		moveButton->SetEnabled(false);
		deleteButton->SetEnabled(false);
		signs[signID].x = pos.X;
		signs[signID].y = pos.Y;
		SetJustification(1);
	}
	else
	{
		signTextbox->SetText(signs[signID].text);
		SetJustification(signs[signID].ju);
	}
}

void CreateSign::OnKeyPress(int key, unsigned short character, unsigned short modifiers)
{
	if (key == SDLK_RETURN)
	{
		SaveSign();
	}
}

void CreateSign::SetJustification(int ju)
{
	signs[signID].ju = ju;
	leftJuButton->SetState(ju == 0 ? Button::INVERTED : Button::NORMAL);
	middleJuButton->SetState(ju == 1 ? Button::INVERTED : Button::NORMAL);
	rightJuButton->SetState(ju == 2 ? Button::INVERTED : Button::NORMAL);
	noneJuButton->SetState(ju == 3 ? Button::INVERTED : Button::NORMAL);
}

void CreateSign::MoveSign()
{
	MSIGN = signID;
	SaveSign();
}

void CreateSign::DeleteSign()
{
	signTextbox->SetText("");
	SaveSign();
}

void CreateSign::SaveSign()
{
	std::string text = signTextbox->GetText();
	if (text.length())
		strncpy(signs[signID].text, text.c_str(), 255);
	else
		signs[signID].text[0] = '\0';
	this->toDelete = true;
}
