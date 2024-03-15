#include "animated_button.h"
#include "game/entity.h"
#include "game/component/animator.h"
#include "game/component/audio/audio_source.h"

AnimatedButton::AnimatedButton(
	const std::string& text,
	const TextProperty& text_prop,
	const PanelProperty& panel_prop,
	const std::function<void()>& function,
	AudioSource* audio_press,
	AudioSource* audio_hover,
	const bool fit
) : Button(text, text_prop, panel_prop, function, fit)
{
	audio_press_ = audio_press;
	audio_hover_ = audio_hover;
}

AnimatedButton::~AnimatedButton()
{
}

bool AnimatedButton::Init()
{
	Button::Init();

	animator_ = GetEntity()->GetComponent<Animator>();

	return true;
}

void AnimatedButton::OnPressed()
{
	if (audio_press_)
		audio_press_->Play();

	animator_->Play("press", 1, false);
	function_();
}

void AnimatedButton::OnHovered()
{
	if (audio_hover_)
		audio_hover_->Play();

	animator_->Play("hover", 1, false);
}

void AnimatedButton::OnUnhovered()
{
	animator_->Play("unhover", 1, false);
}
