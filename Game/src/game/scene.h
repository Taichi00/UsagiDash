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

	// �G���e�B�e�B�𐶐�����
	Entity* CreateEntity(Entity* entity);

	// �폜����G���e�B�e�B�̃��X�g�ɒǉ�����
	void AddDestroyEntity(Entity* entity);

	// �V�[���j�����ɔj�����Ȃ��G���e�B�e�B��ǉ�
	bool DontDestroyOnLoad(Entity* entity);
	std::vector<std::unique_ptr<Entity>> MoveDontDestroyEntities();

	// ���O����G���e�B�e�B���擾����
	Entity* FindEntity(const std::string& name);
	Entity* FindEntityWithTag(const std::string& tag);

	// ���[�g�G���e�B�e�B���擾����
	Entity* RootEntity();

	// ���C���J������ݒ肷��
	void SetMainCamera(Entity* camera);
	Camera* GetMainCamera();

	// �X�J�C�{�b�N�X��ݒ肷��
	void SetSkybox(const std::wstring& path);

	// ���\�[�X��ǂݍ���
	template<class T> std::shared_ptr<T> LoadResource(const std::wstring& path)
	{
		return Game::Get()->LoadResource<T>(path);
	}

private:
	bool PreparePSO();
	void UpdateCB();
	void UpdateEntityList();

	// �G���e�B�e�B���폜����
	void DestroyEntities();

	// �w�肵�����\�b�h���ċA�I�Ɏ��s����
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

	std::unique_ptr<RootSignature> root_signature_;		// ���[�g�V�O�l�`��
	std::unique_ptr<RootSignature> mesh_root_signature_;

	PipelineState* lighting_pso_ = nullptr;			// ���C�e�B���O�pPSO
	PipelineState* ssao_pso_ = nullptr;				// SSAO�pPSO
	PipelineState* blur_horizontal_pso_ = nullptr;	// �u���[�i�����j�pPSO
	PipelineState* blur_vertical_pso_ = nullptr;	// �u���[�i�����j�pPSO
	PipelineState* postprocess_pso_ = nullptr;		// �|�X�g�v���Z�X�pPSO
	PipelineState* fxaa_pso_ = nullptr;				// FXAA�pPSO
	PipelineState* outline_pso_ = nullptr;
	PipelineState* shadow_pso_ = nullptr;
	PipelineState* depth_pso_ = nullptr;
	PipelineState* gbuffer_pso_ = nullptr;
	PipelineState* skybox_pso_ = nullptr; // �X�J�C�{�b�N�X�pPSO

	std::unique_ptr<ConstantBuffer> transform_cb_[Engine::FRAME_BUFFER_COUNT];	// Transform Constant Buffer
	std::unique_ptr<ConstantBuffer> scene_cb_[Engine::FRAME_BUFFER_COUNT];		// Scene Constant Buffer

	Mesh skybox_mesh_; // �X�J�C�{�b�N�X�̃��b�V���f�[�^
	std::shared_ptr<Texture2D> skybox_tex_; // �X�J�C�{�b�N�X�̃e�N�X�`��
	DescriptorHandle skybox_handle_; // �X�J�C�{�b�N�X��DescriptorHandle
	std::unique_ptr<RootSignature> skybox_root_signature_; // �X�J�C�{�b�N�X�p���[�g�V�O�l�`��

	std::shared_ptr<Texture2D> diffusemap_tex_; // DiffuseMap �̃e�N�X�`��
	DescriptorHandle diffusemap_handle_; // DiffuseMap �� DescriptorHandle

	std::shared_ptr<Texture2D> specularmap_tex_; // SpecularMap �̃e�N�X�`��
	DescriptorHandle specularmap_handle_; // SpecularMap �� DescriptorHandle

	std::shared_ptr<Texture2D> brdf_tex_; // BRDF �̃e�N�X�`��
	DescriptorHandle brdf_handle_; // BRDF �� DescriptorHandle

	std::shared_ptr<DescriptorHeap> rtv_heap_;
	std::shared_ptr<DescriptorHeap> dsv_heap_;
	std::shared_ptr<DescriptorHeap> gbuffer_heap_;
};