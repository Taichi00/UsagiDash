#pragma once

#define NOMINMAX

#include "game/component/component.h"
#include "game/entity.h"
#include "app/component/result_manager.h"
#include "math/vec.h"
#include <string>
#include <memory>

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
class Bitmap;

class GameManager : public Component
{
public:
	enum SceneState
	{
		SCENE_TITLE,
		SCENE_GAME,
		SCENE_RESULT,
	};

	GameManager(
		const SceneState state,
		Entity* player, 
		Entity* camera, 
		Label* coin_label,
		const ResultManager::CrownScores& time_scores
	);

	~GameManager() {}

	static GameManager* Get()
	{
		return instance_;
	}

	bool Init() override;
	void Update(const float delta_time) override;

	// コインを取得する
	void AddCoin(const int n);
	// コインの最大数を設定する
	void SetCoinMax(const unsigned int n) { coin_max_ = n; }

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

	// ステージクリア
	void StageClear(Entity* star);

	// スコア
	double Time() const { return time_; }
	unsigned int CoinCount() const { return coin_count_; }
	unsigned int CoinMax() const { return coin_max_; }
	unsigned int DeathCount() const { return death_count_; }

private:
	// コインの枚数テキストを取得する
	std::string GetCoinText(const int n);

	void PlayerFallen();
	void UpdatePause();
	void UpdateTime(const float delta_time);

	void UpdateResult();

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
	Rigidbody* player_rigidbody_ = nullptr;

	// コインの枚数
	unsigned int coin_count_ = 0;
	unsigned int coin_max_ = 0;
	// 経過時間
	double time_ = 0;
	// 落下回数
	unsigned int death_count_ = 0;

	// タイムの王冠スコア
	ResultManager::CrownScores time_scores_ = {};

	// 現在のチェックポイント
	Entity* current_checkpoint_ = nullptr;

	// スタート位置
	Vec3 start_position_;

	// ポーズメニュー
	Entity* pause_menu_ = nullptr;
	ButtonBase* resume_button_ = nullptr;

	// ステージクリアイベント
	Entity* clear_event_ = nullptr;
	TimelinePlayer* timeline_player_ = nullptr;

	SceneState scene_state_ = SCENE_TITLE;
};