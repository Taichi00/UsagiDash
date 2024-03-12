#include "player_controller_2.h"
#include "game/input/input.h"
#include "game/entity.h"
#include "game/component/animator.h"
#include "game/component/rigidbody.h"
#include "game/component/particle_emitter.h"
#include "game/component/camera.h"
#include "game/component/audio/audio_source.h"
#include "game/physics.h"

PlayerController2::PlayerController2(const Property& prop)
{
	speed_ = prop.speed;
	friction_ = prop.friction;
	acceleration_ = prop.acceleration;
	air_acceleration_ = prop.air_acceleration;
	jump_power_ = prop.jump_power;
	dashjump_power_ = prop.dashjump_power;
	dashjump_speed_ = prop.dashjump_speed;
	dash_speed_ = prop.dash_speed;
	walljump_kick_power_ = prop.walljump_kick_power;

	audio_jump_ = prop.audio_jump;
	audio_footstep_ = prop.audio_footstep;
}

PlayerController2::~PlayerController2()
{
}

bool PlayerController2::Init()
{
	animator_ = GetEntity()->GetComponent<Animator>();
	rigidbody_ = GetEntity()->GetComponent<Rigidbody>();

	run_smoke_emitter_ = GetEntity()->Child("run_smoke_emitter")->GetComponent<ParticleEmitter>();
	jump_smoke_emitter_ = GetEntity()->Child("jump_smoke_emitter")->GetComponent<ParticleEmitter>();
	circle_smoke_emitter_ = GetEntity()->Child("circle_smoke_emitter")->GetComponent<ParticleEmitter>();
	wall_slide_smoke_emitter_ = GetEntity()->Child("wall_slide_smoke_emitter")->GetComponent<ParticleEmitter>();

	// state の生成
	idle_state_ = std::make_unique<IdleState>(this);
	run_state_ = std::make_unique<RunState>(this);
	jump_state_ = std::make_unique<JumpState>(this);
	in_air_state_ = std::make_unique<InAirState>(this);
	dashjump_state_ = std::make_unique<DashjumpState>(this);
	dash_state_ = std::make_unique<DashState>(this);
	sliding_wall_state_ = std::make_unique<SlidingWallState>(this);
	walljump_state_ = std::make_unique<WalljumpState>(this);

	// 初期 state の設定
	state_ = idle_state_.get();
	state_->OnStateBegin(nullptr);

	return true;
}

void PlayerController2::Update(const float delta_time)
{
	is_grounded_ = rigidbody_->is_grounded;
	WallRaycast();

	// 更新
	velocity_ = rigidbody_->velocity - rigidbody_->floor_velocity;
	state_->Update(delta_time);
	rigidbody_->velocity = velocity_ + rigidbody_->floor_velocity;

	// 空中にいるフレーム数をカウント
	air_frame_ = is_grounded_ ? 0 : air_frame_ + 60.0f * delta_time;

	// ダッシュ時間を制御
	if (dash_frame_ > 0)
	{
		dash_frame_ -= 60.0f * delta_time;
	}
	
	is_grounded_prev_ = is_grounded_;
	is_running_prev_ = is_running_;
}

void PlayerController2::Idle(const float delta_time)
{
	float friction = friction_;

	// 摩擦
	velocity_.x *= friction;
	velocity_.z *= friction;

	// 坂を滑らないように力を加える
	Vec3 gravity = Vec3(0, -64.8f, 0) * delta_time;
	velocity_ += (-gravity + rigidbody_->floor_normal * gravity.y);

	// 拡縮アニメーション
	ScaleAnimation(velocity_, delta_time);
}

