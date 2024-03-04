#include "game/component/particle_emitter.h"
#include "game/particle.h"
#include "game/resource/texture2d.h"
#include "engine/vertex_buffer.h"
#include "engine/index_buffer.h"
#include "engine/constant_buffer.h"
#include "engine/root_signature.h"
#include "engine/pipeline_state.h"
#include "game/entity.h"
#include "game/scene.h"
#include "game/component/camera.h"
#include "engine/descriptor_heap.h"
#include "math/quaternion.h"
#include "math/easing.h"
#include "game/resource/model.h"
#include "game/particle_renderer.h"
#include "game/particle_renderer_sprite.h"
#include "game/particle_renderer_mesh.h"
#include <memory>

ParticleEmitter::ParticleEmitter(const ParticleEmitterProperty& prop)
{
	prop_ = prop;

	particles_.clear();
	spawn_timer_ = 0;
	spawn_counter_ = 0;
	is_active_ = false;

	// 乱数生成
	std::random_device rd;
	rand_ = std::mt19937(rd());
}

ParticleEmitter::~ParticleEmitter()
{
}

bool ParticleEmitter::Init()
{
	// レンダラーの生成
	switch (prop_.particle_type)
	{
	case PARTICLE_TYPE_SPRITE:
		renderer_ = std::make_unique<ParticleRendererSprite>(this);
		break;
	case PARTICLE_TYPE_MESH:
		renderer_ = std::make_unique<ParticleRendererMesh>(this);
		break;
	}

	// レンダラーの初期化
	renderer_->Init();

	return true;
}

void ParticleEmitter::Update(const float delta_time)
{
	SpawnParticles(delta_time);
	UpdateTimer(delta_time);
	UpdatePosition(prop_, delta_time);
	UpdateRotation(prop_, delta_time);
	UpdateScale(prop_, delta_time);
	UpdateColor(prop_, delta_time);
	
	renderer_->Update();
}

void ParticleEmitter::DrawShadow()
{
	renderer_->DrawShadow();
}

void ParticleEmitter::DrawDepth()
{
	renderer_->DrawDepth();
}

void ParticleEmitter::DrawGBuffer()
{
	renderer_->DrawGBuffer();
}

void ParticleEmitter::SetSpawnRate(float rate)
{
	prop_.spawn_rate = rate;
}

void ParticleEmitter::Emit()
{
	spawn_counter_ = 0;
	is_active_ = true;
}

void ParticleEmitter::Stop()
{
	is_active_ = false;
}

void ParticleEmitter::SpawnParticles(const float delta_time)
{
	if (!is_active_)
		return;

	// spawnCountを超えていたらやめる
	if (prop_.spawn_count > 0 && spawn_counter_ > prop_.spawn_count)
		return;

	if (particles_.size() > MAX_PARTICLE_COUNT)
		return;

	if (prop_.spawn_rate == 0)
		return;

	std::uniform_real_distribution<float> frand(-1, 1);

	// タイマーの更新
	spawn_timer_ += 60 * delta_time;
	
	// 1個発生させるのにかかる時間（frame / 個)
	auto spawnInterval = 1.0f / (prop_.spawn_rate + frand(rand_) * prop_.spawn_rate_range);

	while (spawn_timer_ >= spawnInterval)
	{
		if (particles_.size() > MAX_PARTICLE_COUNT)
			break;

		spawn_timer_ -= spawnInterval;

		auto timeToLive = prop_.time_to_live;

		// パーティクルの生成
		Particle particle;
		particle.time_to_live = timeToLive + frand(rand_) * prop_.time_to_live_range;
		particle.time = 0;
		InitSpawningMethod(particle, prop_);
		InitColor(particle, prop_);
		InitScale(particle, prop_);
		InitRotation(particle, prop_);
		InitPosition(particle, prop_);
		InitTexture(particle, prop_);

		particles_.push_back(particle);

		spawn_counter_ += 1;

		// spawnCountを超えていたらやめる
		if (prop_.spawn_count > 0 && spawn_counter_ > prop_.spawn_count)
		{
			is_active_ = false;
			break;
		}
	}
}

