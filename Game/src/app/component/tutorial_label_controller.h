#pragma once

#include "game/component/component.h"
#include <string>

class Label;
class Animator;
class TutorialTextController;
class AudioSource;

class TutorialLabelController : public Component
{
public:
	TutorialLabelController(AudioSource* audio_show, AudioSource* audio_hide);
	~TutorialLabelController() {}

	bool Init() override;
	
	void Show(const std::string& text, TutorialTextController* tutorial);
	void Hide();

private:
	Label* label_ = nullptr;
	Animator* animator_ = nullptr;
	TutorialTextController* tutorial_ = nullptr;
	AudioSource* audio_show_ = nullptr;
	AudioSource* audio_hide_ = nullptr;
};