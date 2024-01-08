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

	template<class T> std::shared_ptr<T> LoadResource(std::string path)	// リソースを読み込む
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

	std::unique_ptr<RootSignature> m_pRootSignature;		// ルートシグネチャ

	std::unique_ptr<PipelineState> m_pLightingPSO;			// ライティング用PSO
	std::unique_ptr<PipelineState> m_pSSAOPSO;				// SSAO用PSO
	std::unique_ptr<PipelineState> m_pBlurHorizontalPSO;	// ブラー（水平）用PSO
	std::unique_ptr<PipelineState> m_pBlurVerticalPSO;		// ブラー（垂直）用PSO
	std::unique_ptr<PipelineState> m_pPostProcessPSO;		// ポストプロセス用PSO
	std::unique_ptr<PipelineState> m_pFXAAPSO;				// FXAA用PSO

	std::unique_ptr<ConstantBuffer> m_pTransformCB[Engine::FRAME_BUFFER_COUNT];	// Transform Constant Buffer
	std::unique_ptr<ConstantBuffer> m_pSceneCB[Engine::FRAME_BUFFER_COUNT];		// Scene Constant Buffer

	Mesh m_skyboxMesh;					// スカイボックスのメッシュデータ
	std::unique_ptr<Texture2D> m_pSkyboxTex;			// スカイボックスのテクスチャ
	DescriptorHandle m_pSkyboxHandle;	// スカイボックスのDescriptorHandle
	std::unique_ptr<PipelineState> m_pSkyboxPSO;		// スカイボックス用PSO
	std::unique_ptr<RootSignature> m_pSkyboxRootSignature;	// スカイボックス用ルートシグネチャ

	std::unique_ptr<Texture2D> m_pDiffuseMapTex;			// DiffuseMap のテクスチャ
	DescriptorHandle m_pDiffuseMapHandle;	// DiffuseMap の DescriptorHandle

	std::unique_ptr<Texture2D> m_pSpecularMapTex;			// SpecularMap のテクスチャ
	DescriptorHandle m_pSpecularMapHandle;	// SpecularMap の DescriptorHandle

	std::unique_ptr<Texture2D> m_pBrdfTex;					// BRDF のテクスチャ
	DescriptorHandle m_pBrdfHandle;		// BRDF の DescriptorHandle

	std::shared_ptr<DescriptorHeap> m_pRtvHeap;
	std::shared_ptr<DescriptorHeap> m_pDsvHeap;
	std::shared_ptr<DescriptorHeap> m_pGBufferHeap;
};