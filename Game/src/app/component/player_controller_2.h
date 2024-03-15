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
		// �ړ����x
		float speed;
		// �����x
		float acceleration;
		// �󒆂ɂ���Ƃ��̉����x
		float air_acceleration;
		// ���C�W��
		float friction;
		// �W�����v��
		float jump_power;
		// �_�b�V���W�����v��
		float dashjump_power;
		// �_�b�V���W�����v���x
		float dashjump_speed;
		// �_�b�V�����x
		float dash_speed;
		// �ǃW�����v�̃L�b�N��
		float walljump_kick_power;
		// �W�����v�̉�
		AudioSource* audio_jump;
		// �_�b�V���W�����v�̉�
		AudioSource* audio_dashjump;
		// �ǃW�����v�̉�
		AudioSource* audio_walljump;
		// ����
		AudioSource* audio_footstep;
		// ���n��
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
	
	// ���݂̏�Ԃ��擾����
	PlayerState GetCurrentState() const;

private:
	// ��~
	void Idle(const float delta_time);
	// �ړ�
	void Run(const float delta_time);
	// �W�����v
	void Jump(const float jump_frame, const float delta_time);
	// ��
	void InAir(const float delta_time);
	// �_�b�V���W�����v
	void Dashjump(const float jump_frame, const float delta_time);
	// �ړ�
	void Dash(const float delta_time);
	// �ǂ���
	void SlidingWall(const float delta_time);
	// �ǃW�����v
	void Walljump(const float jump_frame, const float delta_time);

	// �g�k�A�j���[�V����
	void ScaleAnimation(const Vec3& velocity, const float delta_time);
	// �������Đ�����
	void PlayFootStep();
	// �J������]���l�������ړ������x�N�g�����擾
	Vec3 GetMoveDirection(const Vec2& input);
	// �A�i���O���͂��l�������ő�ړ����x���擾
	float GetMaxSpeed(const Vec2& input);
	// �ǂɐG��Ă��邩�ǂ����`�F�b�N����
	void WallRaycast();

private:
	// �ړ����x
	float speed_;
	// �ǉ����x�i�������Ƀv���X�ɂȂ�j
	float plus_speed_ = 0;
	// �����x
	float acceleration_;
	// �󒆂ɂ���Ƃ��̉����x
	float air_acceleration_;
	// ���C�W��
	float friction_;
	// �W�����v��
	float jump_power_;
	// �_�b�V���W�����v��
	float dashjump_power_;
	// �_�b�V���W�����v�̑��x
	float dashjump_speed_;
	// �_�b�V�����x
	float dash_speed_;
	// �ǃW�����v�̃L�b�N��
	float walljump_kick_power_;

	Vec3 velocity_;

	// ����
	float angle_ = 0;
	float angle_prev_ = 0;

	// �����Ă��邩�ǂ���
	bool is_running_ = false;
	bool is_running_prev_ = false;

	// �ڒn���Ă��邩�ǂ���
	bool is_grounded_ = false;
	bool is_grounded_prev_ = false;

	// �ǂɐڂ��Ă��邩�ǂ���
	bool is_touching_wall_ = false;

	// �ǂ��X���C�f�B���O���Ă��邩�ǂ���
	bool is_sliding_wall_ = false;

	// �ڂ��Ă���ǂ̖@���x�N�g��
	Vec3 wall_normal_;
	// �R�����ǂ̖@���x�N�g��
	Vec3 sliding_wall_normal_;

	// �󒆂ɂ���t���[����
	float air_frame_ = 0;

	// �ő�W�����v�t���[����
	float jump_frame_max_ = 16;
	float jump_frame_min_ = 13;

	// �ǃW�����v�Ō������Œ肷��t���[����
	float walljump_kick_frame_max_ = 30;

	// �_�b�V���t���[����
	float dash_frame_ = 0;
	float dash_frame_max_ = 120;

	// ���݂̑��x
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
		// �W�����v�t���[����
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
		// �W�����v�t���[����
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