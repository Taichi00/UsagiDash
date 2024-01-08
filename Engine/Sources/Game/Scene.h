#pragma once

#include <vector>
#include <string>
#include <memory>
#include "Engine.h"
#include "SharedStruct.h"
#include "DescriptorHeap.h"
#include "Model.h"
#include "Game.h"

class Entity;
class Camera;
class ShadowMap;
class CollisionManager;
class Collider;
class Rigidbody;
class PipelineState;
class RootSignature;
class ConstantBuffer;
class Texture2D;
class Resource;

class Scene
{
public:
	Scene();
	~Scene();

	virtual bool Init();
	virtual void Update();
	virtual void Draw();

	void DrawLighting();
	void DrawSkybox();
	void DrawSSAO();
	void DrawBlurHorizontal();
	void DrawBlurVertical();
	void DrawPostProcess();
	void DrawFXAA();

	void CreateEntity(Entity* entity);

	void SetMainCamera(Entity* camera);
	Camera* GetMainCamera();

	//ShadowMap* GetShadowMap();

	CollisionManager* GetCollisionManager();

	void SetSkybox(const std::string path);

	template<class T> std::shared_ptr<T> LoadResource(std::string path)	// ���\�[�X��ǂݍ���
	{
		return Game::Get()->LoadResource<T>(path);
	}
	

private:
	bool PreparePSO();
	void UpdateCB();

private:
	std::vector<std::unique_ptr<Entity>> m_pEntities;

	Camera* m_pMainCamera = nullptr;
	//std::unique_ptr<ShadowMap> m_pShadowMap;
	std::unique_ptr<CollisionManager> m_pCollisionManager;

	std::unique_ptr<RootSignature> m_pRootSignature;		// ���[�g�V�O�l�`��

	std::unique_ptr<PipelineState> m_pLightingPSO;			// ���C�e�B���O�pPSO
	std::unique_ptr<PipelineState> m_pSSAOPSO;				// SSAO�pPSO
	std::unique_ptr<PipelineState> m_pBlurHorizontalPSO;	// �u���[�i�����j�pPSO
	std::unique_ptr<PipelineState> m_pBlurVerticalPSO;		// �u���[�i�����j�pPSO
	std::unique_ptr<PipelineState> m_pPostProcessPSO;		// �|�X�g�v���Z�X�pPSO
	std::unique_ptr<PipelineState> m_pFXAAPSO;				// FXAA�pPSO

	std::unique_ptr<ConstantBuffer> m_pTransformCB[Engine::FRAME_BUFFER_COUNT];	// Transform Constant Buffer
	std::unique_ptr<ConstantBuffer> m_pSceneCB[Engine::FRAME_BUFFER_COUNT];		// Scene Constant Buffer

	Mesh m_skyboxMesh;					// �X�J�C�{�b�N�X�̃��b�V���f�[�^
	std::unique_ptr<Texture2D> m_pSkyboxTex;			// �X�J�C�{�b�N�X�̃e�N�X�`��
	DescriptorHandle m_pSkyboxHandle;	// �X�J�C�{�b�N�X��DescriptorHandle
	std::unique_ptr<PipelineState> m_pSkyboxPSO;		// �X�J�C�{�b�N�X�pPSO
	std::unique_ptr<RootSignature> m_pSkyboxRootSignature;	// �X�J�C�{�b�N�X�p���[�g�V�O�l�`��

	std::unique_ptr<Texture2D> m_pDiffuseMapTex;			// DiffuseMap �̃e�N�X�`��
	DescriptorHandle m_pDiffuseMapHandle;	// DiffuseMap �� DescriptorHandle

	std::unique_ptr<Texture2D> m_pSpecularMapTex;			// SpecularMap �̃e�N�X�`��
	DescriptorHandle m_pSpecularMapHandle;	// SpecularMap �� DescriptorHandle

	std::unique_ptr<Texture2D> m_pBrdfTex;					// BRDF �̃e�N�X�`��
	DescriptorHandle m_pBrdfHandle;		// BRDF �� DescriptorHandle

	std::shared_ptr<DescriptorHeap> m_pRtvHeap;
	std::shared_ptr<DescriptorHeap> m_pDsvHeap;
	std::shared_ptr<DescriptorHeap> m_pGBufferHeap;
};