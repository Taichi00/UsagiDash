#pragma once

#include "game/component/component.h"
#include "app/state.h"
#include "math/vec.h"
#include <memory>

class Animator;
class Rigidbody;
class ParticleEmitter;
class AudioSource;

class Player : public Component
{
public:
	/*class IdleState : public State<Player>
	{
		IdleState(Player* player) : State(player) {}
		void Enter() override;
		void Execute() override;
	};

	class RunState : public State<Player>
	{
		RunState(Player* player) : State(player) {}
		void Enter() override;
		void Execute() override;
	};

	friend State<Player>;
	friend IdleState;
	friend RunState;*/

	/*enum PlayerState
	{
		STATE_GROUNDED = 0,
		STATE_JUMPING,

		STATE_IDLE,
		STATE_RUN,
		STATE_DASH,
		STATE_JUMP,
		STATE_DASHJUMP,
		STATE_SLIDING_WALL,
	};*/

	Player(float speed, float acceleration);
	~Player();

	bool Init() override;
	void Update(const float delta_time) override;


private:
	void Move2(const float delta_time);
	void Move(const float delta_time);
	void Animate(const float delta_time);

	Vec2 GetInputDirection();

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

	Animator* animator_ = nullptr;
	Rigidbody* rigidbody_ = nullptr;
	ParticleEmitter* run_smoke_emitter_ = nullptr;
	ParticleEmitter* jump_smoke_emitter_ = nullptr;
	ParticleEmitter* circle_smoke_emitter_ = nullptr;
	AudioSource* audio_source_ = nullptr;

	//State<Player>* state_ = nullptr;
	//std::unique_ptr<IdleState> idle_state_ = nullptr;
	//std::unique_ptr<RunState> run_state_ = nullptr;
};