#include "player_controller.h"
#include "game/input/input.h"
#include "game/entity.h"
#include "game/component/animator.h"
#include "game/component/camera.h"
#include "game/scene.h"
#include "game/component/rigidbody.h"
#include "game/physics.h"
#include "game/component/particle_emitter.h"
#include "math/easing.h"
#include "game/component/audio/audio_source.h"
#include <iostream>

PlayerController::PlayerController(float speed, float acceleration)
{
	speed_ = speed;
	acceleration_ = acceleration;
	plus_speed_ = 0.f;
	dashjump_speed_ = 4;
}

PlayerController::~PlayerController()
{
}

bool PlayerController::Init()
{
	animator_ = GetEntity()->GetComponent<Animator>();
	rigidbody_ = GetEntity()->GetComponent<Rigidbody>();
	audio_source_ = GetEntity()->GetComponent<AudioSource>();

	run_smoke_emitter_ = GetEntity()->Child("run_smoke_emitter")->GetComponent<ParticleEmitter>();
	jump_smoke_emitter_ = GetEntity()->Child("jump_smoke_emitter")->GetComponent<ParticleEmitter>();
	circle_smoke_emitter_ = GetEntity()->Child("circle_smoke_emitter")->GetComponent<ParticleEmitter>();
	wall_slide_smoke_emitter_ = GetEntity()->Child("wall_slide_smoke_emitter")->GetComponent<ParticleEmitter>();

	/*idle_state_ = std::make_unique<IdleState>(this);
	run_state_ = std::make_unique<RunState>(this);
	state_ = idle_state_.get();*/

	return true;
}

void PlayerController::Update(const float delta_time)
{
	Move(delta_time);
	Animate(delta_time);

	//state_->Execute();

	is_running_prev_ = is_running_;
	is_grounded_prev_ = is_grounded_;
	jump_frame_prev_ = jump_frame_;
	
}

void PlayerController::Move2(const float delta_time)
{
	// フラグ更新
	is_grounded_ = rigidbody_->is_grounded;
	is_touching_wall_ = rigidbody_->is_touching_wall;

	float friction = is_grounded_ ? 0.9f : 1.f;
	auto acceleration = is_grounded_ ? acceleration_ : walljump_frame_ ? 0.f : acceleration_ * 0.8f;
	auto speed = speed_ + plus_speed_;
	auto velocity = (rigidbody_->velocity - rigidbody_->floor_velocity) / delta_time;

	// カメラ取得
	auto camera = GetScene()->GetMainCamera();
	auto camera_rot = camera->transform->rotation;

	// カメラ前方方向ベクトル
	auto forward = Vec3::Scale(camera_rot * Vec3(0, 0, -1), 1, 0, 1).Normalized();
	// カメラ左方向ベクトル
	auto left = Vec3::Scale(camera_rot * Vec3(-1, 0, 0), 1, 0, 1).Normalized();

	// 移動
	auto v = Vec3::Zero();
	/*if (Input::GetKey(DIK_UP))
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
	}*/
	v = v.Normalized();

	auto pv = v * acceleration;
	velocity += pv;

	if (is_grounded_)
	{
		is_running_ = v.Length() > 0 ? true : false;
	}
	else
	{
		
	}
}

