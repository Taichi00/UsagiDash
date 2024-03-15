#pragma once

#include "game/component/component.h"

class Collider;
class ParticleEmitter;
class AudioSource;

class StarController : public Component
{
public:
	StarController(ParticleEmitter* confetti, AudioSource* audio_twinkle);
	~StarController();

	bool Init() override;
	void Update(const float delta_time) override;

	void OnCollisionEnter(Collider* collider) override;

private:
	// Šp“x
	float angle_ = 0;
	// ‰ñ“]‚·‚é‘¬“x
	float angle_speed_ = 1.2f;

	bool is_slowed_ = false;
	float slow_time_ = 0;
	float slow_duration_ = 30;

	ParticleEmitter* confetti_ = nullptr;
	AudioSource* audio_twinkle_ = nullptr;
};