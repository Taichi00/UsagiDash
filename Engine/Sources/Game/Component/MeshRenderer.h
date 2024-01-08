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
	bool isShadowFromAbove = false;	// 上から影を落とすかどうか

protected:
	std::shared_ptr<Model> m_pModel;	// モデルデータへのポインタ
	BoneList m_bones;	// モデルのボーン

	std::unique_ptr<ConstantBuffer> m_pTransformCB[Engine::FRAME_BUFFER_COUNT];
	std::unique_ptr<ConstantBuffer> m_pSceneCB[Engine::FRAME_BUFFER_COUNT];
	std::unique_ptr<ConstantBuffer> m_pBoneCB[Engine::FRAME_BUFFER_COUNT];
	std::vector<std::unique_ptr<ConstantBuffer>> m_pMaterialCBs;
	std::unique_ptr<RootSignature> m_pRootSignature;

	std::unique_ptr<PipelineState> m_pOpaquePSO;
	std::unique_ptr<PipelineState> m_pAlphaPSO;
	std::unique_ptr<PipelineState> m_pOutlinePSO;
	std::unique_ptr<PipelineState> m_pShadowPSO;
	std::unique_ptr<PipelineState> m_pDepthPSO;
	std::unique_ptr<PipelineState> m_pGBufferPSO;

	std::unique_ptr<DescriptorHeap> m_pDescriptorHeap;
	std::vector<DescriptorHandle> m_albedoHandles;
	std::vector<DescriptorHandle> m_pbrHandles;
	std::vector<DescriptorHandle> m_normalHandles;

	float m_outlineWidth;
};