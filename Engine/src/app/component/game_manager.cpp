#include "game_manager.h"

GameManager::GameManager(Player* player, Label* coin_label)
{
	player_ = player;
	coin_label_ = coin_label;
}

bool GameManager::Init()
{
	return true;
}

void GameManager::Update(const float delta_time)
{
}
