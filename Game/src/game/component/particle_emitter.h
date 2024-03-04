#pragma once

#include "game/component/component.h"
#include "engine/descriptor_heap.h"
#include "engine/engine.h"
#include "game/resource/model.h"
#include "game/particle.h"
#include "game/scene.h"
#include "game/resource/texture2d.h"
#include "math/vec.h"
#include "math/color.h"
#include <DirectXMath.h>
#include <memory>
#include <random>
#include <vector>

const int MAX_PARTICLE_COUNT = 300;
const float PARTICLE_SPRITE_SIZE_PER_PIXEL = 0.01f;

class ConstantBuffer;
class PipelineState;
class RootSignature;
class ParticleRenderer;

struct ParticleData
{
	XMMATRIX world;
	XMMATRIX light_world;
	XMVECTOR color;
};

struct alignas(256) ParticleParameter
{
	ParticleData data[MAX_PARTICLE_COUNT];
};


struct ParticleEmitterProperty
{
	enum ParticleType
	{
		PARTICLE_TYPE_SPRITE = 0,
		PARTICLE_TYPE_MESH
	};

	enum PropertyType
	{
		PROP_TYPE_PVA = 0,
		PROP_TYPE_EASING,
	};

	enum SpawningMethod
	{
		SPAWN_METHOD_POINT = 0,
		SPAWN_METHOD_CIRCLE,
	};

	enum EasingType
	{
		EASE_LINEAR = 0,
		EASE_IN_CUBIC,
		EASE_OUT_CUBIC,
		EASE_INOUT_CUBIC,
	};

	enum ColorMode
	{
		COLOR_MODE_RGBA = 0,
		COLOR_MODE_HSVA,
	};

	unsigned int spawn_count = 0;		// �������i0 = ����)
	float time_to_live = 100;			// ��������
	float time_to_live_range = 0;		// �������Ԃ̗����͈�
	float spawn_rate = 1;				// �����Ԋu
	float spawn_rate_range = 0;			// �����Ԋu�̗����͈�
	
	PropertyType position_type = PROP_TYPE_PVA;

	struct PositionPropertyPVA
	{
		Vec3 position;					// �ʒu
		Vec3 position_range;			// �ʒu�̗����͈�
		Vec3 velocity;					// ���x
		Vec3 velocity_range;			// ���x�̗����͈�
		Vec3 acceleration;				// �����x
		Vec3 acceleration_range;		// �����x�̗����͈�
	} position_pva;

	struct PositionPropertyEasing
	{
		Vec3 start;						// �J�n�ʒu
		Vec3 start_range;				// �J�n�ʒu�̗����͈�
		Vec3 end;						// �I���ʒu
		Vec3 end_range;					// �I���ʒu�̗����͈�
		EasingType type = EASE_LINEAR;	// ��Ԃ̃^�C�v
		bool middle_enabled = false;	// ���ԓ_���g�p���邩
		Vec3 middle;					// ���Ԉʒu
		Vec3 middle_range;				// ���Ԉʒu�̗����͈�
	} position_easing;

	PropertyType rotation_type = PROP_TYPE_PVA;

	struct RotationPropertyPVA
	{
		Vec3 rotation;					// �p�x
		Vec3 rotation_range;			// �p�x�̗����͈�
		Vec3 velocity;					// ���x
		Vec3 velocity_range;			// ���x�̗����͈�
		Vec3 acceleration;				// �����x
		Vec3 acceleration_range;		// �����x�̗����͈�
	} rotation_pva;

	PropertyType scale_type = PROP_TYPE_PVA;

	struct ScalePropertyPVA
	{
		Vec3 scale = Vec3(1, 1, 1);		// �g�嗦
		Vec3 scale_range;				// �g�嗦�̗����͈�
		Vec3 velocity;					// ���x
		Vec3 velocity_range;			// ���x�̗����͈�
		Vec3 acceleration;				// �����x
		Vec3 acceleration_range;		// �����x�̗����͈�
	} scale_pva;