void PlayerController2::Run(const float delta_time)
{
	auto input = Vec2(
		Input::GetAxis("horizontal"),
		Input::GetAxis("vertical")
	);
	float acceleration = acceleration_;
	float speed = GetMaxSpeed(input);
	auto move_direction = GetMoveDirection(input);

	// 移動
	{
		// 速度の更新
		velocity_ += move_direction * acceleration * delta_time;

		// 坂を滑らないように力を加える
		Vec3 gravity = Vec3(0, -64.8f, 0) * delta_time;
		velocity_ += (-gravity + rigidbody_->floor_normal * gravity.y);

		// xz 最大速度を制限する
		Vec3 velocity_xz = Vec3::Scale(velocity_, 1, 0, 1);
		float dot = Vec3::Dot(velocity_xz.Normalized(), move_direction);
		float speed_xz = dot > 0.5f ? speed * dot : speed;

		if (velocity_xz.Length() > speed_xz)
		{
			auto xz = velocity_xz.Normalized() * speed_xz;
			velocity_.x = xz.x;
			velocity_.z = xz.z;
		}
	}

	// 回転
	{
		// 角度を計算する
		angle_ = atan2(move_direction.x, move_direction.z);

		auto new_quat = Quaternion::FromEuler(0, angle_, 0);
		transform->rotation = Quaternion::Slerp(transform->rotation, new_quat, 0.15f);
	}

	// 拡縮アニメーション
	ScaleAnimation(velocity_, delta_time);
}

void PlayerController2::Jump(const float jump_frame, const float delta_time)
{
	auto input = Vec2(
		Input::GetAxis("horizontal"),
		Input::GetAxis("vertical")
	);

	float acceleration = air_acceleration_;
	float speed = GetMaxSpeed(input);

	auto move_direction = GetMoveDirection(input);

	// 移動
	{
		// 速度の更新
		velocity_ += move_direction * acceleration;

		// xz 最大速度を制限する
		Vec3 velocity_xz = Vec3::Scale(velocity_, 1, 0, 1);

		if (velocity_xz.Length() > speed)
		{
			auto xz = velocity_xz.Normalized() * speed;
			velocity_.x = xz.x;
			velocity_.z = xz.z;
		}
	}

	// 回転
	if (move_direction != Vec3::Zero())
	{
		// 角度を計算する
		angle_ = atan2(move_direction.x, move_direction.z);

		auto new_quat = Quaternion::FromEuler(0, angle_, 0);
		transform->rotation = Quaternion::Slerp(transform->rotation, new_quat, 0.08f);
	}

	// ジャンプ
	velocity_.y += jump_power_ * std::pow(jump_frame / jump_frame_max_, 2.0f);

	// 拡縮アニメーション
	ScaleAnimation(velocity_, delta_time);
}

void PlayerController2::InAir(const float delta_time)
{
	auto input = Vec2(
		Input::GetAxis("horizontal"),
		Input::GetAxis("vertical")
	);
	
	float acceleration = air_acceleration_;
	float speed = GetMaxSpeed(input);

	auto move_direction = GetMoveDirection(input);

	// 移動
	{
		// 速度の更新
		velocity_ += move_direction * acceleration;

		// xz 最大速度を制限する
		Vec3 velocity_xz = Vec3::Scale(velocity_, 1, 0, 1);

		if (velocity_xz.Length() > speed)
		{
			auto xz = velocity_xz.Normalized() * speed;
			velocity_.x = xz.x;
			velocity_.z = xz.z;
		}
	}

	// 回転
	if (move_direction != Vec3::Zero())
	{
		// 角度を計算する
		angle_ = atan2(move_direction.x, move_direction.z);

		auto new_quat = Quaternion::FromEuler(0, angle_, 0);
		transform->rotation = Quaternion::Slerp(transform->rotation, new_quat, 0.08f);
	}

	// 拡縮アニメーション
	ScaleAnimation(velocity_, delta_time);
}

