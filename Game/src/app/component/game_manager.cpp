#include "game_manager.h"
#include "game/component/gui/label.h"
#include "game/scene.h"
#include "game/component/audio/audio_source.h"
#include "game/component/animator.h"
#include "app/component/player_controller.h"
#include "game/component/rigidbody.h"
#include "app/entity/checkpoint.h"

GameManager* GameManager::instance_ = nullptr;

GameManager::GameManager(PlayerController* player, Label* coin_label)
{
	player_ = player;
	coin_label_ = coin_label;
}

bool GameManager::Init()
{
	if (instance_ && instance_ != this)
	{
		instance_->player_ = player_;
		instance_->coin_label_ = coin_label_;
		instance_->current_checkpoint_ = nullptr;

		GetEntity()->Destroy();

		return true;
	}

	instance_ = this;
	GetScene()->DontDestroyOnLoad(GetEntity());

	audio_source_ = GetEntity()->GetComponent<AudioSource>();

	// BGM ���Đ�
	if (audio_source_)
		audio_source_->Play(0.3f, true);

	// coin label �̏�����
	coin_label_->SetText(GetCoinText(num_coins_));

	coin_label_animator_ = coin_label_->GetEntity()->GetComponent<Animator>();

	return true;
}

void GameManager::Update(const float delta_time)
{
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
	if (player_->transform->position.y < -50)
	{
		RespawnPlayer();
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
