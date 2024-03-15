#pragma once

#include "game/component/gui/button.h"

class Animator;
class AudioSource;

class AnimatedButton : public Button
{
public:
	AnimatedButton(
		const std::string& text,
		const TextProperty& text_prop,
		const PanelProperty& panel_prop,
		const std::function<void()>& function,
		AudioSource* audio_press = nullptr,
		AudioSource* audio_hover = nullptr,
		const bool fit = true
	);
	~AnimatedButton();

	bool Init() override;

	void OnPressed() override;
	void OnHovered() override;
	void OnUnhovered() override;

private:
	Animator* animator_ = nullptr;
	AudioSource* audio_press_ = nullptr;
	AudioSource* audio_hover_ = nullptr;
};