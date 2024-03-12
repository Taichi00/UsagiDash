#pragma once

#include "game/component/component.h"
#include <string>

class Label;
class Animator;
class TutorialTextController;

class TutorialLabelController : public Component
{
public:
	TutorialLabelController() {}
	~TutorialLabelController() {}

	bool Init() override;
	
	void Show(const std::string& text, TutorialTextController* tutorial);
	void Hide();

private:
	Label* label_ = nullptr;
	Animator* animator_ = nullptr;
	TutorialTextController* tutorial_ = nullptr;
};