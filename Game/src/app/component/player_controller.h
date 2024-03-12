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
		// �ړ����x
		float speed;
		// �����x
		float acceleration;
		// �_�b�V���W�����v���x
		float dashjump_speed;
		// �W�����v�̉�
		AudioSource* audio_jump;
		// ����
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
	// �ړ����x
	float speed_;
	// �ǉ����x�i�������Ƀv���X�ɂȂ�j
	float plus_speed_ = 0;
	// �����x
	float acceleration_;

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

	// �󒆂ɂ���t���[����
	int air_count_ = 0;

	// �W�����v�t���[����
	float jump_frame_ = 0;
	// �ő�W�����v�t���[����
	float jump_frame_max_ = 16;

	// �ǃW�����v�t���[����
	float walljump_frame_ = 0;

	// �_�b�V���W�����v�̑��x
	float dashjump_speed_;
	// �_�b�V���W�����v�t���[����
	float dashjump_frame_ = 0;
	// �ő�_�b�V���W�����v�t���[����
	float dashjump_frame_max_ = 120;

	// �W�����v���J�n�����t���[�����ǂ���
	bool is_jump_start_frame_ = false;

	// �����p�̃J�E���^�[
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