void ParticleEmitter::InitPosition(Particle& particle, const ParticleEmitterProperty& prop)
{
	switch (prop.position_type)
	{
	case PROP_TYPE_PVA:
		InitPosition(particle, prop.position_pva);
		break;
	case PROP_TYPE_EASING:
		InitPosition(particle, prop.position_easing);
		break;
	}
}

void ParticleEmitter::InitPosition(Particle& particle, const ParticleEmitterProperty::PositionPropertyPVA& prop)
{
	auto position = CalcStartVector(prop.position, prop.position_range);
	auto velocity = CalcStartVector(prop.velocity, prop.velocity_range);
	auto acceleration = CalcStartVector(prop.acceleration, prop.acceleration_range);
	auto pos = particle.position;

	auto world = transform->WorldMatrix();
	auto rot = transform->WorldRotation().Conjugate();
	auto p = (XMMatrixTranslationFromVector(position + pos) * world).r[3].m128_f32;

	auto q = particle.local_direction;

	particle.position = Vec3(p[0], p[1], p[2]);
	particle.position_velocity = rot * q * velocity;
	particle.position_acceleration = rot * q * acceleration;
}

void ParticleEmitter::InitPosition(Particle& particle, const ParticleEmitterProperty::PositionPropertyEasing& prop)
{
	auto start = CalcStartVector(prop.start, prop.start_range);
	auto end = CalcStartVector(prop.end, prop.end_range);
	auto middle = CalcStartVector(prop.middle, prop.middle_range);
	auto pos = particle.position;

	auto world = transform->WorldMatrix();
	auto q = particle.local_direction;

	auto s = (XMMatrixTranslationFromVector(q * start + pos) * world).r[3].m128_f32;
	auto e = (XMMatrixTranslationFromVector(q * end + pos) * world).r[3].m128_f32;
	auto m = (XMMatrixTranslationFromVector(q * middle + pos) * world).r[3].m128_f32;
	
	particle.position_start = Vec3(s[0], s[1], s[2]);
	particle.position_end = Vec3(e[0], e[1], e[2]);
	particle.position_middle = Vec3(m[0], m[1], m[2]);
}

void ParticleEmitter::InitRotation(Particle& particle, const ParticleEmitterProperty& prop)
{
	switch (prop.rotation_type)
	{
	case PROP_TYPE_PVA:
		InitRotation(particle, prop.rotation_pva);
		break;
	}
}

void ParticleEmitter::InitRotation(Particle& particle, const ParticleEmitterProperty::RotationPropertyPVA& prop)
{
	particle.rotation = CalcStartVector(prop.rotation, prop.rotation_range);
	particle.rotation_velocity = CalcStartVector(prop.velocity, prop.velocity_range);
	particle.rotation_acceleration = CalcStartVector(prop.acceleration, prop.acceleration_range);
}

void ParticleEmitter::InitScale(Particle& particle, const ParticleEmitterProperty& prop)
{
	switch (prop.scale_type)
	{
	case PROP_TYPE_PVA:
		InitScale(particle, prop.scale_pva);
		break;
	case PROP_TYPE_EASING:
		InitScale(particle, prop.scale_easing);
		break;
	}
}

void ParticleEmitter::InitScale(Particle& particle, const ParticleEmitterProperty::ScalePropertyPVA& prop)
{
	particle.scale = CalcStartVector(prop.scale, prop.scale_range);
	particle.scale_velocity = CalcStartVector(prop.velocity, prop.velocity_range);
	particle.scale_acceleration = CalcStartVector(prop.acceleration, prop.acceleration_range);
}

