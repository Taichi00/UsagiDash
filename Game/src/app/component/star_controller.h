#pragma once

#include "game/component/component.h"

class Collider;
class ParticleEmitter;
class AudioSource;
class TimelinePlayer;

class StarController : public Component
{
public:
	StarController(
		ParticleEmitter* confetti, 
		AudioSource* audio_twinkle,
		AudioSource* audio_pick
	);
	~StarController();

	bool Init() override;
	void Update(const float delta_time) override;

	void OnCollisionEnter(Collider* collider) override;

private:
	// Šp“x
	float angle_ = 0;
	// ‰ñ“]‚·‚é‘¬“x
	float angle_speed_ = 1.2f;

	bool is_catched_ = false;

	ParticleEmitter* confetti_ = nullptr;
	AudioSource* audio_twinkle_ = nullptr;
	AudioSource* audio_pick_ = nullptr;
	TimelinePlayer* timeline_player_ = nullptr;
};