#include "game_manager.h"
#include "game/component/gui/label.h"
#include "game/scene.h"
#include "game/component/audio/audio_source.h"
#include "game/component/animator.h"
#include "app/component/player_controller.h"
#include "game/component/rigidbody.h"
#include "app/entity/checkpoint.h"
#include "game/component/gui/transition.h"
#include "app/component/camera_controller.h"

GameManager* GameManager::instance_ = nullptr;

GameManager::GameManager(PlayerController* player, CameraController* camera, Label* coin_label)
{
	player_ = player;
	camera_ = camera;
	coin_label_ = coin_label;
}

bool GameManager::Init()
{
	if (instance_ && instance_ != this)
	{
		instance_->player_ = player_;
		instance_->camera_ = camera_;
		instance_->coin_label_ = coin_label_;
		instance_->current_checkpoint_ = nullptr;

		GetEntity()->Destroy();

		return true;
	}

	instance_ = this;
	GetScene()->DontDestroyOnLoad(GetEntity());

	// audio source ‚ðŽæ“¾
	audio_source_ = GetEntity()->GetComponent<AudioSource>();

	// BGM ‚ðÄ¶
	if (audio_source_)
		audio_source_->Play(0.3f, true);

	if (coin_label_)
	{
		// coin label ‚Ì‰Šú‰»
		coin_label_->SetText(GetCoinText(num_coins_));

		coin_label_animator_ = coin_label_->GetEntity()->GetComponent<Animator>();
	}

	// ƒgƒ‰ƒ“ƒWƒVƒ‡ƒ“
	transition_ = GetScene()->FindEntity("transition")->GetComponent<Transition>();

	return true;
}

void GameManager::Update(const float delta_time)
{
	if (player_)
		PlayerFallen();
}

void GameManager::AddCoin(const int n)
{
	num_coins_ = std::max(std::min(num_coins_ + n, 9999u), 0u);

	coin_label_->SetText(GetCoinText(num_coins_));

	if (coin_label_animator_)
		coin_label_animator_->Play("get", 1, false);
}

std::string GameManager::GetCoinText(const int n)
{
	auto str = std::to_string(n);
	str = std::string(std::max(0, 4 - (int)str.size()), '0') + str;

	return str;
}

void GameManager::PlayerFallen()
{
	if (!respawning_)
	{
		if (player_->transform->position.y < -50)
		{
			respawning_ = true;
			transition_->FadeIn(4, Easing::IN_QUAD);
		}
	}
	else
	{
		if (!transition_->IsFadingIn())
		{
			respawning_ = false;
			transition_->FadeOut(4, Easing::OUT_QUAD);

			RespawnPlayer();
			camera_->ForceMove();
		}
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
