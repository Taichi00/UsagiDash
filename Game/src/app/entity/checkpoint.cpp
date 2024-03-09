#include "checkpoint.h"
#include "game/component/mesh_renderer.h"
#include "game/component/collider/sphere_collider.h"
#include "game/game.h"
#include "app/component/checkpoint_controller.h"
#include "game/component/particle_emitter.h"
#include "game/resource/texture2d.h"
#include "game/component/animator.h"
#include "game/resource/audio.h"
#include "game/component/audio/audio_source.h"
#include "app/component/pause_behavior.h"

Checkpoint::Checkpoint() : Entity("", "checkpoint", "event")
{
	auto game = Game::Get();
	auto model = game->LoadResource<Model>(L"assets/model/object/flag.gltf");
	auto audio = game->LoadResource<Audio>(L"assets/se/confirmation_002.wav");

	AddComponent(new MeshRenderer(model));
	AddComponent(new SphereCollider(5));
	AddComponent(new Animator());
	AddComponent(new AudioSource(audio));
	AddComponent(new CheckpointController());
	AddComponent(new PauseBehavior());

	auto confetti = new Entity("confetti_emitter");
	{
		float normal_color[] = { 0.5f, 0.5f, 1, 1 };
		float pbr_color[] = { 0, 0.2f, 0, 1 };
		std::shared_ptr albedo = game->LoadResource<Texture2D>(L"assets/effect/confetti.png");
		std::shared_ptr normal = Texture2D::GetMono(normal_color);
		std::shared_ptr pbr = Texture2D::GetMono(pbr_color);

		ParticleEmitterProperty prop = {};
		prop.time_to_live = 100;
		prop.time_to_live_range = 20;
		prop.spawn_count = 30;
		prop.spawn_rate = 50;
		prop.spawn_rate_range = 0;
		prop.position_type = PROP_TYPE_PVA;
		prop.position_pva.position = Vec3(0, 6, 0);
		prop.position_pva.position_range = Vec3(1.5f, 1.5f, 1.5f);
		prop.position_pva.velocity = Vec3(0, 10, 0);
		prop.position_pva.velocity_range = Vec3(6, 7, 6);
		prop.position_pva.acceleration = Vec3(0, -0.5f, 0);
		prop.rotation_type = PROP_TYPE_PVA;
		prop.rotation_pva.rotation_range = Vec3(3.14f, 3.14f, 3.14f);
		prop.rotation_pva.velocity_range = Vec3(30, 30, 30);
		prop.scale_type = PROP_TYPE_PVA;
		prop.scale_pva.scale = Vec3(1.5f, 1.5f, 1.5f);
		prop.scale_pva.scale_range = Vec3(0.5f, 0.5f, 0.5f);
		prop.color_type = PROP_TYPE_PVA;
		prop.color_mode = COLOR_MODE_HSVA;
		prop.color_pva.color = Color(130, 1, 1, 1);
		prop.color_pva.color_range = Color(0, 0, 0, 0);
		prop.particle_type = PARTICLE_TYPE_MESH;
		prop.mesh.albedo_texture = albedo;
		prop.mesh.normal_texture = normal;
		prop.mesh.pbr_texture = pbr;

		confetti->AddComponent(new ParticleEmitter(prop));
		confetti->AddComponent(new PauseBehavior());
	}
	AddChild(confetti);

	auto smoke = new Entity("smoke_emitter");
	{
		float pbr_color[] = { 0, 1, 0, 1 };
		std::shared_ptr albedo = game->LoadResource<Texture2D>(L"assets/effect/smoke_albedo.png");
		std::shared_ptr normal = game->LoadResource<Texture2D>(L"assets/effect/smoke_normal.png");
		std::shared_ptr pbr = Texture2D::GetMono(pbr_color);

		ParticleEmitterProperty prop = {};
		prop.time_to_live = 50;
		prop.time_to_live_range = 10;
		prop.spawn_count = 10;
		prop.spawn_rate = 10;
		prop.spawn_rate_range = 0;
		prop.position_type = PROP_TYPE_EASING;
		prop.position_easing.type = EASE_OUT_CUBIC;
		prop.position_easing.start = Vec3(0, 5, 0);
		prop.position_easing.start_range = Vec3(1, 1, 1);
		prop.position_easing.end = Vec3(0, 5, 0);
		prop.position_easing.end_range = Vec3(3, 3, 3);
		prop.rotation_type = PROP_TYPE_PVA;
		prop.rotation_pva.rotation_range = Vec3(0, 0, 3.14f);
		prop.rotation_pva.velocity_range = Vec3(0, 0, 6);
		prop.scale_type = PROP_TYPE_EASING;
		prop.scale_easing.keep_aspect = true;
		prop.scale_easing.type = EASE_IN_CUBIC;
		prop.scale_easing.start = Vec3(0.7f, 0.7f, 0.7f);
		prop.scale_easing.start_range = Vec3(0.2f, 0.2f, 0.2f);
		prop.scale_easing.end = Vec3(0, 0, 0);
		prop.color_pva.color = Color(1, 1, 1);
		prop.particle_type = PARTICLE_TYPE_SPRITE;
		prop.sprite.albedo_texture = albedo;
		prop.sprite.normal_texture = normal;
		prop.sprite.pbr_texture = pbr;

		smoke->AddComponent(new ParticleEmitter(prop));
		smoke->AddComponent(new PauseBehavior());
	}
	AddChild(smoke);
}
