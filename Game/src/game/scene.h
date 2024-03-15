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

	virtual void Awake();
	virtual bool Init();
	virtual void Update(const float delta_time);
	virtual void AfterUpdate();
	virtual void Draw();

	void OnDestroy();

	void DrawLighting();
	void DrawSkybox();
	void DrawSSAO();
	void DrawBlurHorizontal();
	void DrawBlurVertical();
	void DrawPostProcess();
	void DrawFXAA();

	// エンティティを生成する
	Entity* CreateEntity(Entity* entity);

	// 削除するエンティティのリストに追加する
	void AddDestroyEntity(Entity* entity);

	// シーン破棄時に破棄しないエンティティを追加
	bool DontDestroyOnLoad(Entity* entity);
	std::vector<std::unique_ptr<Entity>> MoveDontDestroyEntities();

	// 名前からエンティティを取得する
	Entity* FindEntity(const std::string& name);
	Entity* FindEntityWithTag(const std::string& tag);

	// ルートエンティティを取得する
	Entity* RootEntity();

	// メインカメラを設定する
	void SetMainCamera(Entity* camera);
	Camera* GetMainCamera();

	// スカイボックスを設定する
	void SetSkybox(const std::wstring& path);

	// リソースを読み込む
	template<class T> std::shared_ptr<T> LoadResource(const std::wstring& path)
	{
		return Game::Get()->LoadResource<T>(path);
	}

private:
	bool PreparePSO();
	void UpdateCB();
	void UpdateEntityList();

	// エンティティを削除する
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
	std::vector<Entity*> dont_destroy_entities_;
	std::vector<Entity*> entity_list_;

	Camera* main_camera_ = nullptr;

	std::unique_ptr<RootSignature> root_signature_;		// ルートシグネチャ
	std::unique_ptr<RootSignature> mesh_root_signature_;

	PipelineState* lighting_pso_ = nullptr;			// ライティング用PSO
	PipelineState* ssao_pso_ = nullptr;				// SSAO用PSO
	PipelineState* blur_horizontal_pso_ = nullptr;	// ブラー（水平）用PSO
	PipelineState* blur_vertical_pso_ = nullptr;	// ブラー（垂直）用PSO
	PipelineState* postprocess_pso_ = nullptr;		// ポストプロセス用PSO
	PipelineState* fxaa_pso_ = nullptr;				// FXAA用PSO
	PipelineState* outline_pso_ = nullptr;
	PipelineState* shadow_pso_ = nullptr;
	PipelineState* depth_pso_ = nullptr;
	PipelineState* gbuffer_pso_ = nullptr;
	PipelineState* skybox_pso_ = nullptr; // スカイボックス用PSO

	std::unique_ptr<ConstantBuffer> transform_cb_[Engine::FRAME_BUFFER_COUNT];	// Transform Constant Buffer
	std::unique_ptr<ConstantBuffer> scene_cb_[Engine::FRAME_BUFFER_COUNT];		// Scene Constant Buffer

	Mesh skybox_mesh_; // スカイボックスのメッシュデータ
	std::shared_ptr<Texture2D> skybox_tex_; // スカイボックスのテクスチャ
	DescriptorHandle skybox_handle_; // スカイボックスのDescriptorHandle
	std::unique_ptr<RootSignature> skybox_root_signature_; // スカイボックス用ルートシグネチャ

	std::shared_ptr<Texture2D> diffusemap_tex_; // DiffuseMap のテクスチャ
	DescriptorHandle diffusemap_handle_; // DiffuseMap の DescriptorHandle

	std::shared_ptr<Texture2D> specularmap_tex_; // SpecularMap のテクスチャ
	DescriptorHandle specularmap_handle_; // SpecularMap の DescriptorHandle

	std::shared_ptr<Texture2D> brdf_tex_; // BRDF のテクスチャ
	DescriptorHandle brdf_handle_; // BRDF の DescriptorHandle

	std::shared_ptr<DescriptorHeap> rtv_heap_;
	std::shared_ptr<DescriptorHeap> dsv_heap_;
	std::shared_ptr<DescriptorHeap> gbuffer_heap_;
};