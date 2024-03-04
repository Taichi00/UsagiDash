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

	unsigned int spawn_count = 0;		// 生成数（0 = 無限)
	float time_to_live = 100;			// 生存時間
	float time_to_live_range = 0;		// 生存時間の乱数範囲
	float spawn_rate = 1;				// 生成間隔
	float spawn_rate_range = 0;			// 生成間隔の乱数範囲
	
	PropertyType position_type = PROP_TYPE_PVA;

	struct PositionPropertyPVA
	{
		Vec3 position;					// 位置
		Vec3 position_range;			// 位置の乱数範囲
		Vec3 velocity;					// 速度
		Vec3 velocity_range;			// 速度の乱数範囲
		Vec3 acceleration;				// 加速度
		Vec3 acceleration_range;		// 加速度の乱数範囲
	} position_pva;

	struct PositionPropertyEasing
	{
		Vec3 start;						// 開始位置
		Vec3 start_range;				// 開始位置の乱数範囲
		Vec3 end;						// 終了位置
		Vec3 end_range;					// 終了位置の乱数範囲
		EasingType type = EASE_LINEAR;	// 補間のタイプ
		bool middle_enabled = false;	// 中間点を使用するか
		Vec3 middle;					// 中間位置
		Vec3 middle_range;				// 中間位置の乱数範囲
	} position_easing;

	PropertyType rotation_type = PROP_TYPE_PVA;

	struct RotationPropertyPVA
	{
		Vec3 rotation;					// 角度
		Vec3 rotation_range;			// 角度の乱数範囲
		Vec3 velocity;					// 速度
		Vec3 velocity_range;			// 速度の乱数範囲
		Vec3 acceleration;				// 加速度
		Vec3 acceleration_range;		// 加速度の乱数範囲
	} rotation_pva;

	PropertyType scale_type = PROP_TYPE_PVA;

	struct ScalePropertyPVA
	{
		Vec3 scale = Vec3(1, 1, 1);		// 拡大率
		Vec3 scale_range;				// 拡大率の乱数範囲
		Vec3 velocity;					// 速度
		Vec3 velocity_range;			// 速度の乱数範囲
		Vec3 acceleration;				// 加速度
		Vec3 acceleration_range;		// 加速度の乱数範囲
	} scale_pva;

	struct ScalePropertyEasing
	{
		Vec3 start = Vec3(1, 1, 1);		// 開始拡大率
		Vec3 start_range;				// 開始拡大率の乱数範囲
		Vec3 end = Vec3(1, 1, 1);		// 終了拡大率
		Vec3 end_range;					// 終了拡大率の乱数範囲
		EasingType type = EASE_LINEAR;	// 補間のタイプ
		bool middle_enabled = false;	// 中間点を使用するか
		Vec3 middle = Vec3(1, 1, 1);	// 中間拡大率
		Vec3 middle_range;				// 中間拡大率の乱数範囲
		bool keep_aspect = false;		// アスペクト比を維持するか
	} scale_easing;

	PropertyType color_type = PROP_TYPE_PVA;
	ColorMode color_mode = COLOR_MODE_RGBA;
	
	struct ColorPropertyPVA
	{
		Color color = Color(1, 1, 1, 1);// 色
		Color color_range;				// 色の乱数範囲
		Color velocity;					// 速度
		Color velocity_range;			// 速度の乱数範囲
		Color acceleration;				// 加速度
		Color acceleration_range;		// 加速度の乱数範囲
	} color_pva;

	struct ColorPropertyEasing
	{
		Color start = Color(1, 1, 1, 1);// 開始色
		Color start_range;				// 開始色の乱数範囲
		Color end = Color(1, 1, 1, 1);	// 終了色
		Color end_range;				// 終了色の乱数範囲
		EasingType type = EASE_LINEAR;	// 補間のタイプ
		bool middle_enabled = false;	// 中間点を使用するか
		Color middle = Color(1, 1, 1, 1);// 中間色
		Color middle_range;				// 中間色の乱数範囲
		bool keep_aspect = false;		// アスペクト比を維持するか
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
		bool is_two_side = true; // 裏面を描画するかどうか
	} mesh;

	SpawningMethod spawning_method = SPAWN_METHOD_POINT;

	struct SpawningPoint
	{
		Vec3 location;					// 生成位置
		Vec3 location_range;			// 生成位置の乱数範囲
	} spawning_point;

	struct SpawningCircle
	{
		unsigned int vertices = 8;		// 頂点の数
		float radius = 1;				// 半径
		float radius_range = 0;			// 半径の乱数範囲
	} spawning_circle;

};