void PlayerController2::Dashjump(const float jump_frame, const float delta_time)
{
	auto input = Vec2(
		Input::GetAxis("horizontal"),
		Input::GetAxis("vertical")
	);

	float acceleration = air_acceleration_;
	float speed = dashjump_speed_;

	auto move_direction = GetMoveDirection(input);

	// 移動
	{
		// 速度の更新
		velocity_ += move_direction * acceleration;

		// xz 最大速度を制限する
		Vec3 velocity_xz = Vec3::Scale(velocity_, 1, 0, 1);

		if (velocity_xz.Length() > speed)
		{
			auto xz = velocity_xz.Normalized() * speed;
			velocity_.x = xz.x;
			velocity_.z = xz.z;
		}
	}

	// 回転
	if (move_direction != Vec3::Zero())
	{
		// 角度を計算する
		angle_ = atan2(move_direction.x, move_direction.z);

		auto new_quat = Quaternion::FromEuler(0, angle_, 0);
		transform->rotation = Quaternion::Slerp(transform->rotation, new_quat, 0.08f);
	}

	// ジャンプ
	if (jump_frame > 0)
	{
		velocity_.y += dashjump_power_ * std::pow(jump_frame / jump_frame_max_, 2.0f);
	}

	// 拡縮アニメーション
	ScaleAnimation(velocity_, delta_time);
}

void PlayerController2::Dash(const float delta_time)
{
	auto input = Vec2(
		Input::GetAxis("horizontal"),
		Input::GetAxis("vertical")
	);

	float acceleration = acceleration_;
	float speed = dash_speed_;

	auto move_direction = GetMoveDirection(input);

	// 移動
	{
		// 速度の更新
		velocity_ += move_direction * acceleration * delta_time;

		// 坂を滑らないように力を加える
		Vec3 gravity = Vec3(0, -64.8f, 0) * delta_time;
		velocity_ += (-gravity + rigidbody_->floor_normal * gravity.y);

		// xz 最大速度を制限する
		Vec3 velocity_xz = Vec3::Scale(velocity_, 1, 0, 1);
		float speed_xz = Vec3::Dot(velocity_xz.Normalized(), move_direction) * speed;

		if (velocity_xz.Length() > speed)
		{
			auto xz = velocity_xz.Normalized() * speed;
			velocity_.x = xz.x;
			velocity_.z = xz.z;
		}
	}

	// 回転
	{
		// 角度を計算する
		angle_ = atan2(move_direction.x, move_direction.z);

		auto new_quat = Quaternion::FromEuler(0, angle_, 0);
		transform->rotation = Quaternion::Slerp(transform->rotation, new_quat, 0.15f);
	}

	// 拡縮アニメーション
	ScaleAnimation(velocity_, delta_time);
}

void PlayerController2::SlidingWall(const float delta_time)
{
	auto input = Vec2(
		Input::GetAxis("horizontal"),
		Input::GetAxis("vertical")
	);

	float acceleration = air_acceleration_;
	float speed = GetMaxSpeed(input);

	auto move_direction = GetMoveDirection(input);

	// 移動
	{
		// 速度の更新
		velocity_ += move_direction * acceleration;

		// xz 最大速度を制限する
		Vec3 velocity_xz = Vec3::Scale(velocity_, 1, 0, 1);

		if (velocity_xz.Length() > speed)
		{
			auto xz = velocity_xz.Normalized() * speed;
			velocity_.x = xz.x;
			velocity_.z = xz.z;
		}
	}

	// 回転
	{
		auto wall_normal = sliding_wall_normal_;

		// 角度を計算する
		angle_ = atan2(wall_normal.x, wall_normal.z);

		auto new_quat = Quaternion::FromEuler(0, angle_, 0);
		transform->rotation = Quaternion::Slerp(transform->rotation, new_quat, 0.4f);
	}

	// 壁ずり
	velocity_ *= 0.8f;

	// 拡縮アニメーション
	ScaleAnimation(velocity_, delta_time);
}

