#include "animated_button.h"
#include "game/entity.h"
#include "game/component/animator.h"

AnimatedButton::AnimatedButton(
	const std::string& text,
	const TextProperty& text_prop,
	const PanelProperty& panel_prop,
	const std::function<void()>& function,
	const bool fit
) : Button(text, text_prop, panel_prop, function, fit)
{

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
	animator_->Play("press", 1, false);
	function_();
}

void AnimatedButton::OnHovered()
{
	animator_->Play("hover", 1, false);
}

void AnimatedButton::OnUnhovered()
{
	animator_->Play("unhover", 1, false);
}
