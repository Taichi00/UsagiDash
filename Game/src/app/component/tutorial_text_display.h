#pragma once

#include "game/component/component.h"
#include <string>

class Collider;
class Label;

class TutorialTextDisplay : public Component
{
public:
	TutorialTextDisplay(
		const std::string& text, 
		const int index,
		Collider* collider);

	~TutorialTextDisplay() {}

	bool Init() override;

	void OnCollisionEnter(Collider* collider) override;

private:
	std::string text_;
	int index_;

	Collider* collider_ = nullptr;
	Label* label_ = nullptr;
};