#pragma once
#include "Entity.h"
#include "Engine.h"
#include "ConstantBuffer.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "DescriptorHeap.h"
#include "SharedStruct.h"
#include "BoneList.h"
#include <vector>

class ShadowMap;
class Game;
class DescriptorHandle;
class Bone;
class BoneList;
class Animator;
class Animation;

class MeshEntity : public Entity
{
public:
	MeshEntity(
		std::vector<Mesh> meshes, std::vector<Material> materials, 
		BoneList bones, std::vector<Animation*> animations);
	~MeshEntity();

	static MeshEntity* LoadModel(const wchar_t* path);

	Animator* GetAnimator();

private:
	bool Init() override;
	void Update() override;
	void Draw() override;
	void DrawAlpha() override;
	void DrawShadow() override;

	bool PreparePSO();

private:
	void UpdateAnimator();
	void UpdateBone();
	void UpdateCB();

private:
	//Model m_model;

	std::vector<Mesh> m_meshes;
	std::vector<Material> m_materials;
	BoneList m_bones;

	ConstantBuffer* m_pTransformCB[Engine::FRAME_BUFFER_COUNT];
	ConstantBuffer* m_pLightCB[Engine::FRAME_BUFFER_COUNT];
	ConstantBuffer* m_pBoneCB[Engine::FRAME_BUFFER_COUNT];
	RootSignature* m_pRootSignature;

	PipelineState* m_pOpaquePSO;
	PipelineState* m_pAlphaPSO;
	PipelineState* m_pOutlinePSO;
	PipelineState* m_pShadowPSO;

	DescriptorHeap* m_pDescriptorHeap;

	DescriptorHandle* m_pShadowHandle;

	Animator* m_pAnimator;
};