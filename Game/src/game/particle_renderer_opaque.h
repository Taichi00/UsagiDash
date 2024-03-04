#pragma once

#include "game/particle_renderer.h"

class ParticleEmitter;

class ParticleRendererOpaque : public ParticleRenderer
{
public:
	ParticleRendererOpaque(ParticleEmitter* particle_emitter) : ParticleRenderer(particle_emitter) {}
	~ParticleRendererOpaque() {}

	void DrawShadow() override;
	void DrawDepth() override;
	void DrawGBuffer() override;

private:
	bool PrepareRootSignature() override;
};