void PlayerController2::Walljump(const float jump_frame, const float delta_time)
{
	auto input = Vec2(
		Input::GetAxis("horizontal"),
		Input::GetAxis("vertical")
	);

	float acceleration = air_acceleration_;
	float speed = GetMaxSpeed(input);
	auto move_direction = GetMoveDirection(input);
	
	// 回転
	{
		// 角度を計算する
		angle_ = atan2(sliding_wall_normal_.x, sliding_wall_normal_.z);

		auto new_quat = Quaternion::FromEuler(0, angle_, 0);
		transform->rotation = Quaternion::Slerp(transform->rotation, new_quat, 0.4f);
	}

	// ジャンプ
	if (Input::GetButton("jump") && jump_frame > 0)
	{
		velocity_.y += jump_power_ * std::pow(jump_frame / jump_frame_max_, 2.0f);
	}

	// 拡縮アニメーション
	ScaleAnimation(velocity_, delta_time);
}

void PlayerController2::ScaleAnimation(const Vec3& velocity, const float delta_time)
{
	Vec3 scale;
	scale.y = std::abs(velocity.y * delta_time) * 0.9f + 1;
	scale.x = 1 - std::abs(velocity.y * delta_time) * 0.2f;
	scale.z = 1 - std::abs(velocity.y * delta_time) * 0.2f;

	transform->scale = Vec3::Lerp(transform->scale, scale, 0.3f);
}

Vec3 PlayerController2::GetMoveDirection(const Vec2& input)
{
	auto camera = GetEntity()->GetScene()->GetMainCamera();
	auto camera_rotation = camera->transform->rotation;

	// カメラ前方向ベクトル
	auto forward = Vec3::Scale(camera_rotation * Vec3(0, 0, -1), 1, 0, 1).Normalized();
	// カメラ左方向ベクトル
	auto left = Vec3::Scale(camera_rotation * Vec3(-1, 0, 0), 1, 0, 1).Normalized();

	return (-left * input.x + forward * input.y).Normalized();
}

float PlayerController2::GetMaxSpeed(const Vec2& input)
{
	float speed = speed_;

	// ゲームパッドならスティックの倒し具合に応じて最大速度を変更する
	if (Input::CurrentInputType() == Input::InputType::GAMEPAD)
	{
		float input_len = input.Length();
		float velocity_len = Vec3::Scale(velocity_, 1, 0, 1).Length();

		if (velocity_len < speed_ * input_len * 1.1f)
		{
			speed = speed_ * input_len;
		}
		else
		{
			speed = velocity_len;
		}
	}

	return speed;
}

void PlayerController2::WallRaycast()
{
	Vec2 input = Vec2(
		Input::GetAxis("horizontal"),
		Input::GetAxis("vertical")
	);
	Vec3 move_direction = GetMoveDirection(input);

	Vec3 origin = transform->position + Vec3(0, 1, 0);
	Vec3 direction = move_direction.Normalized();
	float distance = 2;

	RaycastHit hit;
	if (Physics::Raycast(origin, direction, distance, hit, { "map" }))
	{
		wall_normal_ = hit.normal;
		is_touching_wall_ = true;
	}
	else
	{
		is_touching_wall_ = false;
	}
}

// --------------------------------------------------------------
// IdleState
// --------------------------------------------------------------
void PlayerController2::IdleState::OnStateBegin(State* prev_state)
{
	// アニメーション
	object->animator_->Play("Idle", 2);

	// パーティクル
	{
		if (!object->is_grounded_prev_&& object->air_frame_ > 10)
		{
			object->circle_smoke_emitter_->Emit();
		}
	}

	object->dash_frame_ = 0;

	printf("idle\n");
}

void PlayerController2::IdleState::OnStateEnd(State* next_state)
{
}

