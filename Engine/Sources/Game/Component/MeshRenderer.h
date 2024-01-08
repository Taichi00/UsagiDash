#pragma once

#include "Component.h"
#include "Engine.h"
#include "BoneList.h"
#include "Model.h"
#include "DescriptorHeap.h"
#include <memory>

class ConstantBuffer;
class RootSignature;
class PipelineState;

class MeshRenderer : public Component
{
public:
	MeshRenderer() {};
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
	bool isShadowFromAbove = false;	// 上から影を落とすかどうか

protected:
	std::shared_ptr<Model> m_pModel;	// モデルデータへのポインタ
	BoneList m_bones;	// モデルのボーン

	ConstantBuffer* m_pTransformCB[Engine::FRAME_BUFFER_COUNT];
	ConstantBuffer* m_pSceneCB[Engine::FRAME_BUFFER_COUNT];
	ConstantBuffer* m_pBoneCB[Engine::FRAME_BUFFER_COUNT];
	std::vector<ConstantBuffer*> m_pMaterialCBs;
	RootSignature* m_pRootSignature;

	PipelineState* m_pOpaquePSO;
	PipelineState* m_pAlphaPSO;
	PipelineState* m_pOutlinePSO;
	PipelineState* m_pShadowPSO;
	PipelineState* m_pDepthPSO;
	PipelineState* m_pGBufferPSO;

	DescriptorHeap* m_pDescriptorHeap;
	std::vector<DescriptorHandle> m_albedoHandles;
	std::vector<DescriptorHandle> m_pbrHandles;
	std::vector<DescriptorHandle> m_normalHandles;

	DescriptorHandle m_pShadowHandle;

	float m_outlineWidth;
};