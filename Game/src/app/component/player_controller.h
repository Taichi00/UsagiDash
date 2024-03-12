#pragma once

#include "game/component/component.h"
#include "app/util/state.h"
#include "math/vec.h"
#include <memory>

class Animator;
class Rigidbody;
class ParticleEmitter;
class AudioSource;

class PlayerController : public Component
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

	struct Property
	{
		// 移動速度
		float speed;
		// 加速度
		float acceleration;
		// ダッシュジャンプ速度
		float dashjump_speed;
		// ジャンプの音
		AudioSource* audio_jump;
		// 足音
		AudioSource* audio_footstep;
	};

	PlayerController(const Property& prop);
	~PlayerController();

	bool Init() override;
	void Update(const float delta_time) override;

private:
	void Move2(const float delta_time);
	void Move(const float delta_time);
	void Animate(const float delta_time);

private:
	// 移動速度
	float speed_;
	// 追加速度（加速中にプラスになる）
	float plus_speed_ = 0;
	// 加速度
	float acceleration_;

	// 向き
	float angle_ = 0;
	float angle_prev_ = 0;

	// 走っているかどうか
	bool is_running_ = false;
	bool is_running_prev_ = false;

	// 接地しているかどうか
	bool is_grounded_ = false;
	bool is_grounded_prev_ = false;

	// 壁に接しているかどうか
	bool is_touching_wall_ = false;

	// 壁をスライディングしているかどうか
	bool is_sliding_wall_ = false;

	// 空中にいるフレーム数
	int air_count_ = 0;

	// ジャンプフレーム数
	float jump_frame_ = 0;
	// 最大ジャンプフレーム数
	float jump_frame_max_ = 16;

	// 壁ジャンプフレーム数
	float walljump_frame_ = 0;

	// ダッシュジャンプの速度
	float dashjump_speed_;
	// ダッシュジャンプフレーム数
	float dashjump_frame_ = 0;
	// 最大ダッシュジャンプフレーム数
	float dashjump_frame_max_ = 120;

	// ジャンプを開始したフレームかどうか
	bool is_jump_start_frame_ = false;

	// 足音用のカウンター
	float footstep_count_ = 0;

	Animator* animator_ = nullptr;
	Rigidbody* rigidbody_ = nullptr;
	ParticleEmitter* run_smoke_emitter_ = nullptr;
	ParticleEmitter* jump_smoke_emitter_ = nullptr;
	ParticleEmitter* circle_smoke_emitter_ = nullptr;
	ParticleEmitter* wall_slide_smoke_emitter_ = nullptr;
	AudioSource* audio_jump_ = nullptr;
	AudioSource* audio_footstep_ = nullptr;

	//State<Player>* state_ = nullptr;
	//std::unique_ptr<IdleState> idle_state_ = nullptr;
	//std::unique_ptr<RunState> run_state_ = nullptr;
};