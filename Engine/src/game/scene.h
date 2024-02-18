#pragma once

#include <vector>
#include <string>
#include <memory>
#include "engine/engine.h"
#include "engine/shared_struct.h"
#include "engine/descriptor_heap.h"
#include "game/resource/model.h"
#include "game/resource/collision_model.h"
#include "game/game.h"
#include "game/entity.h"

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
	virtual void Update(const float delta_time);
	virtual void AfterUpdate();
	virtual void Draw();

	void DrawLighting();
	void DrawSkybox();
	void DrawSSAO();
	void DrawBlurHorizontal();
	void DrawBlurVertical();
	void DrawPostProcess();
	void DrawFXAA();

	Entity* CreateEntity(Entity* entity);
	void AddDestroyEntity(Entity* entity);

	Entity* FindEntityWithTag(const std::string& tag);

	Entity* RootEntity();

	void SetMainCamera(Entity* camera);
	Camera* GetMainCamera();

	//ShadowMap* GetShadowMap();

	//CollisionManager* GetCollisionManager();

	void SetSkybox(const std::wstring& path);

	template<class T> std::shared_ptr<T> LoadResource(const std::wstring& path)	// リソースを読み込む
	{
		return Game::Get()->LoadResource<T>(path);
	}
	

private:
	bool PreparePSO();
	void UpdateCB();
	void UpdateEntityList();

	void DestroyEntities();

	// 指定したメソッドを再帰的に実行する
	template<typename Func>
	void ExecuteOnAllEntities(Func func)
	{
		root_entity_->ExecuteOnAllChildren(func);
	}

private:
	std::unique_ptr<Entity> root_entity_;
	std::vector<Entity*> destroy_entities_;
	std::vector<Entity*> entity_list_;

	Camera* main_camera_ = nullptr;
	//std::unique_ptr<ShadowMap> m_pShadowMap;

	std::unique_ptr<RootSignature> root_signature_;		// ルートシグネチャ
	std::unique_ptr<RootSignature> mesh_root_signature_;

	PipelineState* lighting_pso_;			// ライティング用PSO
	PipelineState* ssao_pso_;				// SSAO用PSO
	PipelineState* blur_horizontal_pso_;	// ブラー（水平）用PSO
	PipelineState* blur_vertical_pso_;		// ブラー（垂直）用PSO
	PipelineState* postprocess_pso_;		// ポストプロセス用PSO
	PipelineState* fxaa_pso_;				// FXAA用PSO
	PipelineState* outline_pso_;
	PipelineState* shadow_pso_;
	PipelineState* depth_pso_;
	PipelineState* gbuffer_pso_;

	std::unique_ptr<ConstantBuffer> transform_cb_[Engine::FRAME_BUFFER_COUNT];	// Transform Constant Buffer
	std::unique_ptr<ConstantBuffer> scene_cb_[Engine::FRAME_BUFFER_COUNT];		// Scene Constant Buffer

	Mesh skybox_mesh_;					// スカイボックスのメッシュデータ
	std::unique_ptr<Texture2D> skybox_tex_;			// スカイボックスのテクスチャ
	DescriptorHandle skybox_handle_;	// スカイボックスのDescriptorHandle
	PipelineState* skybox_pso_;		// スカイボックス用PSO
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