#pragma once

#include "game/scene.h"

class Level1Scene : public Scene
{
public:
	bool Init() override;
	void Update(const float delta_time) override;
};
