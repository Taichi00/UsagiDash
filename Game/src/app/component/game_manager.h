#pragma once

#include "game/component/component.h"
#include "game/entity.h"
#include <string>

#define NOMINMAX

class PlayerController;
class Label;
class AudioSource;
class Animator;

class GameManager : public Component
{
public:
	GameManager(PlayerController* player, Label* coin_label);
	~GameManager() = default;

	static GameManager* Get()
	{
		return instance_;
	}

	/*static GameManager* Create(Player* player, Label* coin_label)
	{
		if (!instance_)
		{
			instance_ = new GameManager(player, coin_label);
			return instance_;
		}

		instance_->player_ = player;
		instance_->coin_label_ = coin_label;

		return instance_;
	}*/

	bool Init() override;
	void Update(const float delta_time) override;

	void AddCoin(const int n);

private:
	std::string GetCoinText(const int n);

private:
	static GameManager* instance_;

	PlayerController* player_;
	Label* coin_label_;
	Animator* coin_label_animator_ = nullptr;

	AudioSource* audio_source_ = nullptr;

	unsigned int num_coins_ = 0; // ÉRÉCÉìÇÃñáêî
};