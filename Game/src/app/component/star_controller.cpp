#define NOMINMAX

#include "star_controller.h"
#include "game/component/transform.h"
#include "game/component/collider/collider.h"
#include "game/entity.h"
#include "game/game.h"
#include "math/easing.h"
#include "game/component/particle_emitter.h"

StarController::StarController(ParticleEmitter* confetti)
{
	confetti_ = confetti;
}

StarController::~StarController()
{
}

bool StarController::Init()
{
	return true;
}

void StarController::Update(const float delta_time)
{
	if (is_slowed_)
	{
		float t = Easing::InQuad(slow_time_ / slow_duration_);
		t = std::max(1 - t, 0.01f);

		Game::Get()->SetTimeScale(t);

		slow_time_ += 1;

		if (slow_time_ > slow_duration_)
		{
			is_slowed_ = false;
			Game::Get()->SetTimeScale(1);
		}
	}

	// ‰ñ“]
	transform->rotation = Quaternion::FromEuler(angle_, angle_, 0);

	angle_ = std::fmod(angle_ + angle_speed_ * delta_time, 2.0f * (float)PI);
}

void StarController::OnCollisionEnter(Collider* collider)
{
	if (is_slowed_)
		return;

	if (collider->GetEntity()->tag == "player")
	{
		confetti_->Emit();
		is_slowed_ = true;
		slow_time_ = 0;
	}
}