void PlayerController2::IdleState::Update(const float delta_time)
{
	if (object->is_grounded_)
	{
		if (Input::GetButtonDown("jump"))
		{
			if (Input::GetButton("crouch"))
			{
				ChangeState(object->dashjump_state_.get(), delta_time);
				return;
			}
			else
			{
				ChangeState(object->jump_state_.get(), delta_time);
				return;
			}
		}

		if (Input::GetAxis("horizontal") || Input::GetAxis("vertical"))
		{
			ChangeState(object->run_state_.get(), delta_time);
			return;
		}
	}
	else
	{
		ChangeState(object->in_air_state_.get(), delta_time);
		return;
	}

	object->Idle(delta_time);
}

// --------------------------------------------------------------
// RunState
// --------------------------------------------------------------
void PlayerController2::RunState::OnStateBegin(State* prev_state)
{
	// アニメーション
	object->animator_->Play("Run", 2);

	// パーティクル
	{
		if (!object->is_grounded_prev_ && object->air_frame_ > 10)
		{
			object->circle_smoke_emitter_->Emit();
		}

		object->run_smoke_emitter_->Emit();
	}

	object->is_running_ = true;

	printf("run\n");
}

void PlayerController2::RunState::OnStateEnd(State* next_state)
{
	// パーティクル
	object->run_smoke_emitter_->Stop();

	object->is_running_ = false;
}

void PlayerController2::RunState::Update(const float delta_time)
{
	if (object->is_grounded_)
	{
		if (Input::GetButtonDown("jump"))
		{
			if (Input::GetButton("crouch"))
			{
				ChangeState(object->dashjump_state_.get(), delta_time);
				return;
			}
			else
			{
				ChangeState(object->jump_state_.get(), delta_time);
				return;
			}
		}

		if (!Input::GetAxis("horizontal") && !Input::GetAxis("vertical"))
		{
			ChangeState(object->idle_state_.get(), delta_time);
			return;
		}
	}
	else
	{
		ChangeState(object->in_air_state_.get(), delta_time);
		return;
	}

	object->Run(delta_time);

	auto velocity = object->velocity_;
	auto velocity_xz = Vec3::Scale(velocity, 1, 0, 1);
	float speed = velocity_xz.Length();

	// 地面の角度
	Quaternion floor_quat = Quaternion::FromToRotation(Vec3(0, 1, 0), object->rigidbody_->floor_normal);

	// アニメーション
	float anim_speed = speed * 0.12f + 1;
	object->animator_->Playing("Run", anim_speed);

	// パーティクル
	float spawn_speed = speed / object->speed_ * 0.22f;
	Quaternion rot = Quaternion::FromToRotation(velocity, velocity_xz);

	object->run_smoke_emitter_->SetSpawnRate(spawn_speed);
	object->run_smoke_emitter_->transform->rotation = rot;
}

// --------------------------------------------------------------
// JumpState
// --------------------------------------------------------------
void PlayerController2::JumpState::OnStateBegin(State* prev_state)
{
	// アニメーション
	object->animator_->Play("Jump", 4, false, 0);
	object->animator_->Push("Jump_Idle", 2);

	// パーティクル
	object->jump_smoke_emitter_->Emit();

	// サウンド
	object->audio_jump_->Play(0.5f);

	object->dash_frame_ = 0;
	jump_frame_ = object->jump_frame_max_;

	printf("jump\n");
}

void PlayerController2::JumpState::OnStateEnd(State* next_state)
{
	jump_frame_ = 0;
}

void PlayerController2::JumpState::Update(const float delta_time)
{
	auto input = Vec2(
		Input::GetAxis("horizontal"),
		Input::GetAxis("vertical")
	);
	auto move_direction = object->GetMoveDirection(input);
	auto velocity = object->velocity_;

	if (object->rigidbody_->is_grounded)
	{
		if (jump_frame_ < object->jump_frame_max_)
		{
			if (!Input::GetAxis("horizontal") && !Input::GetAxis("vertical"))
			{
				ChangeState(object->idle_state_.get(), delta_time);
				return;
			}
			else
			{
				ChangeState(object->run_state_.get(), delta_time);
				return;
			}
		}
	}
	else
	{
		if (object->is_touching_wall_)
		{
			if (Input::GetButtonDown("jump"))
			{
				ChangeState(object->walljump_state_.get(), delta_time);
				return;
			}

			if (velocity.y <= 0 && object->air_frame_ > 30)
			{
				ChangeState(object->sliding_wall_state_.get(), delta_time);
				return;
			}
		}

		if ((!Input::GetButton("jump") && jump_frame_ < object->jump_frame_min_) || jump_frame_ <= 0)
		{
			ChangeState(object->in_air_state_.get(), delta_time);
			return;
		}
	}

	object->Jump(jump_frame_, delta_time);

	jump_frame_ -= 60.0f * delta_time;
}

