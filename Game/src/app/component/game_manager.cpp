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
#include "game/component/gui/button_base.h"
#include "game/component/timeline_player.h"
#include "game/component/particle_emitter.h"
#include "game/physics.h"
#include "game/component/collider/collider.h"
#include "game/component/camera.h"
#include "app/entity/result_menu.h"	
#include "app/component/bgm_manager.h"
#include "game/resource/bitmap.h"
#include "game/component/gui/picture.h"

GameManager* GameManager::instance_ = nullptr;

GameManager::GameManager(
	const SceneState state, 
	Entity* player, 
	Entity* camera, 
	Label* coin_label,
	const ResultManager::CrownScores& time_scores
)
{
	scene_state_ = state;
	player_ = player;
	camera_ = camera;
	coin_label_ = coin_label;
	time_scores_ = time_scores;
}

bool GameManager::Init()
{
	if (instance_ && instance_ != this)
	{
		instance_->scene_state_ = scene_state_;
		instance_->player_ = player_;
		instance_->camera_ = camera_;
		instance_->coin_label_ = coin_label_;
		instance_->time_scores_ = time_scores_;
		instance_->current_checkpoint_ = nullptr;

		GetEntity()->Destroy();

		return true;
	}

	instance_ = this;
	GetScene()->DontDestroyOnLoad(GetEntity());
	
	if (player_)
	{
		player_controller_ = player_->GetComponent<PlayerController2>();
		player_rigidbody_ = player_->GetComponent<Rigidbody>();
	}

	if (camera_)
	{
		camera_controller_ = camera_->GetComponent<CameraController>();
	}

	// coin label の初期化
	if (coin_label_)
	{
		coin_label_->SetText(GetCoinText(coin_count_));

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

		transition_->SetZIndex(100);

		GetEntity()->AddChild(entity);
	}

	// ポーズ
	pause_manager_ = PauseManager::Get();
	if (pause_manager_)
	{
		pause_manager_->Resume();
	}

	// ポーズメニュー
	pause_menu_ = GetScene()->FindEntity("pause_menu");
	if (pause_menu_)
	{
		resume_button_ = pause_menu_->Child("resume_button")->GetComponent<ButtonBase>();
	}

	return true;
}

void GameManager::Update(const float delta_time)
{
	switch (scene_state_)
	{
	case SCENE_GAME:
		PlayerFallen();
		UpdatePause();
		UpdateTime(delta_time);
		break;

	case SCENE_RESULT:
		UpdateResult();
		break;
	}
}

void GameManager::AddCoin(const int n)
{
	coin_count_ = std::max(std::min(coin_count_ + n, 9999u), 0u);

	coin_label_->SetText(GetCoinText(coin_count_));

	if (coin_label_animator_)
		coin_label_animator_->Play("get", 1, false);
}

