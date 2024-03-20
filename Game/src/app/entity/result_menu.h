#pragma once

#include "game/entity.h"
#include "app/component/result_manager.h"

class ResultMenu : public Entity
{
public:
	ResultMenu(const ResultManager::CrownScores& time_scores);
	~ResultMenu() {}
};