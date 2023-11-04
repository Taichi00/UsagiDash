#pragma once

#include "Component.h"
#include "Engine.h"
#include "BoneList.h"
#include "SharedStruct.h"

class ConstantBuffer;
class RootSignature;
class PipelineState;


struct MeshRendererProperty
{
	Model Model;
};


class MeshRenderer : public Component
{
public:
	MeshRenderer(MeshRendererProperty prop);
	~MeshRenderer();

	void SetProperties(MeshRendererProperty prop);
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
	Bone* FindBone(std::string name);

private:
	Model m_model;

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

	DescriptorHandle* m_pShadowHandle;

	float m_outlineWidth;
};