#pragma once

#include "game/component/component.h"

class ParticleEmitter;
class GameManager;
class Animator;
class AudioSource;

class CheckpointController : public Component
{
public:
	CheckpointController();
	~CheckpointController() {}

	bool Init() override;

	void OnCollisionEnter(Collider* collider) override;

private:
	ParticleEmitter* confetti_emitter_ = nullptr;
	ParticleEmitter* smoke_emitter_ = nullptr;
	Animator* animator_ = nullptr;
	AudioSource* audio_ = nullptr;

	GameManager* game_manager_ = nullptr;
};