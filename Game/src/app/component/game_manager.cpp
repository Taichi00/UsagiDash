#include "game_manager.h"
#include "game/component/gui/label.h"
#include "game/scene.h"
#include "game/component/audio/audio_source.h"
#include "game/component/animator.h"
#include "app/component/player_controller.h"
#include "app/component/player_controller_2.h"
#include "game/component/rigidbody.h"
#include "app/entity/checkpoint.h"
#include "game/component/gui/transition.h"
#include "app/component/camera_controller.h"
#include "app/scene/level1_scene.h"
#include "game/input/input.h"
#include "app/component/pause_manager.h"
#include "app/entity/pause_menu.h"
#include "app/scene/title_scene.h"

GameManager* GameManager::instance_ = nullptr;

GameManager::GameManager(
	const SceneState state, 
	PlayerController2* player, 
	CameraController* camera, 
	Label* coin_label
)
{
	scene_state_ = state;
	player_ = player;
	camera_ = camera;
	coin_label_ = coin_label;
}

bool GameManager::Init()
{
	if (instance_ && instance_ != this)
	{
		instance_->scene_state_ = scene_state_;
		instance_->player_ = player_;
		instance_->camera_ = camera_;
		instance_->coin_label_ = coin_label_;
		instance_->current_checkpoint_ = nullptr;

		GetEntity()->Destroy();

		return true;
	}

	instance_ = this;
	GetScene()->DontDestroyOnLoad(GetEntity());

	// audio source を取得
	audio_bgm_ = GetEntity()->GetComponent<AudioSource>();

	// BGM を再生
	if (audio_bgm_)
	{
		audio_bgm_->Play(0.3f, true);
	}

	// coin label の初期化
	if (coin_label_)
	{
		coin_label_->SetText(GetCoinText(num_coins_));

		coin_label_animator_ = coin_label_->GetEntity()->GetComponent<Animator>();
	}

	// トランジション
	if (!transition_)
	{
		auto entity = new Entity("transition");
		transition_ = entity->AddComponent<Transition>(new Transition(
			Color(0.1f, 0.1f, 0.15f),
			Vec2(1, 1)
		));
		GetEntity()->AddChild(entity);
	}

	// ポーズ
	pause_manager_ = PauseManager::Get();

	if (pause_manager_)
		pause_manager_->Resume();

	return true;
}

void GameManager::Update(const float delta_time)
{
	if (scene_state_ == SCENE_GAME)
		PlayerFallen();

	UpdatePause();
}

void GameManager::AddCoin(const int n)
{
	num_coins_ = std::max(std::min(num_coins_ + n, 9999u), 0u);

	coin_label_->SetText(GetCoinText(num_coins_));

	if (coin_label_animator_)
		coin_label_animator_->Play("get", 1, false);
}

void GameManager::SetStartPosition(const Vec3& position)
{
	if (!player_ || !camera_)
		return;

	start_position_ = position;
	player_->transform->position = start_position_;
	camera_->ForceMove();
}

std::string GameManager::GetCoinText(const int n)
{
	auto str = std::to_string(n);
	str = std::string(std::max(0, 4 - (int)str.size()), '0') + str;

	return str;
}

void GameManager::PlayerFallen()
{
	if (!transition_)
		return;

	if (!transition_->IsFadingIn())
	{
		// プレイヤーが落下したら
		if (player_->transform->position.y < -50)
		{
			// フェードイン
			transition_->FadeIn(4, Easing::IN_QUAD, [this]() {
				transition_->FadeOut(4, Easing::OUT_QUAD);

				// リスポーン
				RespawnPlayer();
				camera_->ForceMove();
				});
		}
	}
}

void GameManager::UpdatePause()
{
	if (Input::GetButtonDown("menu"))
	{
		TogglePause();
	}
}

void GameManager::RespawnPlayer()
{
	if (current_checkpoint_)
	{
		player_->transform->position = current_checkpoint_->transform->position + Vec3(0, 5, 0);
	}
	else
	{
		player_->transform->position = start_position_ + Vec3(0, 5, 0);
	}

	player_->GetEntity()->GetComponent<Rigidbody>()->velocity = Vec3(0, 0.1f, 0);
}

void GameManager::StartGame()
{
	transition_->FadeIn(2, Easing::LINEAR, [this]() {
		// 最初のシーンをロードする
		Game::Get()->LoadScene(new Level1Scene());

		transition_->FadeOut(2);
		});
}

void GameManager::EndGame()
{
	Game::Get()->Quit();
}

void GameManager::Pause()
{
	if (!pause_manager_)
		return;

	if (scene_state_ != SCENE_GAME)
		return;

	pause_manager_->Pause();

	pause_menu_ = new PauseMenu();
	GetScene()->CreateEntity(pause_menu_);
}

void GameManager::Resume()
{
	if (!pause_manager_)
		return;

	if (scene_state_ != SCENE_GAME)
		return;

	pause_manager_->Resume();

	if (pause_menu_)
	{
		pause_menu_->Destroy();
		pause_menu_ = nullptr;
	}
}

void GameManager::TogglePause()
{
	if (!pause_manager_)
		return;

	pause_manager_->IsPaused() ? Resume() : Pause();
}

void GameManager::LoadTitle()
{
	pause_menu_ = nullptr;

	Game::Get()->LoadScene(new TitleScene());
}
