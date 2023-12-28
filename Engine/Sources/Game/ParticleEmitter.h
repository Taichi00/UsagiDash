#pragma once

#include <vector>
#include <string>
#include <random>
#include "Component.h"
#include "Vec.h"
#include "Particle.h"
#include "SharedStruct.h"
#include "Engine.h"


const int MAX_PARTICLE_COUNT = 500;
const float PARTICLE_SPRITE_SIZE_PER_PIXEL = 0.01;


class Texture2D;
class ConstantBuffer;
class PipelineState;
class DescriptorHeap;


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
	Vec3 positionRange;				// �ʒu�̗����͈�
	Vec3 velocity;					// ���x
	Vec3 velocityRange;				// ���x�̗����͈�
	Vec3 acceleration;				// �����x
	Vec3 accelerationRange;			// �����x�̗����͈�
};

struct ParticleEmitterPositionPropertyEasing
{
	Vec3 start;						// �J�n�ʒu
	Vec3 startRange;				// �J�n�ʒu�̗����͈�
	Vec3 end;						// �I���ʒu
	Vec3 endRange;					// �I���ʒu�̗����͈�
	ParticleEmitterEasingType type = PARTICLE_EASE_LINEAR;	// ��Ԃ̃^�C�v
	bool middleEnabled = false;		// ���ԓ_���g�p���邩
	Vec3 middle;					// ���Ԉʒu
	Vec3 middleRange;				// ���Ԉʒu�̗����͈�
};

struct ParticleEmitterRotationPropertyPVA
{
	Vec3 rotation;					// �p�x
	Vec3 rotationRange;				// �p�x�̗����͈�
	Vec3 velocity;					// ���x
	Vec3 velocityRange;				// ���x�̗����͈�
	Vec3 acceleration;				// �����x
	Vec3 accelerationRange;			// �����x�̗����͈�
};

struct ParticleEmitterScalePropertyPVA
{
	Vec3 scale = Vec3(1, 1, 1);		// �g�嗦
	Vec3 scaleRange;				// �g�嗦�̗����͈�
	Vec3 velocity;					// ���x
	Vec3 velocityRange;				// ���x�̗����͈�
	Vec3 acceleration;				// �����x
	Vec3 accelerationRange;			// �����x�̗����͈�
};

struct ParticleEmitterScalePropertyEasing
{
	Vec3 start = Vec3(1, 1, 1);		// �J�n�g�嗦
	Vec3 startRange;				// �J�n�g�嗦�̗����͈�
	Vec3 end = Vec3(1, 1, 1);		// �I���g�嗦
	Vec3 endRange;					// �I���g�嗦�̗����͈�
	ParticleEmitterEasingType type = PARTICLE_EASE_LINEAR;	// ��Ԃ̃^�C�v
	bool middleEnabled = false;		// ���ԓ_���g�p���邩
	Vec3 middle = Vec3(1, 1, 1);	// ���Ԋg�嗦
	Vec3 middleRange;				// ���Ԋg�嗦�̗����͈�
	bool keepAspect = false;		// �A�X�y�N�g����ێ����邩
};

struct ParticleEmitterSpriteProperty
{
	Texture2D* albedoTexture;		// Albedo
	Texture2D* normalTexture;		// Normal
	Texture2D* pbrTexture;			// MetallicRoughness
};

struct ParticleEmitterSpawningPoint
{
	Vec3 location;					// �����ʒu
	Vec3 locationRange;				// �����ʒu�̗����͈�
};

struct ParticleEmitterSpawningCircle
{
	unsigned int vertices = 8;		// ���_�̐�
	float radius = 1;				// ���a
	float radiusRange = 0;			// ���a�̗����͈�
};

struct ParticleEmitterProperty
{
	unsigned int spawnCount = 0;	// �������i0 = ����)
	float timeToLive = 100;			// ��������
	float timeToLiveRange = 0;		// �������Ԃ̗����͈�
	float spawnRate = 1;			// �����Ԋu
	float spawnRateRange = 0;		// �����Ԋu�̗����͈�
	
	ParticleEmitterPropertyType positionType = PARTICAL_PROP_TYPE_PVA;
	ParticleEmitterPositionPropertyPVA positionPVA;
	ParticleEmitterPositionPropertyEasing positionEasing;

	ParticleEmitterPropertyType rotationType = PARTICAL_PROP_TYPE_PVA;
	ParticleEmitterRotationPropertyPVA rotationPVA;

	ParticleEmitterPropertyType scaleType = PARTICAL_PROP_TYPE_PVA;
	ParticleEmitterScalePropertyPVA scalePVA;
	ParticleEmitterScalePropertyEasing scaleEasing;

	ParticleEmitterSpriteProperty sprite;

	ParticleEmitterSpawningMethod spawningMethod = PARTICAL_SPAWN_METHOD_POINT;
	ParticleEmitterSpawningPoint spawningPoint;
	ParticleEmitterSpawningCircle spawningCircle;

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
	bool PrepareSRV();		// SRV�̐���
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
	std::vector<Particle> m_particles;

	ParticleEmitterProperty m_prop;

	Model m_particleModel;			// ���f���f�[�^
	float m_spawnTimer;				// �^�C�}�[
	unsigned int m_spawnCounter;	// ���������p�[�e�B�N���̐�
	bool m_isActive = true;		// �A�N�e�B�u���ǂ���

	std::mt19937 m_rand;		// ����������

private:
	ConstantBuffer* m_pTransformCB[Engine::FRAME_BUFFER_COUNT];	// Transform Constant Buffer
	ConstantBuffer* m_pSceneCB[Engine::FRAME_BUFFER_COUNT];		// Scene Constant Buffer
	std::vector<ConstantBuffer*> m_pMaterialCBs;				// Material Constant Buffers
	ConstantBuffer* m_pParticleCB[Engine::FRAME_BUFFER_COUNT];	// Particle Constant Buffer

	RootSignature* m_pRootSignature;	// Root Signature

	PipelineState* m_pShadowPSO;
	PipelineState* m_pDepthPSO;
	PipelineState* m_pGBufferPSO;

	DescriptorHeap* m_pDescriptorHeap;	// SRV���i�[����f�B�X�N���v�^�q�[�v
};