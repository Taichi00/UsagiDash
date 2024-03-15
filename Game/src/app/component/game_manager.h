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
		PlayerController2* player, 
		CameraController* camera, 
		Label* coin_label);

	~GameManager() {}

	static GameManager* Get()
	{
		return instance_;
	}

	bool Init() override;
	void Update(const float delta_time) override;

	// コインを取得する
	void AddCoin(const int n);

	// スタート位置を設定する
	void SetStartPosition(const Vec3& position);

	// プレイヤーのリスポーン位置を指定する
	void SetCurrentCheckpoint(Entity* checkpoint) { current_checkpoint_ = checkpoint; }
	Entity* CurrentCheckpoint() const { return current_checkpoint_; }

	// プレイヤーをリスポーンさせる
	void RespawnPlayer();

	// ゲームの開始処理
	void StartGame();
	// ゲームの終了処理
	void EndGame();

	// ポーズ
	void Pause();
	void Resume();
	void TogglePause();

	// タイトルに戻る
	void LoadTitle();

private:
	// コインの枚数テキストを取得する
	std::string GetCoinText(const int n);

	void PlayerFallen();

	void UpdatePause();

private:
	static GameManager* instance_;

	PlayerController2* player_;
	CameraController* camera_;
	Label* coin_label_;
	Animator* coin_label_animator_ = nullptr;
	Transition* transition_ = nullptr;
	PauseManager* pause_manager_ = nullptr;

	AudioSource* audio_source_ = nullptr;

	// コインの枚数
	unsigned int num_coins_ = 0;

	// 現在のチェックポイント
	Entity* current_checkpoint_ = nullptr;

	// スタート位置
	Vec3 start_position_;

	// ポーズメニュー
	Entity* pause_menu_ = nullptr;

	SceneState scene_state_ = SCENE_TITLE;
};