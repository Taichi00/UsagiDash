#pragma once

#include "game/component/component.h"
#include "math/vec.h"

class Animator;
class Rigidbody;
class ParticleEmitter;

class Player : public Component
{
public:
	enum State
	{
		IDLE = 0,
		RUN,
		DASH,
		JUMP,
		DASHJUMP,
		SLIDING_WALL,
	};

	Player(float speed, float acceleration);
	~Player();

	bool Init() override;
	void Update(const float delta_time) override;


private:
	void Move(const float delta_time);
	void Animate(const float delta_time);

private:
	Vec3 move_direction_;
	float speed_;
	float plus_speed_;
	float acceleration_;
	float angle_, angle_prev_;

	bool is_running_, is_running_prev_;
	bool is_grounded_, is_grounded_prev_;
	bool is_touching_wall_;
	bool is_sliding_wall_;

	int air_count_ = 0;

	float jump_frame_, jump_frame_prev_;
	float jump_frame_max_ = 16;

	float walljump_frame_ = 0;

	float dashjump_speed_;
	float dashjump_frame_ = 0;
	float dashjump_frame_max_ = 120;

	bool is_jump_start_frame_ = false;

	Animator* animator_;
	Rigidbody* rigidbody_;
	ParticleEmitter* run_smoke_emitter_;
	ParticleEmitter* jump_smoke_emitter_;
	ParticleEmitter* circle_smoke_emitter_;

	State state_;
};