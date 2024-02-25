#pragma once

#include "game/component/component.h"
#include "game/entity.h"
#include <string>

class Player;
class Label;
class AudioSource;

class GameManager : public Component
{
public:
	GameManager(Player* player, Label* coin_label);
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
	static GameManager* instance_;

	Player* player_;
	Label* coin_label_;

	AudioSource* audio_source_ = nullptr;

	unsigned int num_coins_ = 0; // ƒRƒCƒ“‚Ì–‡”
};