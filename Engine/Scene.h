#pragma once

#include <vector>
#include <string>
#include "Engine.h"
#include "SharedStruct.h"

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
class DescriptorHandle;

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

	ShadowMap* GetShadowMap();

	CollisionManager* GetCollisionManager();

	void SetSkybox(const std::string path);

private:
	bool PreparePSO();
	void UpdateCB();

private:
	std::vector<Entity*> m_pEntities;

	Camera* m_pMainCamera = nullptr;
	ShadowMap* m_pShadowMap;
	CollisionManager* m_pCollisionManager;

	RootSignature* m_pRootSignature;
	PipelineState* m_pLightingPSO;
	PipelineState* m_pSSAOPSO;
	PipelineState* m_pBlurHorizontalPSO;
	PipelineState* m_pBlurVerticalPSO;
	PipelineState* m_pPostProcessPSO;
	PipelineState* m_pFXAAPSO;

	ConstantBuffer* m_pTransformCB[Engine::FRAME_BUFFER_COUNT];
	ConstantBuffer* m_pSceneCB[Engine::FRAME_BUFFER_COUNT];

	//DescriptorHandle* m_pShadowHandle;

	Mesh m_skyboxMesh;
	Texture2D* m_pSkyboxTex;
	DescriptorHandle* m_pSkyboxHandle;
	PipelineState* m_pSkyboxPSO;
	RootSignature* m_pSkyboxRootSignature;

	Texture2D* m_pDiffuseMapTex;
	DescriptorHandle* m_pDiffuseMapHandle;

	Texture2D* m_pSpecularMapTex;
	DescriptorHandle* m_pSpecularMapHandle;

	Texture2D* m_pBrdfTex;
	DescriptorHandle* m_pBrdfHandle;
};