#pragma once

#include "engine/engine.h"
#include "engine/shared_struct.h"
#include "game/component/particle_emitter.h"
#include "engine/pipeline_state.h"
#include "engine/root_signature.h"
#include <memory>
#include <vector>

class ConstantBuffer;
class Model;

class ParticleRenderer
{
public:
	ParticleRenderer(ParticleEmitter* particle_emitter);
	virtual ~ParticleRenderer();

	bool Init();

	void Update();

	virtual void DrawShadow() {}
	virtual void DrawDepth() {}
	virtual void DrawGBuffer() {}

protected:
	virtual void PrepareModel() {}	// モデルデータの準備
	bool PrepareCB();		// ConstantBufferの生成
	virtual bool PrepareRootSignature() { return false; }	// RootSignatureの生成
	virtual bool PreparePSO() { return false; }		// PipelineStateObjectの生成

	// ConstantBuffer の更新
	virtual void UpdateCB() {}

protected:
	ParticleEmitter* particle_emitter_ = nullptr;

	ParticleEmitterProperty prop_ = {};

	std::shared_ptr<Model> particle_model_; // モデルデータ

	std::unique_ptr<ConstantBuffer> transform_cb_[Engine::FRAME_BUFFER_COUNT];	// Transform Constant Buffer
	std::unique_ptr<ConstantBuffer> scene_cb_[Engine::FRAME_BUFFER_COUNT];		// Scene Constant Buffer
	std::vector<std::unique_ptr<ConstantBuffer>> materials_cb_;				// Material Constant Buffers
	std::unique_ptr<ConstantBuffer> particle_cb_[Engine::FRAME_BUFFER_COUNT];	// Particle Constant Buffer

	std::unique_ptr<RootSignature> root_signature_;	// Root Signature

	// Pipeline State Object
	std::unique_ptr<PipelineState> shadow_pso_;
	std::unique_ptr<PipelineState> depth_pso_;
	std::unique_ptr<PipelineState> gbuffer_pso_;
};