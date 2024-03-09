#pragma once

#include "game/component/component.h"

class PauseBehavior : public Component
{
public:
	PauseBehavior() {}
	~PauseBehavior();

	bool Init() override;
	void OnDestroy() override;
};