void PlayerController::Move(const float delta_time)
{
	auto camera = GetEntity()->GetScene()->GetMainCamera();
	auto camera_rot = camera->transform->rotation;

	// カメラ前方方向ベクトル
	auto forward = Vec3::Scale(camera_rot * Vec3(0, 0, -1), 1, 0, 1).Normalized();

	// カメラ左方向ベクトル
	auto left = Vec3::Scale(camera_rot * Vec3(-1, 0, 0), 1, 0, 1).Normalized();
	
	is_grounded_ = rigidbody_->is_grounded;
	is_touching_wall_ = rigidbody_->is_touching_wall;

	float friction = 0.9f;
	float acceleration = acceleration_;
	float speed = speed_ + plus_speed_;
	auto velocity = (rigidbody_->velocity - rigidbody_->floor_velocity) / delta_time;

	if (!is_grounded_)
	{
		friction = 0.999f;

		if (walljump_frame_ > 0)
		{
			acceleration = 0;
		}
		else
		{
			acceleration *= 0.8f;
		}
	}

	Vec2 input_dir;
	input_dir.x = Input::GetAxis("horizontal");
	input_dir.y = Input::GetAxis("vertical");

	float input_length = std::min(input_dir.Length(), 1.f);

	if (Input::CurrentInputType() == Input::InputType::GAMEPAD)
	{
		if (is_grounded_)
		{
			if (Vec3::Scale(velocity, 1, 0, 1).Length() < speed * input_length * 1.1f)
			{
				speed = speed * input_length;
			}
			else
			{
				speed = Vec3::Scale(velocity, 1, 0, 1).Length();
			}
		}
	}
	
	// 移動
	auto v = Vec3::Zero();
	v += -left * input_dir.x;
	v += forward * input_dir.y;

	v = v.Normalized();
	auto pv = v * acceleration * input_dir.Length();

	// 速度の更新
	velocity += pv;

	if (is_grounded_)
	{
		if (v == Vec3::Zero())
		{
			is_running_ = false;
		}
		else
		{
			is_running_ = true;
		}

		// 摩擦
		if (v.x == 0)
		{
			velocity.x *= friction;
		}
		if (v.z == 0)
		{
			velocity.z *= friction;
		}
	}

	// 坂
	if (is_grounded_)
	{
		auto q = Quaternion::FromToRotation(Vec3(0, 1, 0), rigidbody_->floor_normal);
		
		if ((q * pv).y < 0)
		{
			pv = q * pv;
		}
		
		// 坂を滑らないように力を加える
		velocity += (Vec3(0, 1.08f, 0) - rigidbody_->floor_normal * 1.08f);
	}
	else if (is_grounded_prev_ && jump_frame_ == 0)
	{
		// 坂の終わりで跳ねるのを抑制する
		Vec3 origin = transform->position + Vec3(0, 0, 0);
		Vec3 direction = Vec3(0, -1, 0);
		RaycastHit hit;
		if (Physics::Raycast(origin, direction, 1, hit, { "map" }))
		{
			velocity.y = -hit.distance;
		}
	}

	// 最大速度を制限する
	auto velocityXZ = Vec3::Scale(velocity, 1, 0, 1);
	if (is_grounded_ && plus_speed_ == 0)
	{
		auto speedXZ = Vec3::Dot(velocityXZ.Normalized(), v) * speed;

		// Length > speedXZ とすることで壁に向かって加速し続けるのを防ぐ
		// speedXZ > 0 とすることで進行方向と反対の方向への加速は制限しない
		if (velocityXZ.Length() > speedXZ && speedXZ > 0)
		{
			auto xz = velocityXZ.Normalized() * speedXZ;
			velocity.x = xz.x;
			velocity.z = xz.z;
		}
	}
	else
	{
		auto velocityXZ = Vec3::Scale(velocity, 1, 0, 1);

		if (velocityXZ.Length() > speed)
		{
			auto xz = velocityXZ.Normalized() * speed;
			velocity.x = xz.x;
			velocity.z = xz.z;
		}
	}

	// 壁ずり
	is_sliding_wall_ = false;
	if (!is_grounded_ && velocity.y < 0 && air_count_ > 30)
	{
		if (is_touching_wall_ && v.Length() > 0)
		{
			auto normal = rigidbody_->wall_normal;
			if (Vec3::Dot(v, -Vec3::Scale(normal, 1, 0, 1).Normalized()) > 0.5)
			{
				is_sliding_wall_ = true;
				velocity *= 0.8f;
			}
		}
	}

	// ジャンプ
	is_jump_start_frame_ = false;
	if (Input::GetButtonDown("jump"))
	{
		if (is_grounded_ || is_touching_wall_)
		{
			if (Input::GetButton("crouch"))
			{
				velocity += v.Normalized() * speed_;
				dashjump_frame_ = 1;
				jump_frame_ = 2;
			}
			else
			{
				jump_frame_ = 1;
				plus_speed_ = 0;
			}
			
			is_jump_start_frame_ = true;
			velocity.y = 0;

			if (is_touching_wall_ && !is_grounded_)
			{
				velocity = Vec3::Scale(rigidbody_->wall_normal, 1, 0, 1) * 15;
				walljump_frame_ = 25;
			}

			audio_source_->Play(0.5f);
		}
	}
	if (jump_frame_ > 0 && jump_frame_ <= jump_frame_max_)
	{
		float jump_power = 7.f;

		if (Input::GetButton("jump"))
		{
			(velocity).y += jump_power * std::pow((jump_frame_max_ - jump_frame_) / (float)jump_frame_max_, 2.f);
		}
		jump_frame_ += 60.f * delta_time;
	}
	else if (jump_frame_ > jump_frame_max_ || is_grounded_)
	{
		jump_frame_ = 0;
	}

	if (walljump_frame_ > 0)
	{
		walljump_frame_ -= 60.f * delta_time;
	}

	if (dashjump_frame_ > 0)
	{
		auto t = (dashjump_frame_ - dashjump_frame_max_ + 10) / dashjump_frame_max_;
		plus_speed_ = dashjump_speed_ * (1.f - Easing::Linear(t));

		dashjump_frame_ += 60.f * delta_time;

		if (dashjump_frame_ > dashjump_frame_max_)
		{
			plus_speed_ = 0;
			dashjump_frame_ = 0;
		}
	}
	
	// 回転
	if (v.Length() > 0)
	{
		if (is_grounded_)
		{
			angle_ = atan2(v.x, v.z);

			auto new_quat = Quaternion::FromEuler(0, angle_, 0);
			transform->rotation = Quaternion::Slerp(transform->rotation, new_quat, 0.15f);
		}
		else
		{
			float t = 0.08f;

			angle_ = atan2(v.x, v.z);

			if (is_sliding_wall_)
			{
				auto n = rigidbody_->wall_normal;
				angle_ = atan2(n.x, n.z);
				t = 0.4f;
			}

			auto new_quat = Quaternion::FromEuler(0, angle_, 0);
			transform->rotation = Quaternion::Slerp(transform->rotation, new_quat, t);
		}
	}
	
	auto floor_quat = Quaternion::FromToRotation(Vec3(0, 1, 0), rigidbody_->floor_normal).Conjugate();
	run_smoke_emitter_->transform->rotation = floor_quat;
	
	rigidbody_->velocity = velocity * delta_time + rigidbody_->floor_velocity;
}