void ParticleEmitter::InitScale(Particle& particle, const ParticleEmitterProperty::ScalePropertyEasing& prop)
{
	auto scaleStart = CalcStartVector(prop.start, prop.start_range);
	auto scaleEnd = CalcStartVector(prop.end, prop.end_range);
	auto scaleMiddle = CalcStartVector(prop.middle, prop.middle_range);

	if (prop.keep_aspect)
	{
		scaleStart = Vec3(1, 1, 1) * scaleStart.x;
		scaleEnd = Vec3(1, 1, 1) * scaleEnd.x;
		scaleMiddle = Vec3(1, 1, 1) * scaleMiddle.x;
	}

	particle.scale_start = scaleStart;
	particle.scale_end = scaleEnd;
	particle.scale_middle = scaleMiddle;
}

void ParticleEmitter::InitColor(Particle& particle, const ParticleEmitterProperty& prop)
{
	switch (prop.color_type)
	{
	case PROP_TYPE_PVA:
		InitColor(particle, prop.color_pva);
		break;
	case PROP_TYPE_EASING:
		InitColor(particle, prop.color_easing);
		break;
	}
}

void ParticleEmitter::InitColor(Particle& particle, const ParticleEmitterProperty::ColorPropertyPVA& prop)
{
	particle.color = CalcStartColor(prop.color, prop.color_range);
	particle.color_velocity = CalcStartColor(prop.velocity, prop.velocity_range);
	particle.color_acceleration = CalcStartColor(prop.acceleration, prop.acceleration_range);
}

void ParticleEmitter::InitColor(Particle& particle, const ParticleEmitterProperty::ColorPropertyEasing& prop)
{
	particle.color_start = CalcStartColor(prop.start, prop.start_range);
	particle.color_end = CalcStartColor(prop.end, prop.end_range);
	particle.color_middle = CalcStartColor(prop.middle, prop.middle_range);
}

Vec3 ParticleEmitter::CalcStartVector(const Vec3& v, const Vec3& range)
{
	std::uniform_real_distribution<float> frand(-1, 1);

	auto res = Vec3();
	res.x = v.x + range.x * frand(rand_);
	res.y = v.y + range.y * frand(rand_);
	res.z = v.z + range.z * frand(rand_);

	return res;
}

Color ParticleEmitter::CalcStartColor(const Color& c, const Color& range)
{
	std::uniform_real_distribution<float> frand(-1, 1);

	auto res = Color();
	res.r = c.r + range.r * frand(rand_);
	res.g = c.g + range.g * frand(rand_);
	res.b = c.b + range.b * frand(rand_);
	res.a = c.a + range.a * frand(rand_);

	return res;
}

float ParticleEmitter::CalcStartFloat(const float& f, const float& range)
{
	std::uniform_real_distribution<float> frand(-1, 1);

	return f + range * frand(rand_);
}

void ParticleEmitter::InitSpawningMethod(Particle& particle, const ParticleEmitterProperty& prop)
{
	switch (prop.spawning_method)
	{
	case SPAWN_METHOD_POINT:
		InitSpawningMethod(particle, prop.spawning_point);
		break;
	case SPAWN_METHOD_CIRCLE:
		InitSpawningMethod(particle, prop.spawning_circle);
		break;
	}
}

void ParticleEmitter::InitSpawningMethod(Particle& particle, const ParticleEmitterProperty::SpawningPoint& prop)
{
	auto pos = CalcStartVector(prop.location, prop.location_range);

	particle.position = pos;
	particle.position_middle = pos;
}

void ParticleEmitter::InitSpawningMethod(Particle& particle, const ParticleEmitterProperty::SpawningCircle& prop)
{
	float n = (float)(spawn_counter_ % prop.vertices);
	float r = CalcStartFloat(prop.radius, prop.radius_range);
	float rad = 2 * (float)PI / prop.vertices * n;

	Vec3 pos = Vec3::Zero();
	pos.x = cos(rad) * r;
	pos.y = 0;
	pos.z = sin(rad) * r;

	auto q = Quaternion::FromToRotation(Vec3(0, 0, 1), pos.Normalized());

	particle.position = pos;
	particle.local_direction = q;
}

void ParticleEmitter::InitTexture(Particle& particle, const ParticleEmitterProperty& prop)
{
	switch (prop.particle_type)
	{
	case PARTICLE_TYPE_SPRITE:
		InitSpriteTexture(particle, prop.sprite);
		break;
	case PARTICLE_TYPE_MESH:
		InitMeshTexture(particle, prop.mesh);
		break;
	}
}

