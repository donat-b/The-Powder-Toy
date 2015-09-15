#include "SDLCompat.h"
#include "CreateSign.h"
#include "interface/Label.h"
#include "interface/Button.h"
#include "interface/Textbox.h"

CreateSign::CreateSign(int signID, Point pos):
	Window_(Point(CENTERED, CENTERED), Point(250, 100)),
	signID(signID)
{
	if (signID == -1)
		newSignLabel = new Label(Point(5, 3), Point(Label::AUTOSIZE, Label::AUTOSIZE), "New sign:");
	else
		newSignLabel = new Label(Point(5, 3), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Edit sign:");
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
		Sign::Justification ju;
	public:
		JustificationAction(Sign::Justification ju):
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
	leftJuButton->SetCallback(new JustificationAction(Sign::Left));
	this->AddComponent(leftJuButton);

	middleJuButton = new Button(leftJuButton->Right(Point(5, 0)), Point(Button::AUTOSIZE, Button::AUTOSIZE), "\x9E Middle");
	middleJuButton->SetCallback(new JustificationAction(Sign::Middle));
	this->AddComponent(middleJuButton);

	rightJuButton = new Button(middleJuButton->Right(Point(5, 0)), Point(Button::AUTOSIZE, Button::AUTOSIZE), "\x9F Right");
	rightJuButton->SetCallback(new JustificationAction(Sign::Right));
	this->AddComponent(rightJuButton);

	noneJuButton = new Button(rightJuButton->Right(Point(5, 0)), Point(Button::AUTOSIZE, Button::AUTOSIZE), "  None");
	noneJuButton->SetCallback(new JustificationAction(Sign::NoJustification));
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

	if (signID == -1)
	{
		moveButton->SetEnabled(false);
		deleteButton->SetEnabled(false);
		theSign = new Sign("", pos.X, pos.Y, Sign::Middle);
		SetJustification(Sign::Middle);
	}
	else
	{
		theSign = signs[signID];
		signTextbox->SetText(theSign->GetText());
		SetJustification(theSign->GetJustification());
	}
}

void CreateSign::OnKeyPress(int key, unsigned short character, unsigned short modifiers)
{
	if (key == SDLK_RETURN)
	{
		SaveSign();
	}
}

void CreateSign::SetJustification(Sign::Justification ju)
{
	theSign->SetJustification(ju);
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
	if (signTextbox->GetText() == "")
	{
		if (signID != -1)
		{
			delete signs[signID];
			signs.erase(signs.begin()+signID);
		}
	}
	else
	{
		theSign->SetText(signTextbox->GetText());
		if (signID == -1)
			signs.push_back(theSign);
	}
	this->toDelete = true;
}
