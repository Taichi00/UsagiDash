#include "app/player.h"
#include "game/input.h"
#include "game/entity.h"
#include "game/component/animator.h"
#include "game/component/camera.h"
#include "game/scene.h"
#include "game/component/rigidbody.h"
#include "game/physics.h"
#include "game/component/particle_emitter.h"

Player::Player(PlayerProperty prop)
{
	speed_ = prop.speed;
	acceleration_ = prop.acceleration;
	
}

Player::~Player()
{
}

bool Player::Init()
{
	animator_ = GetEntity()->GetComponent<Animator>();
	rigidbody_ = GetEntity()->GetComponent<Rigidbody>();

	run_smoke_emitter_ = GetEntity()->GetChild("Run Smoke Emitter")->GetComponent<ParticleEmitter>();
	jump_smoke_emitter_ = GetEntity()->GetChild("Jump Smoke Emitter")->GetComponent<ParticleEmitter>();
	circle_smoke_emitter_ = GetEntity()->GetChild("Circle Smoke Emitter")->GetComponent<ParticleEmitter>();

	return true;
}

void Player::Update()
{
	Move();
	Animate();

	is_running_prev_ = is_running_;
	is_grouded_prev_ = is_grounded_;
	jump_frame_prev_ = jump_frame_;

}

void Player::Move()
{
	auto camera = GetEntity()->GetScene()->GetMainCamera();
	auto cameraRot = camera->transform->rotation;
	auto forward = Vec3::Scale(cameraRot * Vec3(0, 0, -1), 1, 0, 1).Normalized();
	auto left = Vec3::Scale(cameraRot * Vec3(-1, 0, 0), 1, 0, 1).Normalized();

	is_grounded_ = rigidbody_->is_grounded;

	float friction = 0.9;
	float acceleration = acceleration_;

	if (!is_grounded_)
	{
		friction = 0.99;
		acceleration *= 0.4;
	}

	auto velocity = rigidbody_->velocity - rigidbody_->floor_velocity;
	
	Vec3 v = Vec3::Zero();

	if (Input::GetKey(DIK_UP))
	{
		v += forward;
	}
	else if (Input::GetKey(DIK_DOWN))
	{
		v += -forward;
	}

	if (Input::GetKey(DIK_LEFT))
	{
		v += left;
	}
	else if (Input::GetKey(DIK_RIGHT))
	{
		v += -left;
	}

	v = v.Normalized();

	auto pv = v * acceleration_;

	// ‘¬“x‚ÌXV
	velocity += pv;

	// –€ŽC
	if (v == Vec3::Zero())
	{
		is_running_ = false;

		velocity.x *= friction;
		velocity.z *= friction;
	}
	else
	{
		is_running_ = true;
	}

	// â
	if (is_grounded_)
	{
		auto q = Quaternion::FromToRotation(Vec3(0, 1, 0), rigidbody_->floor_normal);
		
		if ((q * pv).y < 0)
		{
			pv = q * pv;
		}
		
		// â‚ðŠŠ‚ç‚È‚¢‚æ‚¤‚É—Í‚ð‰Á‚¦‚é
		velocity += Vec3(0, 0.018, 0) - rigidbody_->floor_normal * 0.018;
	}

	// Å‘å‘¬“x‚ð§ŒÀ‚·‚é
	auto velocityXZ = Vec3::Scale(velocity, 1, 0, 1);
	auto speedXZ = Vec3::Dot(velocityXZ.Normalized(), v) * speed_;

	// speedXZ > 0 ‚Æ‚·‚é‚±‚Æ‚Åis•ûŒü‚Æ”½‘Î‚Ì•ûŒü‚Ö‚Ì‰Á‘¬‚Í§ŒÀ‚µ‚È‚¢
	if (velocityXZ.Length() > speedXZ && speedXZ > 0)
	{
		auto xz = velocityXZ.Normalized() * speedXZ;
		velocity.x = xz.x;
		velocity.z = xz.z;
	}

	// ƒWƒƒƒ“ƒv
	if (Input::GetKeyDown(DIK_SPACE))
	{
		jump_frame_ = 0;
		velocity.y = 0;
	}
	if (jump_frame_ < jump_frame_max_)
	{
		if (Input::GetKey(DIK_SPACE))
		{
			(velocity).y += 0.15 * std::pow((jump_frame_max_ - jump_frame_) / (float)jump_frame_max_, 4);
		}
		jump_frame_++;
	}

	// ‰ñ“]
	if (v.Length() > 0)
	{
		angle_ = atan2(v.x, v.z);

		auto new_quat = Quaternion::FromEuler(0, angle_, 0);
		transform->rotation = Quaternion::Slerp(transform->rotation, new_quat, 0.15);
	}
	
	auto floor_quat = Quaternion::FromToRotation(Vec3(0, 1, 0), rigidbody_->floor_normal).Conjugate();
	run_smoke_emitter_->transform->rotation = floor_quat;

	rigidbody_->velocity = velocity + rigidbody_->floor_velocity;

}

void Player::Animate()
{
	if (animator_ == nullptr)
	{
		return;
	}
	
	auto velocity = rigidbody_->velocity - rigidbody_->floor_velocity;

	auto scale = Vec3(1, 1, 1);

	if (is_grounded_)
	{
		if ((is_running_ && !is_running_prev_) || (is_running_ && !is_grouded_prev_))
		{
			animator_->Play("Run", 2.0);
		}
		else if (!is_running_ && is_running_prev_ || (!is_running_ && !is_grouded_prev_))
		{
			animator_->Play("Idle", 2.0);
		}
		if (is_running_)
		{
			auto speed = Vec3::Scale(velocity, 1, 0, 1).Length();
			animator_->SetSpeed(speed * 7 + 1.0);
			run_smoke_emitter_->SetSpawnRate(speed / speed_ * 0.22);
		}
		else
		{
			run_smoke_emitter_->SetSpawnRate(0);
		}

		if (!is_grouded_prev_ && air_count_ > 10)
		{
			circle_smoke_emitter_->Emit();
		}

		air_count_ = 0;
	}
	else
	{
		run_smoke_emitter_->SetSpawnRate(0);

		if (is_grouded_prev_ && jump_frame_ > 2)
		{
			animator_->Play("Jump_Idle", 2.0);
		}

		// Šgk
		scale.y = std::abs(velocity.y) * 0.8 + 1;
		scale.x = 1 - std::abs(velocity.y) * 0.1;
		scale.z = 1 - std::abs(velocity.y) * 0.1;

		air_count_++;
	}

	if (jump_frame_ == 1)
	{
		animator_->Play("Jump", 4.0, false);
		animator_->Push("Jump_Idle", 2.0);

		jump_smoke_emitter_->Emit();
	}
	if (jump_frame_ > 0 && jump_frame_ < jump_frame_max_)
	{
		auto& prop = jump_smoke_emitter_->GetProperety();
		float frame = prop.spawn_count / prop.spawn_rate;
		float rate = jump_frame_ / frame;
		
		prop.scale_easing.start = Vec3::Lerp(Vec3(0.8, 0.8, 0.8), Vec3(0.3, 0.3, 0.3), rate);
		prop.time_to_live = 50 * (1.0 - rate) + 20;
	}
	
	transform->scale = Vec3::Lerp(transform->scale, scale, 0.3);
}
