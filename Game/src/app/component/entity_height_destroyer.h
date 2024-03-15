#pragma once

#include "game/component/component.h"

class EntityHeightDestroyer : public Component
{
public:
	EntityHeightDestroyer(const float height);
	~EntityHeightDestroyer() {}

	void Update(const float delta_time) override;

private:
	float height_;
};