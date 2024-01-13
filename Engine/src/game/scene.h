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

	template<class T> std::shared_ptr<T> LoadResource(std::string path)	// ���\�[�X��ǂݍ���
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

	std::unique_ptr<RootSignature> root_signature_;		// ���[�g�V�O�l�`��

	std::unique_ptr<PipelineState> lighting_pso_;			// ���C�e�B���O�pPSO
	std::unique_ptr<PipelineState> ssao_pso_;				// SSAO�pPSO
	std::unique_ptr<PipelineState> blur_horizontal_pso_;	// �u���[�i�����j�pPSO
	std::unique_ptr<PipelineState> blur_vertical_pso_;		// �u���[�i�����j�pPSO
	std::unique_ptr<PipelineState> postprocess_pso_;		// �|�X�g�v���Z�X�pPSO
	std::unique_ptr<PipelineState> fxaa_pso_;				// FXAA�pPSO

	std::unique_ptr<ConstantBuffer> transform_cb_[Engine::FRAME_BUFFER_COUNT];	// Transform Constant Buffer
	std::unique_ptr<ConstantBuffer> scene_cb_[Engine::FRAME_BUFFER_COUNT];		// Scene Constant Buffer

	Mesh skybox_mesh_;					// �X�J�C�{�b�N�X�̃��b�V���f�[�^
	std::unique_ptr<Texture2D> skybox_tex_;			// �X�J�C�{�b�N�X�̃e�N�X�`��
	DescriptorHandle skybox_handle_;	// �X�J�C�{�b�N�X��DescriptorHandle
	std::unique_ptr<PipelineState> skybox_pso_;		// �X�J�C�{�b�N�X�pPSO
	std::unique_ptr<RootSignature> skybox_root_signature_;	// �X�J�C�{�b�N�X�p���[�g�V�O�l�`��

	std::unique_ptr<Texture2D> diffusemap_tex_;			// DiffuseMap �̃e�N�X�`��
	DescriptorHandle diffusemap_handle_;	// DiffuseMap �� DescriptorHandle

	std::unique_ptr<Texture2D> specularmap_tex_;			// SpecularMap �̃e�N�X�`��
	DescriptorHandle specularmap_handle_;	// SpecularMap �� DescriptorHandle

	std::unique_ptr<Texture2D> brdf_tex_;					// BRDF �̃e�N�X�`��
	DescriptorHandle brdf_handle_;		// BRDF �� DescriptorHandle

	std::shared_ptr<DescriptorHeap> rtv_heap_;
	std::shared_ptr<DescriptorHeap> dsv_heap_;
	std::shared_ptr<DescriptorHeap> gbuffer_heap_;
};