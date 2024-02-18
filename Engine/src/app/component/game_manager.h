#pragma once

#include "game/component/component.h"

class Player;
class Label;

class GameManager : public Component
{
public:
	GameManager(Player* player, Label* coin_label);
	~GameManager() = default;

	bool Init() override;
	void Update(const float delta_time) override;

private:
	Player* player_;
	Label* coin_label_;
};