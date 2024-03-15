#pragma once

#include "game/component/component.h"

class AudioSource;
class Collider;

class MetalBallController : public Component
{
public:
	MetalBallController(AudioSource* audio_rolling);
	~MetalBallController();

	void OnCollisionEnter(Collider* collider) override;

private:
	// “]‚ª‚é‚Æ‚«‚Ì‰¹
	AudioSource* audio_rolling_;
};