	struct ScalePropertyEasing
	{
		Vec3 start = Vec3(1, 1, 1);		// �J�n�g�嗦
		Vec3 start_range;				// �J�n�g�嗦�̗����͈�
		Vec3 end = Vec3(1, 1, 1);		// �I���g�嗦
		Vec3 end_range;					// �I���g�嗦�̗����͈�
		EasingType type = EASE_LINEAR;	// ��Ԃ̃^�C�v
		bool middle_enabled = false;	// ���ԓ_���g�p���邩
		Vec3 middle = Vec3(1, 1, 1);	// ���Ԋg�嗦
		Vec3 middle_range;				// ���Ԋg�嗦�̗����͈�
		bool keep_aspect = false;		// �A�X�y�N�g����ێ����邩
	} scale_easing;

	PropertyType color_type = PROP_TYPE_PVA;
	ColorMode color_mode = COLOR_MODE_RGBA;
	
	struct ColorPropertyPVA
	{
		Color color = Color(1, 1, 1, 1);// �F
		Color color_range;				// �F�̗����͈�
		Color velocity;					// ���x
		Color velocity_range;			// ���x�̗����͈�
		Color acceleration;				// �����x
		Color acceleration_range;		// �����x�̗����͈�
	} color_pva;

	struct ColorPropertyEasing
	{
		Color start = Color(1, 1, 1, 1);// �J�n�F
		Color start_range;				// �J�n�F�̗����͈�
		Color end = Color(1, 1, 1, 1);	// �I���F
		Color end_range;				// �I���F�̗����͈�
		EasingType type = EASE_LINEAR;	// ��Ԃ̃^�C�v
		bool middle_enabled = false;	// ���ԓ_���g�p���邩
		Color middle = Color(1, 1, 1, 1);// ���ԐF
		Color middle_range;				// ���ԐF�̗����͈�
		bool keep_aspect = false;		// �A�X�y�N�g����ێ����邩
	} color_easing;

	ParticleType particle_type = PARTICLE_TYPE_SPRITE;

	struct SpriteProperty
	{
		std::shared_ptr<Texture2D> albedo_texture;		// Albedo
		std::shared_ptr<Texture2D> normal_texture;		// Normal
		std::shared_ptr<Texture2D> pbr_texture;			// MetallicRoughness
	} sprite;

	struct MeshProperty
	{
		std::shared_ptr<Texture2D> albedo_texture;		// Albedo
		std::shared_ptr<Texture2D> normal_texture;		// Normal
		std::shared_ptr<Texture2D> pbr_texture;			// MetallicRoughness
		bool is_two_side = true; // ���ʂ�`�悷�邩�ǂ���
	} mesh;

	SpawningMethod spawning_method = SPAWN_METHOD_POINT;

	struct SpawningPoint
	{
		Vec3 location;					// �����ʒu
		Vec3 location_range;			// �����ʒu�̗����͈�
	} spawning_point;

	struct SpawningCircle
	{
		unsigned int vertices = 8;		// ���_�̐�
		float radius = 1;				// ���a
		float radius_range = 0;			// ���a�̗����͈�
	} spawning_circle;

};

// �X�R�[�v�w����ȗ�����
using enum ParticleEmitterProperty::ParticleType;
using enum ParticleEmitterProperty::PropertyType;
using enum ParticleEmitterProperty::SpawningMethod;
using enum ParticleEmitterProperty::EasingType;
using enum ParticleEmitterProperty::ColorMode;

class ParticleEmitter : public Component
{
public:
	ParticleEmitter(const ParticleEmitterProperty& prop);
	~ParticleEmitter();

	bool Init() override;
	void Update(const float delta_time) override;

	void DrawShadow() override;
	void DrawDepth() override;
	void DrawGBuffer() override;

	ParticleEmitterProperty& GetProperety();
	void SetSpawnRate(float rate);
	void Emit();
	void Stop();

	const std::vector<Particle>& Particles() const { return particles_; }

private:
	void SpawnParticles(const float delta_time);	// �p�[�e�B�N���̐���
	
