#pragma once

#include "game/component/component.h"

class Collider;
class AudioSource;

class CoinScript : public Component
{
public:
	CoinScript();
	~CoinScript() = default;

	bool Init() override;
	void Update(const float delta_time) override;

	void OnCollisionEnter(Collider* collider) override;

private:
	Collider* collider_ = nullptr;
	AudioSource* audio_source_ = nullptr;

	float angle_;
	float angle_speed_;
	float effect_time_;
	bool destroy_flag_ = false;
};