void PlayerController::Animate(const float delta_time)
{
	if (animator_ == nullptr)
	{
		return;
	}
	
	auto velocity = (rigidbody_->velocity - rigidbody_->floor_velocity) / delta_time;

	auto scale = Vec3(1, 1, 1);

	if (is_grounded_ && jump_frame_ == 0)
	{
		if (is_running_)
		{
			auto speed = Vec3::Scale(velocity, 1, 0, 1).Length();
			animator_->Playing("Run", 2.0f);
			animator_->SetSpeed(speed * 0.12f + 1.0f);

			run_smoke_emitter_->Emit();
			run_smoke_emitter_->SetSpawnRate(speed / speed_ * 0.22f);
		}
		else
		{
			animator_->Playing("Idle", 2.0f);
			run_smoke_emitter_->Stop();
		}

		if (!is_grounded_prev_ && air_count_ > 10)
		{
			circle_smoke_emitter_->Emit();
		}

		air_count_ = 0;
	}
	else
	{
		run_smoke_emitter_->SetSpawnRate(0);

		if (is_sliding_wall_)
		{
			animator_->Playing("SlidingWall", 2.0f);
		}
		else if (jump_frame_ == 0)
		{
			animator_->Playing("Jump_Idle", 2.0f);
		}

		// 拡縮
		scale.y = std::abs(velocity.y * delta_time) * 0.9f + 1;
		scale.x = 1 - std::abs(velocity.y * delta_time) * 0.2f;
		scale.z = 1 - std::abs(velocity.y * delta_time) * 0.2f;

		air_count_++;
	}

	if (is_jump_start_frame_)
	{
		animator_->Play("Jump", 4.0, false, 0);
		animator_->Push("Jump_Idle", 2.0);
		
		jump_smoke_emitter_->Emit();
	}
	if (jump_frame_ > 0)
	{
		auto& prop = jump_smoke_emitter_->GetProperety();
		float frame = prop.spawn_count / prop.spawn_rate;
		float rate = jump_frame_ / frame;
		
		prop.scale_easing.start = Vec3::Lerp(Vec3(0.8f, 0.8f, 0.8f), Vec3(0.3f, 0.3f, 0.3f), rate);
		prop.time_to_live = 50 * (1.0f - rate) + 20;
	}

	if (is_sliding_wall_)
	{
		wall_slide_smoke_emitter_->Emit();
	}
	else
	{
		wall_slide_smoke_emitter_->Stop();
	}

	transform->scale = Vec3::Lerp(transform->scale, scale, 0.3f);
}

//void Player::IdleState::Enter()
//{
//	std::cout << "IdleState enter." << std::endl;
//}
//
//void Player::IdleState::Execute()
//{
//	auto input_dir = object_->GetInputDirection();
//
//	if (input_dir.Length())
//	{
//		ChangeState(object_->run_state_.get());
//	}
//}
//
//void Player::RunState::Enter()
//{
//	std::cout << "RunState enter." << std::endl;
//}
//
//void Player::RunState::Execute()
//{
//	auto input_dir = object_->GetInputDirection();
//
//	if (!input_dir.Length())
//	{
//		ChangeState(object_->idle_state_.get());
//	}
//}
//
