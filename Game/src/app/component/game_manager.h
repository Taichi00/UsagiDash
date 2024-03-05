#pragma once

#include "game/component/component.h"
#include "game/entity.h"
#include "math/vec.h"
#include <string>

#define NOMINMAX

class PlayerController;
class Label;
class AudioSource;
class Animator;
class Transition;
class CameraController;

class GameManager : public Component
{
public:
	GameManager(PlayerController* player, CameraController* camera, Label* coin_label);
	~GameManager() = default;

	static GameManager* Get()
	{
		return instance_;
	}

	bool Init() override;
	void Update(const float delta_time) override;

	// �R�C�����擾����
	void AddCoin(const int n);

	// �X�^�[�g�ʒu��ݒ肷��
	void SetStartPosition(const Vec3& position) { start_position_ = position; }

	// �v���C���[�̃��X�|�[���ʒu���w�肷��
	void SetCurrentCheckpoint(Entity* checkpoint) { current_checkpoint_ = checkpoint; }
	Entity* CurrentCheckpoint() const { return current_checkpoint_; }

	void RespawnPlayer();

private:
	// �R�C���̖����e�L�X�g���擾����
	std::string GetCoinText(const int n);

	void PlayerFallen();

private:
	static GameManager* instance_;

	PlayerController* player_;
	CameraController* camera_;
	Label* coin_label_;
	Animator* coin_label_animator_ = nullptr;
	Transition* transition_ = nullptr;

	AudioSource* audio_source_ = nullptr;

	// �R�C���̖���
	unsigned int num_coins_ = 0;

	// ���݂̃`�F�b�N�|�C���g
	Entity* current_checkpoint_ = nullptr;

	// �X�^�[�g�ʒu
	Vec3 start_position_;

	// ���X�|�[����
	bool respawning_ = false;
};