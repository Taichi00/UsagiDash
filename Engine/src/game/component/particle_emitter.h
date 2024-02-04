#pragma once

#include "game/component/component.h"
#include "engine/descriptor_heap.h"
#include "engine/engine.h"
#include "game/resource/model.h"
#include "game/particle.h"
#include "game/scene.h"
#include "game/resource/texture2d.h"
#include "math/vec.h"
#include <DirectXMath.h>
#include <memory>
#include <random>
#include <vector>


const int MAX_PARTICLE_COUNT = 500;
const float PARTICLE_SPRITE_SIZE_PER_PIXEL = 0.01;


class ConstantBuffer;
class PipelineState;
class RootSignature;


struct alignas(256) ParticleData
{
	XMMATRIX world;
	XMMATRIX lightWorld;
};

struct alignas(256) ParticleParameter
{
	ParticleData data[MAX_PARTICLE_COUNT];
};


enum ParticleEmitterPropertyType
{
	PARTICAL_PROP_TYPE_PVA = 0,
	PARTICAL_PROP_TYPE_EASING,
};

enum ParticleEmitterSpawningMethod
{
	PARTICAL_SPAWN_METHOD_POINT = 0,
	PARTICAL_SPAWN_METHOD_CIRCLE,
};

enum ParticleEmitterEasingType
{
	PARTICLE_EASE_LINEAR = 0,
	PARTICLE_EASE_OUT_CUBIC,
	PARTICLE_EASE_INOUT_CUBIC,
};

struct ParticleEmitterPositionPropertyPVA
{
	Vec3 position;					// �ʒu
	Vec3 position_range;				// �ʒu�̗����͈�
	Vec3 velocity;					// ���x
	Vec3 velocity_range;				// ���x�̗����͈�
	Vec3 acceleration;				// �����x
	Vec3 acceleration_range;			// �����x�̗����͈�
};

struct ParticleEmitterPositionPropertyEasing
{
	Vec3 start;						// �J�n�ʒu
	Vec3 start_range;				// �J�n�ʒu�̗����͈�
	Vec3 end;						// �I���ʒu
	Vec3 end_range;					// �I���ʒu�̗����͈�
	ParticleEmitterEasingType type = PARTICLE_EASE_LINEAR;	// ��Ԃ̃^�C�v
	bool middle_enabled = false;		// ���ԓ_���g�p���邩
	Vec3 middle;					// ���Ԉʒu
	Vec3 middle_range;				// ���Ԉʒu�̗����͈�
};

struct ParticleEmitterRotationPropertyPVA
{
	Vec3 rotation;					// �p�x
	Vec3 rotation_range;				// �p�x�̗����͈�
	Vec3 velocity;					// ���x
	Vec3 velocity_range;				// ���x�̗����͈�
	Vec3 acceleration;				// �����x
	Vec3 acceleration_range;			// �����x�̗����͈�
};

struct ParticleEmitterScalePropertyPVA
{
	Vec3 scale = Vec3(1, 1, 1);		// �g�嗦
	Vec3 scale_range;				// �g�嗦�̗����͈�
	Vec3 velocity;					// ���x
	Vec3 velocity_range;				// ���x�̗����͈�
	Vec3 acceleration;				// �����x
	Vec3 acceleration_range;			// �����x�̗����͈�
};

struct ParticleEmitterScalePropertyEasing
{
	Vec3 start = Vec3(1, 1, 1);		// �J�n�g�嗦
	Vec3 start_range;				// �J�n�g�嗦�̗����͈�
	Vec3 end = Vec3(1, 1, 1);		// �I���g�嗦
	Vec3 end_range;					// �I���g�嗦�̗����͈�
	ParticleEmitterEasingType type = PARTICLE_EASE_LINEAR;	// ��Ԃ̃^�C�v
	bool middle_enabled = false;		// ���ԓ_���g�p���邩
	Vec3 middle = Vec3(1, 1, 1);	// ���Ԋg�嗦
	Vec3 middle_range;				// ���Ԋg�嗦�̗����͈�
	bool keep_aspect = false;		// �A�X�y�N�g����ێ����邩
};

struct ParticleEmitterSpriteProperty
{
	std::shared_ptr<Texture2D> albedo_texture;		// Albedo
	std::shared_ptr<Texture2D> normal_texture;		// Normal
	std::shared_ptr<Texture2D> pbr_texture;			// MetallicRoughness
};

struct ParticleEmitterSpawningPoint
{
	Vec3 location;					// �����ʒu
	Vec3 location_range;				// �����ʒu�̗����͈�
};

struct ParticleEmitterSpawningCircle
{
	unsigned int vertices = 8;		// ���_�̐�
	float radius = 1;				// ���a
	float radius_range = 0;			// ���a�̗����͈�
};

struct ParticleEmitterProperty
{
	unsigned int spawn_count = 0;	// �������i0 = ����)
	float time_to_live = 100;			// ��������
	float time_to_live_range = 0;		// �������Ԃ̗����͈�
	float spawn_rate = 1;			// �����Ԋu
	float spawn_rate_range = 0;		// �����Ԋu�̗����͈�
	
	ParticleEmitterPropertyType position_type = PARTICAL_PROP_TYPE_PVA;
	ParticleEmitterPositionPropertyPVA position_pva;
	ParticleEmitterPositionPropertyEasing position_easing;

