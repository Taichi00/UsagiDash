#pragma once

#include "game/component/component.h"

class Collider;

class CoinScript : public Component
{
public:
	CoinScript();
	~CoinScript() = default;

	bool Init() override;
	void Update() override;

	void OnCollisionEnter(Collider* collider) override;

private:
	Collider* collider_ = nullptr;
	Collider* player_collider_ = nullptr;
	float angle_;
	float angle_speed_;
	unsigned int effect_time_;
	bool destroy_flag_ = false;
};