void ParticleEmitter::InitSpriteTexture(Particle& particle, const ParticleEmitterProperty::SpriteProperty& prop)
{
	auto meta = prop.albedo_texture->Metadata();

	particle.width = meta.width * PARTICLE_SPRITE_SIZE_PER_PIXEL * 0.5f;
	particle.height = meta.height * PARTICLE_SPRITE_SIZE_PER_PIXEL * 0.5f;
}

void ParticleEmitter::InitMeshTexture(Particle& particle, const ParticleEmitterProperty::MeshProperty& prop)
{
	auto meta = prop.albedo_texture->Metadata();

	particle.width = meta.width * PARTICLE_SPRITE_SIZE_PER_PIXEL * 0.5f;
	particle.height = meta.height * PARTICLE_SPRITE_SIZE_PER_PIXEL * 0.5f;
}

ParticleEmitterProperty& ParticleEmitter::GetProperety()
{
	return prop_;
}

void ParticleEmitter::UpdateTimer(const float delta_time)
{
	for (auto& particle : particles_)
	{
		particle.time += 60 * delta_time;
	}

	// timeToLiveを過ぎたら消去
	particles_.erase(
		std::remove_if(particles_.begin(), particles_.end(),[](Particle const& p) {
			return p.time > p.time_to_live;
		}), particles_.end());
}

void ParticleEmitter::UpdatePosition(const ParticleEmitterProperty& prop, const float delta_time)
{
	switch (prop.position_type)
	{
	case PROP_TYPE_PVA:
		UpdatePosition(prop.position_pva, delta_time);
		break;
	case PROP_TYPE_EASING:
		UpdatePosition(prop.position_easing, delta_time);
		break;
	}
}

void ParticleEmitter::UpdatePosition(const ParticleEmitterProperty::PositionPropertyPVA& prop, const float delta_time)
{
	for (auto& particle : particles_)
	{
		particle.position_velocity += particle.position_acceleration;
		particle.position += particle.position_velocity * delta_time;
	}
}

void ParticleEmitter::UpdatePosition(const ParticleEmitterProperty::PositionPropertyEasing& prop, const float delta_time)
{
	float (*easing)(const float) = GetEasingFunc(prop.type);

	if (prop.middle_enabled)
	{
		for (auto& particle : particles_)
		{
			auto t = particle.time / particle.time_to_live;
			auto t1 = std::clamp(t * 2, 0.0f, 1.0f);
			auto t2 = std::clamp(t * 2 - 1.0f, 0.0f, 1.0f);
			auto start = particle.position_start;
			auto middle = particle.position_middle;
			auto end = particle.position_end;

			particle.position = Vec3::Lerp(Vec3::Lerp(start, middle, easing(t1)), end, easing(t2));
		}
	}
	else
	{
		for (auto& particle : particles_)
		{
			auto t = particle.time / particle.time_to_live;
			auto start = particle.position_start;
			auto end = particle.position_end;

			particle.position = Vec3::Lerp(start, end, easing(t));
		}
	}
}

void ParticleEmitter::UpdateRotation(const ParticleEmitterProperty& prop, const float delta_time)
{
	switch (prop.rotation_type)
	{
	case PROP_TYPE_PVA:
		UpdateRotation(prop.rotation_pva, delta_time);
		break;
	}
}

void ParticleEmitter::UpdateRotation(const ParticleEmitterProperty::RotationPropertyPVA& prop, const float delta_time)
{
	for (auto& particle : particles_)
	{
		particle.rotation_velocity += particle.rotation_acceleration;
		particle.rotation += particle.rotation_velocity * delta_time;
	}
}

void ParticleEmitter::UpdateScale(const ParticleEmitterProperty& prop, const float delta_time)
{
	switch (prop.scale_type)
	{
	case PROP_TYPE_PVA:
		UpdateScale(prop.scale_pva, delta_time);
		break;
	case PROP_TYPE_EASING:
		UpdateScale(prop.scale_easing, delta_time);
		break;
	}
}

