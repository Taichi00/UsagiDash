#pragma once

#include "game/particle_renderer_opaque.h"

class ParticleEmitter;

class ParticleRendererMesh : public ParticleRendererOpaque
{
public:
	ParticleRendererMesh(ParticleEmitter* particle_emitter) : ParticleRendererOpaque(particle_emitter) {}
	~ParticleRendererMesh() {}

private:
	void PrepareModel() override;
	bool PreparePSO() override;

	void UpdateCB() override;
};