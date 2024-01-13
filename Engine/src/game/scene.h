#pragma once

#include <vector>
#include <string>
#include <memory>
#include "engine/engine.h"
#include "engine/shared_struct.h"
#include "engine/descriptor_heap.h"
#include "game/model.h"
#include "game/game.h"

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
	std::vector<std::unique_ptr<Entity>> entities_;

	Camera* main_camera_ = nullptr;
	//std::unique_ptr<ShadowMap> m_pShadowMap;
	std::unique_ptr<CollisionManager> collision_manager_;

	std::unique_ptr<RootSignature> root_signature_;		// ルートシグネチャ

	std::unique_ptr<PipelineState> lighting_pso_;			// ライティング用PSO
	std::unique_ptr<PipelineState> ssao_pso_;				// SSAO用PSO
	std::unique_ptr<PipelineState> blur_horizontal_pso_;	// ブラー（水平）用PSO
	std::unique_ptr<PipelineState> blur_vertical_pso_;		// ブラー（垂直）用PSO
	std::unique_ptr<PipelineState> postprocess_pso_;		// ポストプロセス用PSO
	std::unique_ptr<PipelineState> fxaa_pso_;				// FXAA用PSO

	std::unique_ptr<ConstantBuffer> transform_cb_[Engine::FRAME_BUFFER_COUNT];	// Transform Constant Buffer
	std::unique_ptr<ConstantBuffer> scene_cb_[Engine::FRAME_BUFFER_COUNT];		// Scene Constant Buffer

	Mesh skybox_mesh_;					// スカイボックスのメッシュデータ
	std::unique_ptr<Texture2D> skybox_tex_;			// スカイボックスのテクスチャ
	DescriptorHandle skybox_handle_;	// スカイボックスのDescriptorHandle
	std::unique_ptr<PipelineState> skybox_pso_;		// スカイボックス用PSO
	std::unique_ptr<RootSignature> skybox_root_signature_;	// スカイボックス用ルートシグネチャ

	std::unique_ptr<Texture2D> diffusemap_tex_;			// DiffuseMap のテクスチャ
	DescriptorHandle diffusemap_handle_;	// DiffuseMap の DescriptorHandle

	std::unique_ptr<Texture2D> specularmap_tex_;			// SpecularMap のテクスチャ
	DescriptorHandle specularmap_handle_;	// SpecularMap の DescriptorHandle

	std::unique_ptr<Texture2D> brdf_tex_;					// BRDF のテクスチャ
	DescriptorHandle brdf_handle_;		// BRDF の DescriptorHandle

	std::shared_ptr<DescriptorHeap> rtv_heap_;
	std::shared_ptr<DescriptorHeap> dsv_heap_;
	std::shared_ptr<DescriptorHeap> gbuffer_heap_;
};