// --------------------------------------------------------------
// InAirState
// --------------------------------------------------------------
void PlayerController2::InAirState::OnStateBegin(State* prev_state)
{
	// アニメーション
	object->animator_->Playing("Jump_Idle", 2);

	// 坂の終わりで跳ねるのを抑制する
	if (object->is_grounded_prev_)
	{
		Vec3 origin = object->transform->position + Vec3(0, 0, 0);
		Vec3 direction = Vec3(0, -1, 0);
		RaycastHit hit;
		if (Physics::Raycast(origin, direction, 1, hit, { "map" }))
		{
			object->velocity_.y = -hit.distance;
		}
	}

	printf("in air\n");
}

void PlayerController2::InAirState::OnStateEnd(State* next_state)
{
}

void PlayerController2::InAirState::Update(const float delta_time)
{
	auto input = Vec2(
		Input::GetAxis("horizontal"),
		Input::GetAxis("vertical")
	);
	auto move_direction = object->GetMoveDirection(input);
	auto velocity = object->velocity_;

	if (object->is_grounded_)
	{
		if (input == Vec2::Zero())
		{
			ChangeState(object->idle_state_.get(), delta_time);
			return;
		}
		else
		{
			if (object->dash_frame_ > 0)
			{
				ChangeState(object->dash_state_.get(), delta_time);
				return;
			}
			else
			{
				ChangeState(object->run_state_.get(), delta_time);
				return;
			}
		}
	}
	else
	{
		if (object->is_touching_wall_)
		{
			if (Input::GetButtonDown("jump"))
			{
				ChangeState(object->walljump_state_.get(), delta_time);
				return;
			}

			if (velocity.y <= 0 && object->air_frame_ > 30)
			{
				ChangeState(object->sliding_wall_state_.get(), delta_time);
				return;
			}
		}
	}
	
	object->InAir(delta_time);
}

// --------------------------------------------------------------
// DashjumpState
// --------------------------------------------------------------
void PlayerController2::DashjumpState::OnStateBegin(State* prev_state)
{
	// アニメーション
	object->animator_->Play("Jump", 4, false, 0);
	object->animator_->Push("Jump_Idle", 2);

	// パーティクル
	object->jump_smoke_emitter_->Emit();

	// サウンド
	object->audio_jump_->Play(0.5f);

	auto input = Vec2(
		Input::GetAxis("horizontal"),
		Input::GetAxis("vertical")
	);
	auto move_direction = object->GetMoveDirection(input);
	auto floor_quat = Quaternion::FromToRotation(Vec3(0, 1, 0), object->rigidbody_->floor_normal);

	object->velocity_.y = 0;
	
	// 上り坂なら上に飛ばす
	auto v = move_direction.Normalized() * object->dashjump_speed_;
	if ((floor_quat * v).y > 0)
	{
		object->velocity_ += floor_quat * v;
	}
	else
	{
		object->velocity_ += v;
	}

	jump_frame_ = object->jump_frame_max_;

	printf("dashjump\n");
}

void PlayerController2::DashjumpState::OnStateEnd(State* next_state)
{
	jump_frame_ = 0;
}

