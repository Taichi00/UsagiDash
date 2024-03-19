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

	bool Init() override;
	void Update(const float delta_time) override;
	void BeforeDraw() override;
	void Draw() override;
	void DrawAlpha() override;
	void DrawShadow() override;
	void DrawDepth() override;
	void DrawGBuffer() override;
	void DrawOutline() override;

	// �A�E�g���C���̑�����ݒ肷��
	void SetOutlineWidth(const float width) { outline_width_ = width; }

	// �f�B�U�̃��x����ݒ肷��
	void SetDitherLevel(const float level) { dither_level_ = level; }

private:
	void UpdateBone();
	void UpdateCB();

public:
	const Model& GetModel() const;

	Bone* FindBone(std::string name);
	BoneList* GetBones();

protected:
	std::shared_ptr<Model> model_;	// ���f���f�[�^�ւ̃|�C���^
	BoneList bones_;	// ���f���̃{�[��

	std::unique_ptr<ConstantBuffer> transform_cb_[Engine::FRAME_BUFFER_COUNT];
	std::unique_ptr<ConstantBuffer> scene_cb_[Engine::FRAME_BUFFER_COUNT];
	std::unique_ptr<ConstantBuffer> bone_cb_[Engine::FRAME_BUFFER_COUNT];
	std::vector<std::unique_ptr<ConstantBuffer>> materials_cb_;

	PipelineStateManager* pipeline_manager_ = nullptr;

	// �A�E�g���C���̑���
	float outline_width_;

	// �f�B�U�̃��x�� (0-1)
	float dither_level_ = 0;
};