void GameManager::SetStartPosition(const Vec3& position)
{
	if (!player_ || !camera_)
		return;

	start_position_ = position;

	player_->transform->position = start_position_;
	camera_->GetComponent<CameraController>()->ForceMove(0);
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
			death_count_++;

			Input::SetActive(false);

			// フェードイン
			transition_->FadeIn(4, Easing::IN_QUAD, [this]() {
				transition_->FadeOut(4, Easing::OUT_QUAD);

				// リスポーン
				RespawnPlayer();

				Input::SetActive(true);
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

void GameManager::UpdateTime(const float delta_time)
{
	if (!pause_manager_)
		return;

	if (pause_manager_->IsPaused())
		return;

	time_ += delta_time;
}

void GameManager::UpdateResult()
{
	// クリア演出再生中
	if (timeline_player_->IsPlaying())
	{
		// 演出スキップ
		/*if (Input::GetButtonDown("ok"))
		{
			timeline_player_->SetTime(timeline_player_->Duration());
		}*/
		return;
	}
}

void GameManager::RespawnPlayer()
{
	float camera_angle = 0;

	if (current_checkpoint_)
	{
		auto pos = current_checkpoint_->transform->position;
		auto rot = current_checkpoint_->transform->rotation;

		auto angles = rot.EulerAngles();
		
		if (angles.x < 0)
		{
			camera_angle = angles.y + 3.14f;
		}
		else
		{
			camera_angle = -angles.y;
		}

		player_->transform->position = pos + Vec3(0, 5, 0);
		player_->transform->rotation = rot * Quaternion::FromEuler(0, 3.14f, 0);
	}
	else
	{
		player_->transform->position = start_position_ + Vec3(0, 5, 0);
		player_->transform->rotation = Quaternion::Identity();
	}

	player_rigidbody_->velocity = Vec3(0, 0.1f, 0);

	camera_controller_->ForceMove(camera_angle);
}

void GameManager::StartGame()
{
	coin_count_ = 0;
	time_ = 0;
	death_count_ = 0;

	Game::Get()->GetAudioEngine()->SetMasterVolume(0, 40);

	Input::SetActive(false);

	transition_->FadeIn(2, Easing::LINEAR, [this]() {
		// 最初のシーンをロードする
		Game::Get()->LoadScene(new Level1Scene());
		transition_->FadeOut(2);
		Game::Get()->GetAudioEngine()->SetMasterVolume(1, 5);
		Input::SetActive(true);
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

	if (pause_menu_)
	{
		// ポーズメニューを表示する
		pause_menu_->SetActive(true);
		resume_button_->Pick();
	}

	BGMManager::Get()->SetVolume(0.5f, 30);
}

void GameManager::Resume()
{
	if (!pause_manager_)
		return;

	pause_manager_->Resume();

	if (pause_menu_)
	{
		// ポーズメニューを非表示にする
		pause_menu_->SetActive(false);
	}

	BGMManager::Get()->SetVolume(1, 30);
}

void GameManager::TogglePause()
{
	if (!pause_manager_)
		return;

	pause_manager_->IsPaused() ? Resume() : Pause();
}

void GameManager::LoadTitle()
{
	Game::Get()->GetAudioEngine()->SetMasterVolume(0, 40);

	Input::SetActive(false);

	transition_->FadeIn(2, Easing::LINEAR, [this]() {
		Game::Get()->LoadScene(new TitleScene());
		Game::Get()->GetAudioEngine()->SetMasterVolume(1, 5);
		transition_->FadeOut(4, Easing::LINEAR, [this]() {
			Input::SetActive(true);
			});
		});
}

void GameManager::StageClear(Entity* star)
{
	clear_event_ = GetScene()->FindEntity("stage_clear_event");
	if (!clear_event_)
		return;

	// 地面の座標を取得する
	auto floor_position = star->transform->position;
	RaycastHit hit = {};
	if (Physics::Raycast(floor_position, Vec3(0, -1, 0), 10, hit, { "map" }))
	{
		floor_position = hit.point;
	}

	// プレイヤーの transform を設定
	player_->transform->position = floor_position;
	player_->transform->rotation = Quaternion::Identity();
	player_->transform->scale = Vec3(1, 1, 1);

	// カメラの transform を設定
	camera_->transform->position = floor_position;
	camera_->transform->rotation = Quaternion::Identity();

	// star のコライダーを無効にする
	star->GetComponent<Collider>()->enabled = false;

	player_controller_->enabled = false;
	player_rigidbody_->enabled = false;

	player_->Child("run_smoke_emitter")->GetComponent<ParticleEmitter>()->Stop();
	player_->Child("circle_smoke_emitter")->GetComponent<ParticleEmitter>()->Stop();
	player_->Child("jump_smoke_emitter")->GetComponent<ParticleEmitter>()->Stop();

	camera_controller_->enabled = false;

	// GUIを削除する
	GetScene()->FindEntity("gui")->Destroy();

	// BGMを止める
	BGMManager::Get()->SetVolume(0, 10);

	// timeline を再生
	timeline_player_ = clear_event_->GetComponent<TimelinePlayer>();
	timeline_player_->Play(1, false, [this]()
		{
			// 演出終了時にリザルトメニューを生成する
			GetScene()->CreateEntity(new ResultMenu(time_scores_));
		});

	scene_state_ = SCENE_RESULT;
}
