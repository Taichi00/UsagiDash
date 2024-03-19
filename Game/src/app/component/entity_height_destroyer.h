#pragma once

#include "game/component/component.h"

class MeshRenderer;

class EntityHeightDestroyer : public Component
{
public:
	EntityHeightDestroyer(const float height, const float destroy_time = 20);
	~EntityHeightDestroyer() {}

	bool Init() override;
	void Update(const float delta_time) override;

private:
	float height_;

	float destroy_time_;
	float destroy_timer_ = 0;

	MeshRenderer* mesh_renderer_ = nullptr;
};