void PlayerController2::DashjumpState::Update(const float delta_time)
{
	auto input = Vec2(
		Input::GetAxis("horizontal"),
		Input::GetAxis("vertical")
	);
	auto move_direction = object->GetMoveDirection(input);
	auto velocity = object->velocity_;

	if (object->is_grounded_)
	{
		if (jump_frame_ < object->jump_frame_min_)
		{
			if (!Input::GetAxis("horizontal") && !Input::GetAxis("vertical"))
			{
				ChangeState(object->idle_state_.get(), delta_time);
				return;
			}
			else
			{
				ChangeState(object->dash_state_.get(), delta_time);
				return;
			}
		}
	}
	else
	{
		if (object->is_touching_wall_)
		{
			if (Input::GetButtonDown("jump"))
			{
				ChangeState(object->walljump_state_.get(), delta_time);
				return;
			}

			if (velocity.y <= 0 && object->air_frame_ > 30)
			{
				ChangeState(object->sliding_wall_state_.get(), delta_time);
				return;
			}
		}

		if ((!Input::GetButton("jump") && jump_frame_ < object->jump_frame_min_) || jump_frame_ <= 0)
		{
			ChangeState(object->in_air_state_.get(), delta_time);
			return;
		}
	}
	
	object->Dashjump(jump_frame_, delta_time);

	object->dash_frame_ = object->dash_frame_max_;
	jump_frame_ -= 60.0f * delta_time;
}

// --------------------------------------------------------------
// DashState
// --------------------------------------------------------------
void PlayerController2::DashState::OnStateBegin(State* prev_state)
{
	// アニメーション
	object->animator_->Play("Run", 2);

	// パーティクル
	{
		if (!object->is_grounded_prev_ && object->air_frame_ > 10)
		{
			object->circle_smoke_emitter_->Emit();
		}

		object->run_smoke_emitter_->Emit();
	}

	object->is_running_ = true;

	printf("dash\n");
}

void PlayerController2::DashState::OnStateEnd(State* next_state)
{
	// パーティクル
	object->run_smoke_emitter_->Stop();

	object->is_running_ = false;
}

void PlayerController2::DashState::Update(const float delta_time)
{
	auto input = Vec2(
		Input::GetAxis("horizontal"),
		Input::GetAxis("vertical")
	);
	auto move_direction = object->GetMoveDirection(input);

	if (object->is_grounded_)
	{
		if (Input::GetButtonDown("jump"))
		{
			if (Input::GetButton("crouch"))
			{
				ChangeState(object->dashjump_state_.get(), delta_time);
				return;
			}
			else
			{
				ChangeState(object->jump_state_.get(), delta_time);
				return;
			}
		}

		if (object->dash_frame_ <= 0)
		{
			ChangeState(object->run_state_.get(), delta_time);
			return;
		}

		if (input == Vec2::Zero())
		{
			ChangeState(object->idle_state_.get(), delta_time);
			return;
		}
	}
	else
	{
		ChangeState(object->in_air_state_.get(), delta_time);
		return;
	}

	object->Dash(delta_time);

	auto velocity = object->velocity_;
	auto velocity_xz = Vec3::Scale(velocity, 1, 0, 1);
	float speed = velocity_xz.Length();

	// 地面の角度
	Quaternion floor_quat = Quaternion::FromToRotation(Vec3(0, 1, 0), object->rigidbody_->floor_normal);

	// アニメーション
	float anim_speed = speed * 0.12f + 1;
	object->animator_->Playing("Run", anim_speed);

	// パーティクル
	float spawn_speed = speed / object->speed_ * 0.22f;
	Quaternion rot = Quaternion::FromToRotation(velocity, velocity_xz);

	object->run_smoke_emitter_->SetSpawnRate(spawn_speed);
	object->run_smoke_emitter_->transform->rotation = rot;
}

// --------------------------------------------------------------
// SlidingWallState
// --------------------------------------------------------------
void PlayerController2::SlidingWallState::OnStateBegin(State* prev_state)
{
	// アニメーション
	object->animator_->Play("SlidingWall", 2);

	// パーティクル
	object->wall_slide_smoke_emitter_->Emit();

	printf("sliding wall\n");
}

