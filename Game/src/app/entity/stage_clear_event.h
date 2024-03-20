#pragma once

#include "game/entity.h"

class Scene;

class StageClearEvent : public Entity
{
public:
	StageClearEvent(Scene* scene);
	~StageClearEvent() {}
};