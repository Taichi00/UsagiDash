#pragma once

#include "game/particle_renderer_opaque.h"

class ParticleEmitter;

class ParticleRendererSprite : public ParticleRendererOpaque
{
public:
	ParticleRendererSprite(ParticleEmitter* particle_emitter) : ParticleRendererOpaque(particle_emitter) {}
	~ParticleRendererSprite() {}

private:
	void PrepareModel() override;
	bool PreparePSO() override;

	void UpdateCB() override;
};