void PlayerController2::SlidingWallState::OnStateEnd(State* next_state)
{
	// パーティクル
	object->wall_slide_smoke_emitter_->Stop();
}

void PlayerController2::SlidingWallState::Update(const float delta_time)
{
	auto input = Vec2(
		Input::GetAxis("horizontal"),
		Input::GetAxis("vertical")
	);
	auto move_direction = object->GetMoveDirection(input);
	auto velocity = object->velocity_;

	object->sliding_wall_normal_ = object->wall_normal_;

	if (object->rigidbody_->is_grounded)
	{
		if (input != Vec2::Zero())
		{
			ChangeState(object->idle_state_.get(), delta_time);
			return;
		}
		else
		{
			ChangeState(object->run_state_.get(), delta_time);
			return;
		}
	}
	else
	{
		if (Input::GetButtonDown("jump"))
		{
			ChangeState(object->walljump_state_.get(), delta_time);
			return;
		}

		if (!object->is_touching_wall_)
		{
			ChangeState(object->in_air_state_.get(), delta_time);
			return;
		}
	}

	object->SlidingWall(delta_time);
}

// --------------------------------------------------------------
// WalljumpState
// --------------------------------------------------------------
void PlayerController2::WalljumpState::OnStateBegin(State* prev_state)
{
	// アニメーション
	object->animator_->Play("Jump", 4, false, 0);
	object->animator_->Push("Jump_Idle", 2);

	// パーティクル
	object->jump_smoke_emitter_->Emit();

	// サウンド
	object->audio_jump_->Play(0.5f);

	Vec3 wall_normal = object->wall_normal_; 
	auto wall_dir = Vec3::Scale(wall_normal, 1, 0, 1).Normalized();
	auto jump_dir = (wall_dir + Vec3(0, 1, 0)).Normalized();

	object->sliding_wall_normal_ = wall_normal;

	// 初速
	object->velocity_ = wall_dir * object->walljump_kick_power_;

	jump_frame_ = object->jump_frame_max_;
	kick_frame_ = object->walljump_kick_frame_max_;

	printf("walljump\n");
}

void PlayerController2::WalljumpState::OnStateEnd(State* next_state)
{
}

void PlayerController2::WalljumpState::Update(const float delta_time)
{
	auto input = Vec2(
		Input::GetAxis("horizontal"),
		Input::GetAxis("vertical")
	);
	auto move_direction = object->GetMoveDirection(input);
	auto velocity = object->velocity_;

	if (object->is_grounded_)
	{
		if (jump_frame_ < object->jump_frame_max_)
		{
			if (!Input::GetAxis("horizontal") && !Input::GetAxis("vertical"))
			{
				ChangeState(object->idle_state_.get(), delta_time);
				return;
			}
			else
			{
				ChangeState(object->dash_state_.get(), delta_time);
				return;
			}
		}
	}
	else
	{
		if (object->is_touching_wall_)
		{
			if (jump_frame_ < object->jump_frame_max_)
			{
				if (Input::GetButtonDown("jump"))
				{
					ChangeState(object->walljump_state_.get(), delta_time);
					return;
				}

				if (velocity.y <= 0 && object->air_frame_ > 30)
				{
					ChangeState(object->sliding_wall_state_.get(), delta_time);
					return;
				}
			}
		}

		if (kick_frame_ <= 0)
		{
			ChangeState(object->in_air_state_.get(), delta_time);
			return;
		}
	}

	object->Walljump(jump_frame_, delta_time);

	if (!Input::GetButton("jump") && jump_frame_ < object->jump_frame_min_)
	{
		jump_frame_ = 0;
	}
	else
	{
		jump_frame_ -= 60.0f * delta_time;
	}

	kick_frame_ -= 60.0f * delta_time;
}