void ParticleEmitter::UpdateScale(const ParticleEmitterProperty::ScalePropertyPVA& prop, const float delta_time)
{
	for (auto& particle : particles_)
	{
		particle.scale_velocity += particle.scale_acceleration;
		particle.scale += particle.scale_velocity * delta_time;
	}
}

void ParticleEmitter::UpdateScale(const ParticleEmitterProperty::ScalePropertyEasing& prop, const float delta_time)
{
	float (*easing)(const float);
	easing = GetEasingFunc(prop.type);

	if (prop.middle_enabled)
	{
		for (auto& particle : particles_)
		{
			auto t = particle.time / particle.time_to_live;
			auto t1 = std::clamp(t * 2, 0.0f, 1.0f);
			auto t2 = std::clamp(t * 2 - 1.0f, 0.0f, 1.0f);
			auto start = particle.scale_start;
			auto middle = particle.scale_middle;
			auto end = particle.scale_end;

			particle.scale = Vec3::Lerp(Vec3::Lerp(start, middle, easing(t1)), end, easing(t2));
		}
	}
	else
	{
		for (auto& particle : particles_)
		{
			auto t = particle.time / particle.time_to_live;
			auto start = particle.scale_start;
			auto end = particle.scale_end;

			particle.scale = Vec3::Lerp(start, end, easing(t));
		}
	}
}

void ParticleEmitter::UpdateColor(const ParticleEmitterProperty& prop, const float delta_time)
{
	switch (prop.color_type)
	{
	case PROP_TYPE_PVA:
		UpdateColor(prop.color_pva, delta_time);
		break;
	case PROP_TYPE_EASING:
		UpdateColor(prop.color_easing, delta_time);
		break;
	}

	// HSVA mode なら変換する
	if (prop.color_mode == COLOR_MODE_HSVA)
	{
		for (auto& particle : particles_)
		{
			auto c = particle.color;
			particle.final_color = Color::HSVtoRGB(c.r, c.g, c.b, c.a);
		}
	}
	else
	{
		for (auto& particle : particles_)
		{
			particle.final_color = particle.color;
		}
	}
}

void ParticleEmitter::UpdateColor(const ParticleEmitterProperty::ColorPropertyPVA& prop, const float delta_time)
{
	for (auto& particle : particles_)
	{
		particle.color_velocity += particle.color_acceleration;
		particle.color += particle.color_velocity * delta_time;
	}
}

void ParticleEmitter::UpdateColor(const ParticleEmitterProperty::ColorPropertyEasing& prop, const float delta_time)
{
	float (*easing)(const float);
	easing = GetEasingFunc(prop.type);

	if (prop.middle_enabled)
	{
		for (auto& particle : particles_)
		{
			auto t = particle.time / particle.time_to_live;
			auto t1 = std::clamp(t * 2, 0.0f, 1.0f);
			auto t2 = std::clamp(t * 2 - 1.0f, 0.0f, 1.0f);
			auto start = particle.color_start;
			auto middle = particle.color_middle;
			auto end = particle.color_end;

			particle.color = Color::Lerp(Color::Lerp(start, middle, easing(t1)), end, easing(t2));
		}
	}
	else
	{
		for (auto& particle : particles_)
		{
			auto t = particle.time / particle.time_to_live;
			auto start = particle.color_start;
			auto end = particle.color_end;

			particle.color = Color::Lerp(start, end, easing(t));
		}
	}
}

float (*ParticleEmitter::GetEasingFunc(const ParticleEmitterProperty::EasingType& type))(const float)
{
	switch (type)
	{
	case EASE_LINEAR:
		return &Easing::Linear;
	case EASE_IN_CUBIC:
		return &Easing::InCubic;
	case EASE_OUT_CUBIC:
		return &Easing::OutCubic;
	case EASE_INOUT_CUBIC:
		return &Easing::InOutCubic;
	}

	return &Easing::Linear;
}
