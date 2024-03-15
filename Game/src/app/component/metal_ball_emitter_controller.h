#pragma once

#include "game/component/component.h"
#include <random>

class MetalBallEmitterController : public Component
{
public:
	MetalBallEmitterController(const float radius, const float width, const float spawn_rate);
	~MetalBallEmitterController();

	bool Init() override;
	void Update(const float delta_time) override;

private:
	// “S‹…‚Ì”¼Œa
	float radius_;
	// ¶¬”ÍˆÍ‚Ì•
	float width_;
	// ¶¬ŠÔŠu
	float spawn_rate_;

	// ƒ^ƒCƒ}[
	float spawn_timer_ = 0;

	// —”¶¬Ší
	std::mt19937 rand_; 
};