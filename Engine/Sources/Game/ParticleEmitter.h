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
	Vec3 position;					// 位置
	Vec3 positionRange;				// 位置の乱数範囲
	Vec3 velocity;					// 速度
	Vec3 velocityRange;				// 速度の乱数範囲
	Vec3 acceleration;				// 加速度
	Vec3 accelerationRange;			// 加速度の乱数範囲
};

struct ParticleEmitterPositionPropertyEasing
{
	Vec3 start;						// 開始位置
	Vec3 startRange;				// 開始位置の乱数範囲
	Vec3 end;						// 終了位置
	Vec3 endRange;					// 終了位置の乱数範囲
	ParticleEmitterEasingType type = PARTICLE_EASE_LINEAR;	// 補間のタイプ
	bool middleEnabled = false;		// 中間点を使用するか
	Vec3 middle;					// 中間位置
	Vec3 middleRange;				// 中間位置の乱数範囲
};

struct ParticleEmitterRotationPropertyPVA
{
	Vec3 rotation;					// 角度
	Vec3 rotationRange;				// 角度の乱数範囲
	Vec3 velocity;					// 速度
	Vec3 velocityRange;				// 速度の乱数範囲
	Vec3 acceleration;				// 加速度
	Vec3 accelerationRange;			// 加速度の乱数範囲
};

struct ParticleEmitterScalePropertyPVA
{
	Vec3 scale = Vec3(1, 1, 1);		// 拡大率
	Vec3 scaleRange;				// 拡大率の乱数範囲
	Vec3 velocity;					// 速度
	Vec3 velocityRange;				// 速度の乱数範囲
	Vec3 acceleration;				// 加速度
	Vec3 accelerationRange;			// 加速度の乱数範囲
};

struct ParticleEmitterScalePropertyEasing
{
	Vec3 start = Vec3(1, 1, 1);		// 開始拡大率
	Vec3 startRange;				// 開始拡大率の乱数範囲
	Vec3 end = Vec3(1, 1, 1);		// 終了拡大率
	Vec3 endRange;					// 終了拡大率の乱数範囲
	ParticleEmitterEasingType type = PARTICLE_EASE_LINEAR;	// 補間のタイプ
	bool middleEnabled = false;		// 中間点を使用するか
	Vec3 middle = Vec3(1, 1, 1);	// 中間拡大率
	Vec3 middleRange;				// 中間拡大率の乱数範囲
	bool keepAspect = false;		// アスペクト比を維持するか
};

struct ParticleEmitterSpriteProperty
{
	Texture2D* albedoTexture;		// Albedo
	Texture2D* normalTexture;		// Normal
	Texture2D* pbrTexture;			// MetallicRoughness
};

struct ParticleEmitterSpawningPoint
{
	Vec3 location;					// 生成位置
	Vec3 locationRange;				// 生成位置の乱数範囲
};

struct ParticleEmitterSpawningCircle
{
	unsigned int vertices = 8;		// 頂点の数
	float radius = 1;				// 半径
	float radiusRange = 0;			// 半径の乱数範囲
};

struct ParticleEmitterProperty
{
	unsigned int spawnCount = 0;	// 生成数（0 = 無限)
	float timeToLive = 100;			// 生存時間
	float timeToLiveRange = 0;		// 生存時間の乱数範囲
	float spawnRate = 1;			// 生成間隔
	float spawnRateRange = 0;		// 生成間隔の乱数範囲
	
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
	void PrepareModel();	// モデルデータの準備
	bool PrepareCB();		// ConstantBufferの生成
	bool PrepareSRV();		// SRVの生成
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
	std::vector<Particle> m_particles;

	ParticleEmitterProperty m_prop;

	Model m_particleModel;			// モデルデータ
	float m_spawnTimer;				// タイマー
	unsigned int m_spawnCounter;	// 生成したパーティクルの数
	bool m_isActive = true;		// アクティブかどうか

	std::mt19937 m_rand;		// 乱数生成器

private:
	ConstantBuffer* m_pTransformCB[Engine::FRAME_BUFFER_COUNT];	// Transform Constant Buffer
	ConstantBuffer* m_pSceneCB[Engine::FRAME_BUFFER_COUNT];		// Scene Constant Buffer
	std::vector<ConstantBuffer*> m_pMaterialCBs;				// Material Constant Buffers
	ConstantBuffer* m_pParticleCB[Engine::FRAME_BUFFER_COUNT];	// Particle Constant Buffer

	RootSignature* m_pRootSignature;	// Root Signature

	PipelineState* m_pShadowPSO;
	PipelineState* m_pDepthPSO;
	PipelineState* m_pGBufferPSO;

	DescriptorHeap* m_pDescriptorHeap;	// SRVを格納するディスクリプタヒープ
};