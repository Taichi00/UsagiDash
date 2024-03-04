#include "particle_renderer.h"
#include "game/game.h"
#include "engine/constant_buffer.h"
#include "game/component/particle_emitter.h"
#include "engine/root_signature.h"

ParticleRenderer::ParticleRenderer(ParticleEmitter* particle_emitter)
{
	particle_emitter_ = particle_emitter;

	if (particle_emitter_)
		prop_ = particle_emitter_->GetProperety();
}

ParticleRenderer::~ParticleRenderer()
{
}

bool ParticleRenderer::Init()
{
    PrepareModel();
    if (!PrepareCB()) return false;
    if (!PrepareRootSignature()) return false;
    if (!PreparePSO()) return false;
    return true;
}

void ParticleRenderer::Update()
{
	UpdateCB();
}

bool ParticleRenderer::PrepareCB()
{
	auto engine = Game::Get()->GetEngine();

	// Transform CB
	for (size_t i = 0; i < Engine::FRAME_BUFFER_COUNT; i++)
	{
		transform_cb_[i] = engine->CreateConstantBuffer(sizeof(TransformParameter));
		scene_cb_[i] = engine->CreateConstantBuffer(sizeof(SceneParameter));
		particle_cb_[i] = engine->CreateConstantBuffer(sizeof(ParticleParameter));
	}

	// Material CBs
	for (size_t i = 0; i < particle_model_->materials.size(); i++)
	{
		auto cb = engine->CreateConstantBuffer(sizeof(MaterialParameter));

		auto mat = &particle_model_->materials[i];
		auto ptr = cb->GetPtr<MaterialParameter>();

		ptr->base_color = mat->base_color;

		materials_cb_.push_back(std::move(cb));
	}

	return true;
}
