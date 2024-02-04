#pragma once

#include "game/component/component.h"
#include "math/vec.h"

class Animator;
class Rigidbody;
class ParticleEmitter;

struct PlayerProperty
{
	float speed;
	float acceleration;
};

class Player : public Component
{
public:
	Player(PlayerProperty prop);
	~Player();

	bool Init();
	void Update();


private:
	void Move();
	void Animate();

private:
	Vec3 move_direction_;
	float speed_;
	float acceleration_;
	float angle_;

	bool is_running_, is_running_prev_;
	bool is_grounded_, is_grouded_prev_;
	int air_count_ = 0;
	int jump_frame_, jump_frame_prev_;
	int jump_frame_max_ = 15;

	Animator* animator_;
	Rigidbody* rigidbody_;
	ParticleEmitter* run_smoke_emitter_;
	ParticleEmitter* jump_smoke_emitter_;
	ParticleEmitter* circle_smoke_emitter_;
};