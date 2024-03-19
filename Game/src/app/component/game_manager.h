#pragma once

#include "game/component/component.h"
#include "game/entity.h"
#include "math/vec.h"
#include <string>

#define NOMINMAX

class PlayerController2;
class Label;
class AudioSource;
class Animator;
class Transition;
class CameraController;
class PauseManager;
class ButtonBase;
class Rigidbody;
class TimelinePlayer;

class GameManager : public Component
{
public:
	enum SceneState
	{
		SCENE_TITLE,
		SCENE_GAME,
	};

	GameManager(
		const SceneState state,
		Entity* player, 
		Entity* camera, 
		Label* coin_label
	);

	~GameManager() {}

	static GameManager* Get()
	{
		return instance_;
	}

	bool Init() override;
	void Update(const float delta_time) override;

	// �R�C�����擾����
	void AddCoin(const int n);

	// �X�^�[�g�ʒu��ݒ肷��
	void SetStartPosition(const Vec3& position);

	// �v���C���[�̃��X�|�[���ʒu���w�肷��
	void SetCurrentCheckpoint(Entity* checkpoint) { current_checkpoint_ = checkpoint; }
	Entity* CurrentCheckpoint() const { return current_checkpoint_; }

	// �v���C���[�����X�|�[��������
	void RespawnPlayer();

	// �Q�[���̊J�n����
	void StartGame();
	// �Q�[���̏I������
	void EndGame();

	// �|�[�Y
	void Pause();
	void Resume();
	void TogglePause();

	// �^�C�g���ɖ߂�
	void LoadTitle();

	// �X�e�[�W�N���A
	void StageClear(Entity* star);

private:
	// �R�C���̖����e�L�X�g���擾����
	std::string GetCoinText(const int n);

	void PlayerFallen();

	void UpdatePause();

private:
	static GameManager* instance_;

	Entity* player_ = nullptr;
	Entity* camera_ = nullptr;

	PlayerController2* player_controller_ = nullptr;
	CameraController* camera_controller_ = nullptr;
	Label* coin_label_ = nullptr;
	Animator* coin_label_animator_ = nullptr;
	Transition* transition_ = nullptr;
	PauseManager* pause_manager_ = nullptr;
	AudioSource* audio_bgm_ = nullptr;
	Rigidbody* player_rigidbody_ = nullptr;

	// �R�C���̖���
	unsigned int num_coins_ = 0;

	// ���݂̃`�F�b�N�|�C���g
	Entity* current_checkpoint_ = nullptr;

	// �X�^�[�g�ʒu
	Vec3 start_position_;

	// �|�[�Y���j���[
	Entity* pause_menu_ = nullptr;
	ButtonBase* resume_button_ = nullptr;

	// �X�e�[�W�N���A�C�x���g
	Entity* clear_event_ = nullptr;
	TimelinePlayer* timeline_player_ = nullptr;

	SceneState scene_state_ = SCENE_TITLE;
};