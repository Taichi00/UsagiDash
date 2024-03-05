#include "transition.h"
#include "engine/engine2d.h"

Transition::Transition(const Color& color, const Vec2& direction)
{
	color_ = color;
	direction_ = direction;
	time_ = 0;
	speed_ = 1;
	easing_type_ = Easing::LINEAR;
}

Transition::~Transition()
{
}

bool Transition::Init()
{
	return true;
}

void Transition::Update(const float delta_time)
{
	if (is_fading_in_)
	{
		time_ += 0.6f * speed_ * delta_time;

		if (time_ > 1)
		{
			time_ = 1;
			is_fading_in_ = false;
		}
	}
	else if (is_fading_out_)
	{
		time_ += 0.6f * speed_ * delta_time;

		if (time_ > 1)
		{
			time_ = 0;
			is_fading_out_ = false;
			is_fading_ = false;
		}
	}
}

void Transition::Draw2D()
{
	if (is_fading_)
	{
		Vec2 direction = direction_;

		// �C�[�W���O
		float time = Easing::GetFunction(easing_type_)(time_);

		if (is_fading_out_)
		{
			direction = -direction;
			time = 1 - time;
		}

		// �`��
		engine_->DrawTransition(color_ * GetColor(), time, direction, true);
	}
}

void Transition::FadeIn(const float speed, const Easing::Type easing_type)
{
	speed_ = speed;
	time_ = 0;
	easing_type_ = easing_type;

	is_fading_in_ = true;
	is_fading_ = true;
}

void Transition::FadeOut(const float speed, const Easing::Type easing_type)
{
	speed_ = speed;
	time_ = 0;
	easing_type_ = easing_type;

	is_fading_out_ = true;
	is_fading_ = true;
}
