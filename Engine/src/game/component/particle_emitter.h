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
	Vec3 position;					// 位置
	Vec3 position_range;				// 位置の乱数範囲
	Vec3 velocity;					// 速度
	Vec3 velocity_range;				// 速度の乱数範囲
	Vec3 acceleration;				// 加速度
	Vec3 acceleration_range;			// 加速度の乱数範囲
};

struct ParticleEmitterPositionPropertyEasing
{
	Vec3 start;						// 開始位置
	Vec3 start_range;				// 開始位置の乱数範囲
	Vec3 end;						// 終了位置
	Vec3 end_range;					// 終了位置の乱数範囲
	ParticleEmitterEasingType type = PARTICLE_EASE_LINEAR;	// 補間のタイプ
	bool middle_enabled = false;		// 中間点を使用するか
	Vec3 middle;					// 中間位置
	Vec3 middle_range;				// 中間位置の乱数範囲
};

struct ParticleEmitterRotationPropertyPVA
{
	Vec3 rotation;					// 角度
	Vec3 rotation_range;				// 角度の乱数範囲
	Vec3 velocity;					// 速度
	Vec3 velocity_range;				// 速度の乱数範囲
	Vec3 acceleration;				// 加速度
	Vec3 acceleration_range;			// 加速度の乱数範囲
};

struct ParticleEmitterScalePropertyPVA
{
	Vec3 scale = Vec3(1, 1, 1);		// 拡大率
	Vec3 scale_range;				// 拡大率の乱数範囲
	Vec3 velocity;					// 速度
	Vec3 velocity_range;				// 速度の乱数範囲
	Vec3 acceleration;				// 加速度
	Vec3 acceleration_range;			// 加速度の乱数範囲
};

struct ParticleEmitterScalePropertyEasing
{
	Vec3 start = Vec3(1, 1, 1);		// 開始拡大率
	Vec3 start_range;				// 開始拡大率の乱数範囲
	Vec3 end = Vec3(1, 1, 1);		// 終了拡大率
	Vec3 end_range;					// 終了拡大率の乱数範囲
	ParticleEmitterEasingType type = PARTICLE_EASE_LINEAR;	// 補間のタイプ
	bool middle_enabled = false;		// 中間点を使用するか
	Vec3 middle = Vec3(1, 1, 1);	// 中間拡大率
	Vec3 middle_range;				// 中間拡大率の乱数範囲
	bool keep_aspect = false;		// アスペクト比を維持するか
};

struct ParticleEmitterSpriteProperty
{
	std::shared_ptr<Texture2D> albedo_texture;		// Albedo
	std::shared_ptr<Texture2D> normal_texture;		// Normal
	std::shared_ptr<Texture2D> pbr_texture;			// MetallicRoughness
};

struct ParticleEmitterSpawningPoint
{
	Vec3 location;					// 生成位置
	Vec3 location_range;				// 生成位置の乱数範囲
};

struct ParticleEmitterSpawningCircle
{
	unsigned int vertices = 8;		// 頂点の数
	float radius = 1;				// 半径
	float radius_range = 0;			// 半径の乱数範囲
};

struct ParticleEmitterProperty
{
	unsigned int spawn_count = 0;	// 生成数（0 = 無限)
	float time_to_live = 100;			// 生存時間
	float time_to_live_range = 0;		// 生存時間の乱数範囲
	float spawn_rate = 1;			// 生成間隔
	float spawn_rate_range = 0;		// 生成間隔の乱数範囲
	
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
	void PrepareModel();	// モデルデータの準備
	bool PrepareCB();		// ConstantBufferの生成
	bool PrepareRootSignature();	// RootSignatureの生成
	bool PreparePSO();		// PipelineStateObjectの生成

	void SpawnParticles();	// パーティクルの生成
	
	void InitPosition(Particle& particle, const ParticleEmitterProperty& prop);	// 初期位置の設定
	void InitPosition(Particle& particle, const ParticleEmitterPositionPropertyPVA& prop);
	void InitPosition(Particle& particle, const ParticleEmitterPositionPropertyEasing& prop);

	void InitRotation(Particle& particle, const ParticleEmitterProperty& prop);	// 初期角度の設定
	void InitRotation(Particle& particle, const ParticleEmitterRotationPropertyPVA& prop);

	void InitScale(Particle& particle, const ParticleEmitterProperty& prop);	// 初期拡大率の設定
	void InitScale(Particle& particle, const ParticleEmitterScalePropertyPVA& prop);
	void InitScale(Particle& particle, const ParticleEmitterScalePropertyEasing& prop);

	float CalcStartFloat(const float& f, const float& range);
	Vec3 CalcStartVector(const Vec3& v, const Vec3& range);	// 初期ベクトルを計算する

	void InitSpawningMethod(Particle& particle, const ParticleEmitterProperty& prop);	// 生成方法の設定
	void InitSpawningMethod(Particle& particle, const ParticleEmitterSpawningPoint& prop);
	void InitSpawningMethod(Particle& particle, const ParticleEmitterSpawningCircle& prop);

	void InitSprite(Particle& particle, const ParticleEmitterSpriteProperty& prop); // 画像の設定

	void UpdateTimer();	// タイマーの更新

	void UpdatePosition(const ParticleEmitterProperty& prop);	// 位置の更新
	void UpdatePosition(const ParticleEmitterPositionPropertyPVA& prop);
	void UpdatePosition(const ParticleEmitterPositionPropertyEasing& prop);

	void UpdateRotation(const ParticleEmitterProperty& prop);	// 角度の更新
	void UpdateRotation(const ParticleEmitterRotationPropertyPVA& prop);

	void UpdateScale(const ParticleEmitterProperty& prop);		// 拡大率の更新
	void UpdateScale(const ParticleEmitterScalePropertyPVA& prop);
	void UpdateScale(const ParticleEmitterScalePropertyEasing& prop);

	float (*GetEasingFunc(const ParticleEmitterEasingType& type))(const float&);	// Easing関数へのポインタを取得

	void UpdateCB();	// ConstantBufferの更新

private:
	std::vector<Particle> particles_;

	ParticleEmitterProperty prop_;

	std::shared_ptr<Model> particle_model_; // モデルデータ
	float spawn_timer_; // タイマー
	unsigned int spawn_counter_; // 生成したパーティクルの数
	bool is_active_ = true; // アクティブかどうか

	std::mt19937 rand_; // 乱数生成器

private:
	std::unique_ptr<ConstantBuffer> transform_cb_[Engine::FRAME_BUFFER_COUNT];	// Transform Constant Buffer
	std::unique_ptr<ConstantBuffer> scene_cb_[Engine::FRAME_BUFFER_COUNT];		// Scene Constant Buffer
	std::vector<std::unique_ptr<ConstantBuffer>> materials_cb_;				// Material Constant Buffers
	std::unique_ptr<ConstantBuffer> particle_cb_[Engine::FRAME_BUFFER_COUNT];	// Particle Constant Buffer

	std::unique_ptr<RootSignature> root_signature_;	// Root Signature

	std::unique_ptr<PipelineState> shadow_pso_;
	std::unique_ptr<PipelineState> depth_pso_;
	std::unique_ptr<PipelineState> gbuffer_pso_;

	//std::unique_ptr<DescriptorHeap> descriptor_heap_;	// SRVを格納するディスクリプタヒープ
};