	ParticleEmitterPropertyType rotationType = PARTICAL_PROP_TYPE_PVA;
	ParticleEmitterRotationPropertyPVA rotation_pva;

	ParticleEmitterPropertyType scaleType = PARTICAL_PROP_TYPE_PVA;
	ParticleEmitterScalePropertyPVA scale_pva;
	ParticleEmitterScalePropertyEasing scale_easing;

	ParticleEmitterSpriteProperty sprite;

	ParticleEmitterSpawningMethod spawning_method = PARTICAL_SPAWN_METHOD_POINT;
	ParticleEmitterSpawningPoint spawning_point;
	ParticleEmitterSpawningCircle spawning_circle;

};


class ParticleEmitter : public Component
{
public:
	ParticleEmitter(ParticleEmitterProperty prop);
	~ParticleEmitter();

	bool Init() override;
	void Update() override;

	void DrawShadow() override;
	void DrawDepth() override;
	void DrawGBuffer() override;

	ParticleEmitterProperty& GetProperety();
	void SetSpawnRate(float rate);
	void Emit();

private:
	void PrepareModel();	// ���f���f�[�^�̏���
	bool PrepareCB();		// ConstantBuffer�̐���
	bool PrepareRootSignature();	// RootSignature�̐���
	bool PreparePSO();		// PipelineStateObject�̐���

	void SpawnParticles();	// �p�[�e�B�N���̐���
	
	void InitPosition(Particle& particle, const ParticleEmitterProperty& prop);	// �����ʒu�̐ݒ�
	void InitPosition(Particle& particle, const ParticleEmitterPositionPropertyPVA& prop);
	void InitPosition(Particle& particle, const ParticleEmitterPositionPropertyEasing& prop);

	void InitRotation(Particle& particle, const ParticleEmitterProperty& prop);	// �����p�x�̐ݒ�
	void InitRotation(Particle& particle, const ParticleEmitterRotationPropertyPVA& prop);

	void InitScale(Particle& particle, const ParticleEmitterProperty& prop);	// �����g�嗦�̐ݒ�
	void InitScale(Particle& particle, const ParticleEmitterScalePropertyPVA& prop);
	void InitScale(Particle& particle, const ParticleEmitterScalePropertyEasing& prop);

	float CalcStartFloat(const float& f, const float& range);
	Vec3 CalcStartVector(const Vec3& v, const Vec3& range);	// �����x�N�g�����v�Z����

	void InitSpawningMethod(Particle& particle, const ParticleEmitterProperty& prop);	// �������@�̐ݒ�
	void InitSpawningMethod(Particle& particle, const ParticleEmitterSpawningPoint& prop);
	void InitSpawningMethod(Particle& particle, const ParticleEmitterSpawningCircle& prop);

	void InitSprite(Particle& particle, const ParticleEmitterSpriteProperty& prop); // �摜�̐ݒ�

	void UpdateTimer();	// �^�C�}�[�̍X�V

	void UpdatePosition(const ParticleEmitterProperty& prop);	// �ʒu�̍X�V
	void UpdatePosition(const ParticleEmitterPositionPropertyPVA& prop);
	void UpdatePosition(const ParticleEmitterPositionPropertyEasing& prop);

	void UpdateRotation(const ParticleEmitterProperty& prop);	// �p�x�̍X�V
	void UpdateRotation(const ParticleEmitterRotationPropertyPVA& prop);

	void UpdateScale(const ParticleEmitterProperty& prop);		// �g�嗦�̍X�V
	void UpdateScale(const ParticleEmitterScalePropertyPVA& prop);
	void UpdateScale(const ParticleEmitterScalePropertyEasing& prop);

	float (*GetEasingFunc(const ParticleEmitterEasingType& type))(const float&);	// Easing�֐��ւ̃|�C���^���擾

	void UpdateCB();	// ConstantBuffer�̍X�V

private:
	std::vector<Particle> particles_;

	ParticleEmitterProperty prop_;

	std::shared_ptr<Model> particle_model_; // ���f���f�[�^
	float spawn_timer_; // �^�C�}�[
	unsigned int spawn_counter_; // ���������p�[�e�B�N���̐�
	bool is_active_ = true; // �A�N�e�B�u���ǂ���

	std::mt19937 rand_; // ����������

private:
	std::unique_ptr<ConstantBuffer> transform_cb_[Engine::FRAME_BUFFER_COUNT];	// Transform Constant Buffer
	std::unique_ptr<ConstantBuffer> scene_cb_[Engine::FRAME_BUFFER_COUNT];		// Scene Constant Buffer
	std::vector<std::unique_ptr<ConstantBuffer>> materials_cb_;				// Material Constant Buffers
	std::unique_ptr<ConstantBuffer> particle_cb_[Engine::FRAME_BUFFER_COUNT];	// Particle Constant Buffer

	std::unique_ptr<RootSignature> root_signature_;	// Root Signature

	std::unique_ptr<PipelineState> shadow_pso_;
	std::unique_ptr<PipelineState> depth_pso_;
	std::unique_ptr<PipelineState> gbuffer_pso_;

	//std::unique_ptr<DescriptorHeap> descriptor_heap_;	// SRV���i�[����f�B�X�N���v�^�q�[�v
};