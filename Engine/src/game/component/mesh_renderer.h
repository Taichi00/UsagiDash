#pragma once

#include "game/component/component.h"
#include "engine/engine.h"
#include "game/bone_list.h"
#include "game/resource/model.h"
#include "engine/descriptor_heap.h"
#include <memory>

class ConstantBuffer;
class RootSignature;
class PipelineState;
class PipelineStateManager;

class MeshRenderer : public Component
{
public:
	MeshRenderer();
	MeshRenderer(std::shared_ptr<Model> model);
	~MeshRenderer();

	void SetOutlineWidth(float width);

	bool Init() override;
	void Update() override;
	void Draw() override;
	void DrawAlpha() override;
	void DrawShadow() override;

	void DrawDepth() override;
	void DrawGBuffer() override;
	void DrawOutline() override;

private:
	bool PreparePSO();

	void UpdateBone();
	void UpdateCB();

public:
	const Model& GetModel() const;

	Bone* FindBone(std::string name);
	BoneList* GetBones();

public:
	bool is_shadow_from_above = false;	// 上から影を落とすかどうか

protected:
	std::shared_ptr<Model> model_;	// モデルデータへのポインタ
	BoneList bones_;	// モデルのボーン

	std::unique_ptr<ConstantBuffer> transform_cb_[Engine::FRAME_BUFFER_COUNT];
	std::unique_ptr<ConstantBuffer> scene_cb_[Engine::FRAME_BUFFER_COUNT];
	std::unique_ptr<ConstantBuffer> bone_cb_[Engine::FRAME_BUFFER_COUNT];
	std::vector<std::unique_ptr<ConstantBuffer>> materials_cb_;

	PipelineStateManager* pipeline_manager_;

	float outline_width_;
};