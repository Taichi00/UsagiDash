#include "jump_lift_controller.h"
#include "game/entity.h"
#include "game/component/collider/collider.h"
#include "game/component/rigidbody.h"
#include "game/component/audio/audio_source.h"

JumpLiftController::JumpLiftController(
	const Vec3& direction,
	const float distance,
	AudioSource* audio_move,
	AudioSource* audio_stop
)
{
	direction_ = direction.Normalized();
	distance_ = distance;
	audio_move_ = audio_move;
	audio_stop_ = audio_stop;
}

JumpLiftController::~JumpLiftController()
{
}

bool JumpLiftController::Init()
{
	rigidbody_ = GetEntity()->GetComponent<Rigidbody>();

	start_position_ = transform->position;

	return true;
}

void JumpLiftController::Update(const float delta_time)
{
	// “®‚­
	if (is_moving_)
	{
		rigidbody_->velocity += direction_ * acceleration_ * delta_time;

		auto distance = (transform->position - start_position_).Length();
		if (distance > distance_)
		{
			rigidbody_->velocity = Vec3::Zero();
			audio_stop_->Play(2);

			end_position_ = transform->position;

			is_back_ = true;
			is_moving_ = false;
		}
	}

	// Œ³‚ÌêŠ‚É–ß‚é
	if (is_back_)
	{
		rigidbody_->velocity = -direction_ * back_speed_;
		
		auto distance = (transform->position - end_position_).Length();
		if (distance > distance_)
		{
			rigidbody_->velocity = Vec3::Zero();

			transform->position = start_position_;

			is_back_ = false;
		}
	}
}

void JumpLiftController::OnCollisionEnter(Collider* collider)
{
	if (is_moving_ || is_back_)
		return;

	if (collider->GetEntity()->tag == "player")
	{
		auto y = collider->transform->position.y - transform->position.y;

		// ã‚Éæ‚Á‚Ä‚¢‚½‚ç
		if (y > 0)
		{
			audio_move_->Play(2);
			is_moving_ = true;
		}
	}
}
