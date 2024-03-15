#pragma once

#include "game/component/component.h"
#include "app/util/state.h"
#include "math/vec.h"
#include <memory>

class Animator;
class Rigidbody;
class ParticleEmitter;
class AudioSource;

class PlayerController2 : public Component
{
public:
	struct Property
	{
		// 移動速度
		float speed;
		// 加速度
		float acceleration;
		// 空中にいるときの加速度
		float air_acceleration;
		// 摩擦係数
		float friction;
		// ジャンプ力
		float jump_power;
		// ダッシュジャンプ力
		float dashjump_power;
		// ダッシュジャンプ速度
		float dashjump_speed;
		// ダッシュ速度
		float dash_speed;
		// 壁ジャンプのキック力
		float walljump_kick_power;
		// ジャンプの音
		AudioSource* audio_jump;
		// ダッシュジャンプの音
		AudioSource* audio_dashjump;
		// 壁ジャンプの音
		AudioSource* audio_walljump;
		// 足音
		AudioSource* audio_footstep;
		// 着地音
		AudioSource* audio_landing;
	};

	PlayerController2(const Property& prop);
	~PlayerController2();

	bool Init() override;
	void Update(const float delta_time) override;

	enum PlayerState
	{
		STATE_IDLE,
		STATE_RUN,
		STATE_DASH,
		STATE_JUMP,
		STATE_IN_AIR,
		STATE_DASHJUMP,
		STATE_SLIDING_WALL,
		STATE_WALLJUMP,
	};
	
	// 現在の状態を取得する
	PlayerState GetCurrentState() const;

private:
	// 停止
	void Idle(const float delta_time);
	// 移動
	void Run(const float delta_time);
	// ジャンプ
	void Jump(const float jump_frame, const float delta_time);
	// 空中
	void InAir(const float delta_time);
	// ダッシュジャンプ
	void Dashjump(const float jump_frame, const float delta_time);
	// 移動
	void Dash(const float delta_time);
	// 壁ずり
	void SlidingWall(const float delta_time);
	// 壁ジャンプ
	void Walljump(const float jump_frame, const float delta_time);

	// 拡縮アニメーション
	void ScaleAnimation(const Vec3& velocity, const float delta_time);
	// 足音を再生する
	void PlayFootStep();
	// カメラ回転を考慮した移動方向ベクトルを取得
	Vec3 GetMoveDirection(const Vec2& input);
	// アナログ入力を考慮した最大移動速度を取得
	float GetMaxSpeed(const Vec2& input);
	// 壁に触れているかどうかチェックする
	void WallRaycast();

private:
	// 移動速度
	float speed_;
	// 追加速度（加速中にプラスになる）
	float plus_speed_ = 0;
	// 加速度
	float acceleration_;
	// 空中にいるときの加速度
	float air_acceleration_;
	// 摩擦係数
	float friction_;
	// ジャンプ力
	float jump_power_;
	// ダッシュジャンプ力
	float dashjump_power_;
	// ダッシュジャンプの速度
	float dashjump_speed_;
	// ダッシュ速度
	float dash_speed_;
	// 壁ジャンプのキック力
	float walljump_kick_power_;

	Vec3 velocity_;

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

	// 接している壁の法線ベクトル
	Vec3 wall_normal_;
	// 蹴った壁の法線ベクトル
	Vec3 sliding_wall_normal_;

	// 空中にいるフレーム数
	float air_frame_ = 0;

	// 最大ジャンプフレーム数
	float jump_frame_max_ = 16;
	float jump_frame_min_ = 13;

	// 壁ジャンプで向きを固定するフレーム数
	float walljump_kick_frame_max_ = 30;

	// ダッシュフレーム数
	float dash_frame_ = 0;
	float dash_frame_max_ = 120;

	// 現在の速度
	float current_speed_ = 0;

	Animator* animator_ = nullptr;
	Rigidbody* rigidbody_ = nullptr;
	ParticleEmitter* run_smoke_emitter_ = nullptr;
	ParticleEmitter* jump_smoke_emitter_ = nullptr;
	ParticleEmitter* circle_smoke_emitter_ = nullptr;
	ParticleEmitter* wall_slide_smoke_emitter_ = nullptr;
	AudioSource* audio_jump_ = nullptr;
	AudioSource* audio_dashjump_ = nullptr;
	AudioSource* audio_walljump_ = nullptr;
	AudioSource* audio_footstep_ = nullptr;
	AudioSource* audio_landing_ = nullptr;


	class IdleState : public State<PlayerController2>
	{
	public:
		IdleState(PlayerController2* player, PlayerState state) : State(player, state) {}
		void OnStateBegin(State* prev_state) override;
		void OnStateEnd(State* next_state) override;
		void Update(const float delta_time) override;
	};

	class RunState : public State<PlayerController2>
	{
	public:
		RunState(PlayerController2* player, PlayerState state) : State(player, state) {}
		void OnStateBegin(State* prev_state) override;
		void OnStateEnd(State* next_state) override;
		void Update(const float delta_time) override;
	};

	class JumpState : public State<PlayerController2>
	{
	public:
		JumpState(PlayerController2* player, PlayerState state) : State(player, state) {}
		void OnStateBegin(State* prev_state) override;
		void OnStateEnd(State* next_state) override;
		void Update(const float delta_time) override;

	private:
		// ジャンプフレーム数
		float jump_frame_ = 0;
	};

	class InAirState : public State<PlayerController2>
	{
	public:
		InAirState(PlayerController2* player, PlayerState state) : State(player, state) {}
		void OnStateBegin(State* prev_state) override;
		void OnStateEnd(State* next_state) override;
		void Update(const float delta_time) override;
	};

	class DashjumpState : public State<PlayerController2>
	{
	public:
		DashjumpState(PlayerController2* player, PlayerState state) : State(player, state) {}
		void OnStateBegin(State* prev_state) override;
		void OnStateEnd(State* next_state) override;
		void Update(const float delta_time) override;

	private:
		// ジャンプフレーム数
		float jump_frame_ = 0;
	};

	class DashState : public State<PlayerController2>
	{
	public:
		DashState(PlayerController2* player, PlayerState state) : State(player, state) {}
		void OnStateBegin(State* prev_state) override;
		void OnStateEnd(State* next_state) override;
		void Update(const float delta_time) override;
	};

	class SlidingWallState : public State<PlayerController2>
	{
	public:
		SlidingWallState(PlayerController2* player, PlayerState state) : State(player, state) {}
		void OnStateBegin(State* prev_state) override;
		void OnStateEnd(State* next_state) override;
		void Update(const float delta_time) override;
	};

	class WalljumpState : public State<PlayerController2>
	{
	public:
		WalljumpState(PlayerController2* player, PlayerState state) : State(player, state) {}
		void OnStateBegin(State* prev_state) override;
		void OnStateEnd(State* next_state) override;
		void Update(const float delta_time) override;

	private:
		float jump_frame_ = 0;
		float kick_frame_ = 0;
	};

	friend State<PlayerController2>;

	State<PlayerController2>* state_ = nullptr;
	std::unique_ptr<IdleState> idle_state_ = nullptr;
	std::unique_ptr<RunState> run_state_ = nullptr;
	std::unique_ptr<JumpState> jump_state_ = nullptr;
	std::unique_ptr<InAirState> in_air_state_ = nullptr;
	std::unique_ptr<DashjumpState> dashjump_state_ = nullptr;
	std::unique_ptr<DashState> dash_state_ = nullptr;
	std::unique_ptr<SlidingWallState> sliding_wall_state_ = nullptr;
	std::unique_ptr<WalljumpState> walljump_state_ = nullptr;
};