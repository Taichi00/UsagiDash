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

	// コインを取得する
	void AddCoin(const int n);

	// スタート位置を設定する
	void SetStartPosition(const Vec3& position) { start_position_ = position; }

	// プレイヤーのリスポーン位置を指定する
	void SetCurrentCheckpoint(Entity* checkpoint) { current_checkpoint_ = checkpoint; }
	Entity* CurrentCheckpoint() const { return current_checkpoint_; }

	void RespawnPlayer();

private:
	// コインの枚数テキストを取得する
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

	// コインの枚数
	unsigned int num_coins_ = 0;

	// 現在のチェックポイント
	Entity* current_checkpoint_ = nullptr;

	// スタート位置
	Vec3 start_position_;

	// リスポーン中
	bool respawning_ = false;
};