	void InitPosition(Particle& particle, const ParticleEmitterProperty& prop);	// �����ʒu�̐ݒ�
	void InitPosition(Particle& particle, const ParticleEmitterProperty::PositionPropertyPVA& prop);
	void InitPosition(Particle& particle, const ParticleEmitterProperty::PositionPropertyEasing& prop);

	void InitRotation(Particle& particle, const ParticleEmitterProperty& prop);	// �����p�x�̐ݒ�
	void InitRotation(Particle& particle, const ParticleEmitterProperty::RotationPropertyPVA& prop);

	void InitScale(Particle& particle, const ParticleEmitterProperty& prop);	// �����g�嗦�̐ݒ�
	void InitScale(Particle& particle, const ParticleEmitterProperty::ScalePropertyPVA& prop);
	void InitScale(Particle& particle, const ParticleEmitterProperty::ScalePropertyEasing& prop);

	void InitColor(Particle& particle, const ParticleEmitterProperty& prop);	// �����F�̐ݒ�
	void InitColor(Particle& particle, const ParticleEmitterProperty::ColorPropertyPVA& prop);
	void InitColor(Particle& particle, const ParticleEmitterProperty::ColorPropertyEasing& prop);

	float CalcStartFloat(const float& f, const float& range);
	Vec3 CalcStartVector(const Vec3& v, const Vec3& range);	// �����x�N�g�����v�Z����
	Color CalcStartColor(const Color& c, const Color& range);

	void InitSpawningMethod(Particle& particle, const ParticleEmitterProperty& prop);	// �������@�̐ݒ�
	void InitSpawningMethod(Particle& particle, const ParticleEmitterProperty::SpawningPoint& prop);
	void InitSpawningMethod(Particle& particle, const ParticleEmitterProperty::SpawningCircle& prop);

	void InitTexture(Particle& particle, const ParticleEmitterProperty& prop); // �摜�̐ݒ�
	void InitSpriteTexture(Particle& particle, const ParticleEmitterProperty::SpriteProperty& prop);
	void InitMeshTexture(Particle& particle, const ParticleEmitterProperty::MeshProperty& prop);

	void UpdateTimer(const float delta_time);	// �^�C�}�[�̍X�V

	void UpdatePosition(const ParticleEmitterProperty& prop, const float delta_time);	// �ʒu�̍X�V
	void UpdatePosition(const ParticleEmitterProperty::PositionPropertyPVA& prop, const float delta_time);
	void UpdatePosition(const ParticleEmitterProperty::PositionPropertyEasing& prop, const float delta_time);

	void UpdateRotation(const ParticleEmitterProperty& prop, const float delta_time);	// �p�x�̍X�V
	void UpdateRotation(const ParticleEmitterProperty::RotationPropertyPVA& prop, const float delta_time);

	void UpdateScale(const ParticleEmitterProperty& prop, const float delta_time);		// �g�嗦�̍X�V
	void UpdateScale(const ParticleEmitterProperty::ScalePropertyPVA& prop, const float delta_time);
	void UpdateScale(const ParticleEmitterProperty::ScalePropertyEasing& prop, const float delta_time);

	void UpdateColor(const ParticleEmitterProperty& prop, const float delta_time);		// �F�̍X�V
	void UpdateColor(const ParticleEmitterProperty::ColorPropertyPVA& prop, const float delta_time);
	void UpdateColor(const ParticleEmitterProperty::ColorPropertyEasing& prop, const float delta_time);

	float (*GetEasingFunc(const ParticleEmitterProperty::EasingType& type))(const float);	// Easing�֐��ւ̃|�C���^���擾

private:
	std::vector<Particle> particles_;

	ParticleEmitterProperty prop_;

	float spawn_timer_; // �^�C�}�[
	unsigned int spawn_counter_; // ���������p�[�e�B�N���̐�
	bool is_active_; // �A�N�e�B�u���ǂ���

	std::unique_ptr<ParticleRenderer> renderer_;

	std::mt19937 rand_; // ����������
};