// スコープ指定を省略する
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
	void SpawnParticles(const float delta_time);	// パーティクルの生成
	
	void InitPosition(Particle& particle, const ParticleEmitterProperty& prop);	// 初期位置の設定
	void InitPosition(Particle& particle, const ParticleEmitterProperty::PositionPropertyPVA& prop);
	void InitPosition(Particle& particle, const ParticleEmitterProperty::PositionPropertyEasing& prop);

	void InitRotation(Particle& particle, const ParticleEmitterProperty& prop);	// 初期角度の設定
	void InitRotation(Particle& particle, const ParticleEmitterProperty::RotationPropertyPVA& prop);

	void InitScale(Particle& particle, const ParticleEmitterProperty& prop);	// 初期拡大率の設定
	void InitScale(Particle& particle, const ParticleEmitterProperty::ScalePropertyPVA& prop);
	void InitScale(Particle& particle, const ParticleEmitterProperty::ScalePropertyEasing& prop);

	void InitColor(Particle& particle, const ParticleEmitterProperty& prop);	// 初期色の設定
	void InitColor(Particle& particle, const ParticleEmitterProperty::ColorPropertyPVA& prop);
	void InitColor(Particle& particle, const ParticleEmitterProperty::ColorPropertyEasing& prop);

	float CalcStartFloat(const float& f, const float& range);
	Vec3 CalcStartVector(const Vec3& v, const Vec3& range);	// 初期ベクトルを計算する
	Color CalcStartColor(const Color& c, const Color& range);

	void InitSpawningMethod(Particle& particle, const ParticleEmitterProperty& prop);	// 生成方法の設定
	void InitSpawningMethod(Particle& particle, const ParticleEmitterProperty::SpawningPoint& prop);
	void InitSpawningMethod(Particle& particle, const ParticleEmitterProperty::SpawningCircle& prop);

	void InitTexture(Particle& particle, const ParticleEmitterProperty& prop); // 画像の設定
	void InitSpriteTexture(Particle& particle, const ParticleEmitterProperty::SpriteProperty& prop);
	void InitMeshTexture(Particle& particle, const ParticleEmitterProperty::MeshProperty& prop);

	void UpdateTimer(const float delta_time);	// タイマーの更新

	void UpdatePosition(const ParticleEmitterProperty& prop, const float delta_time);	// 位置の更新
	void UpdatePosition(const ParticleEmitterProperty::PositionPropertyPVA& prop, const float delta_time);
	void UpdatePosition(const ParticleEmitterProperty::PositionPropertyEasing& prop, const float delta_time);

	void UpdateRotation(const ParticleEmitterProperty& prop, const float delta_time);	// 角度の更新
	void UpdateRotation(const ParticleEmitterProperty::RotationPropertyPVA& prop, const float delta_time);

	void UpdateScale(const ParticleEmitterProperty& prop, const float delta_time);		// 拡大率の更新
	void UpdateScale(const ParticleEmitterProperty::ScalePropertyPVA& prop, const float delta_time);
	void UpdateScale(const ParticleEmitterProperty::ScalePropertyEasing& prop, const float delta_time);

	void UpdateColor(const ParticleEmitterProperty& prop, const float delta_time);		// 色の更新
	void UpdateColor(const ParticleEmitterProperty::ColorPropertyPVA& prop, const float delta_time);
	void UpdateColor(const ParticleEmitterProperty::ColorPropertyEasing& prop, const float delta_time);

	float (*GetEasingFunc(const ParticleEmitterProperty::EasingType& type))(const float);	// Easing関数へのポインタを取得

private:
	std::vector<Particle> particles_;

	ParticleEmitterProperty prop_;

	float spawn_timer_; // タイマー
	unsigned int spawn_counter_; // 生成したパーティクルの数
	bool is_active_; // アクティブかどうか

	std::unique_ptr<ParticleRenderer> renderer_;

	std::mt19937 rand_; // 乱数生成器
};