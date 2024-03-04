#include "app/component/player_controller.h"
#include "game/component/animator.h"
#include "game/component/audio/audio_source.h"
#include "game/component/collider/capsule_collider.h"
#include "game/component/mesh_renderer.h"
#include "game/component/particle_emitter.h"
#include "game/component/rigidbody.h"
#include "game/game.h"
#include "game/resource/audio.h"
#include "game/resource/model.h"
#include "game/resource/texture2d.h"
#include "player.h"

Player::Player(const std::string& name) : Entity(name, "player", "player")
{
	auto game = Game::Get();

	auto model = game->LoadResource<Model>(L"assets/model/character/Character2.glb");

	float pbr_color[4] = { 0, 1, 0, 1 };
	auto smoke_albedo = game->LoadResource<Texture2D>(L"assets/effect/smoke_albedo.png");
	auto smoke_normal = game->LoadResource<Texture2D>(L"assets/effect/smoke_normal.png");
	std::shared_ptr smoke_pbr = Texture2D::GetMono(pbr_color);

	auto run_smoke_prop = ParticleEmitterProperty{};
	run_smoke_prop.time_to_live = 50;
	run_smoke_prop.time_to_live_range = 10;
	run_smoke_prop.spawn_rate = 0.2f;
	run_smoke_prop.spawn_rate_range = 0;
	run_smoke_prop.position_type = PROP_TYPE_EASING;
	run_smoke_prop.position_easing.type = EASE_LINEAR;
	run_smoke_prop.position_easing.start = Vec3(0, 0.3f, -1);
	run_smoke_prop.position_easing.start_range = Vec3(0.5f, 0.1f, 0.5f);
	run_smoke_prop.position_easing.end = Vec3(0, 2, -2);
	run_smoke_prop.position_easing.end_range = Vec3(0.5f, 0.8f, 0.5f);
	run_smoke_prop.rotation_pva.rotation_range = Vec3(0, 0, 3.14f);
	run_smoke_prop.rotation_pva.velocity_range = Vec3(0, 0, 6);
	run_smoke_prop.scale_type = PROP_TYPE_EASING;
	run_smoke_prop.scale_easing.type = EASE_OUT_CUBIC;
	run_smoke_prop.scale_easing.keep_aspect = true;
	run_smoke_prop.scale_easing.middle_enabled = true;
	run_smoke_prop.scale_easing.start = Vec3(0.1f, 0.1f, 0.1f);
	run_smoke_prop.scale_easing.middle = Vec3(0.5f, 0.5f, 0.5f);
	run_smoke_prop.scale_easing.middle_range = Vec3(0.1f, 0.1f, 0.1f);
	run_smoke_prop.scale_easing.end = Vec3(0, 0, 0);
	run_smoke_prop.sprite.albedo_texture = smoke_albedo;
	run_smoke_prop.sprite.normal_texture = smoke_normal;
	run_smoke_prop.sprite.pbr_texture = smoke_pbr;

	auto jump_smoke_prop = ParticleEmitterProperty{};
	jump_smoke_prop.spawn_count = 5;
	jump_smoke_prop.time_to_live = 50;
	jump_smoke_prop.spawn_rate = 0.5f;
	jump_smoke_prop.position_pva.position = Vec3(0, 0.5f, 0);
	jump_smoke_prop.position_pva.velocity_range = Vec3(0.01f, 0.01f, 0.01f);
	jump_smoke_prop.rotation_pva.rotation_range = Vec3(0, 0, 3.14f);
	jump_smoke_prop.rotation_pva.velocity_range = Vec3(0, 0, 6);
	jump_smoke_prop.scale_type = PROP_TYPE_EASING;
	jump_smoke_prop.scale_easing.type = EASE_LINEAR;
	jump_smoke_prop.scale_easing.start = Vec3(0.6f, 0.6f, 0.6f);
	jump_smoke_prop.scale_easing.end = Vec3(0, 0, 0);
	jump_smoke_prop.sprite.albedo_texture = smoke_albedo;
	jump_smoke_prop.sprite.normal_texture = smoke_normal;
	jump_smoke_prop.sprite.pbr_texture = smoke_pbr;

	auto circle_smoke_prop = ParticleEmitterProperty{};
	circle_smoke_prop.spawn_count = 5;
	circle_smoke_prop.spawn_rate = 5;
	circle_smoke_prop.time_to_live = 35;
	circle_smoke_prop.time_to_live_range = 15;
	circle_smoke_prop.spawning_method = SPAWN_METHOD_CIRCLE;
	circle_smoke_prop.spawning_circle.radius = 0.5;
	circle_smoke_prop.spawning_circle.vertices = 5;
	circle_smoke_prop.position_type = PROP_TYPE_EASING;
	circle_smoke_prop.position_easing.type = EASE_OUT_CUBIC;
	circle_smoke_prop.position_easing.start = Vec3(0, 0.3f, 0);
	circle_smoke_prop.position_easing.end = Vec3(2, 0.8f, 0);
	circle_smoke_prop.position_easing.end_range = Vec3(0.1f, 0.1f, 0.1f);
	circle_smoke_prop.rotation_pva.rotation_range = Vec3(0, 0, 3.14f);
	circle_smoke_prop.rotation_pva.velocity_range = Vec3(0, 0, 6);
	circle_smoke_prop.scale_type = PROP_TYPE_EASING;
	circle_smoke_prop.scale_easing.type = EASE_OUT_CUBIC;
	circle_smoke_prop.scale_easing.keep_aspect = true;
	circle_smoke_prop.scale_easing.middle_enabled = true;
	circle_smoke_prop.scale_easing.start = Vec3(0.1f, 0.1f, 0.1f);
	circle_smoke_prop.scale_easing.middle = Vec3(0.4f, 0.4f, 0.4f);
	//circleSmokeProp.scaleEasing.middleRange = Vec3(0.1, 0.1, 0.1);
	circle_smoke_prop.scale_easing.end = Vec3(0, 0, 0);
	circle_smoke_prop.sprite.albedo_texture = smoke_albedo;
	circle_smoke_prop.sprite.normal_texture = smoke_normal;
	circle_smoke_prop.sprite.pbr_texture = smoke_pbr;

	auto wall_slide_smoke_prop = ParticleEmitterProperty{};
	wall_slide_smoke_prop.time_to_live = 50;
	wall_slide_smoke_prop.time_to_live_range = 10;
	wall_slide_smoke_prop.spawn_rate = 0.08f;
	wall_slide_smoke_prop.spawn_rate_range = 0;
	wall_slide_smoke_prop.position_type = PROP_TYPE_EASING;
	wall_slide_smoke_prop.position_easing.type = EASE_LINEAR;
	wall_slide_smoke_prop.position_easing.start = Vec3(-0.5f, 2, -0.5f);
	wall_slide_smoke_prop.position_easing.start_range = Vec3(0.5f, 0.5f, 0.1f);
	wall_slide_smoke_prop.position_easing.end = Vec3(-0.5f, 4, -0.2f);
	wall_slide_smoke_prop.position_easing.end_range = Vec3(0.5f, 0.5f, 0.8f);
	wall_slide_smoke_prop.rotation_pva.rotation_range = Vec3(0, 0, 3.14f);
	wall_slide_smoke_prop.rotation_pva.velocity_range = Vec3(0, 0, 6);
	wall_slide_smoke_prop.scale_type = PROP_TYPE_EASING;
	wall_slide_smoke_prop.scale_easing.type = EASE_OUT_CUBIC;
	wall_slide_smoke_prop.scale_easing.keep_aspect = true;
	wall_slide_smoke_prop.scale_easing.middle_enabled = true;
	wall_slide_smoke_prop.scale_easing.start = Vec3(0.1f, 0.1f, 0.1f);
	wall_slide_smoke_prop.scale_easing.middle = Vec3(0.5f, 0.5f, 0.5f);
	wall_slide_smoke_prop.scale_easing.middle_range = Vec3(0.1f, 0.1f, 0.1f);
	wall_slide_smoke_prop.scale_easing.end = Vec3(0, 0, 0);
	wall_slide_smoke_prop.sprite.albedo_texture = smoke_albedo;
	wall_slide_smoke_prop.sprite.normal_texture = smoke_normal;
	wall_slide_smoke_prop.sprite.pbr_texture = smoke_pbr;

	AddComponent(new MeshRenderer(model));
	AddComponent(new CapsuleCollider({ 1, 1 }));
	AddComponent(new Rigidbody(1, true, false, 0));
	AddComponent(new Animator());
	AddComponent(new AudioSource(game->LoadResource<Audio>(L"assets/se/Retro Jump Classic 08.wav")));
	AddComponent(new PlayerController(20, 1.2f));

	GetComponent<Animator>()->Play("Idle", 2.0f);
	GetComponent<Collider>()->offset = Vec3(0, 1.5f, 0);
	GetComponent<MeshRenderer>()->is_shadow_from_above = true;

	auto run_smoke_emitter = new Entity("run_smoke_emitter");
	run_smoke_emitter->AddComponent(new ParticleEmitter(run_smoke_prop));

	auto jump_smoke_emitter = new Entity("jump_smoke_emitter");
	jump_smoke_emitter->AddComponent(new ParticleEmitter(jump_smoke_prop));

	auto circle_smoke_emitter = new Entity("circle_smoke_emitter");
	circle_smoke_emitter->AddComponent(new ParticleEmitter(circle_smoke_prop));

	auto wall_slide_smoke_emitter = new Entity("wall_slide_smoke_emitter");
	wall_slide_smoke_emitter->AddComponent(new ParticleEmitter(wall_slide_smoke_prop));

	AddChild(run_smoke_emitter);
	AddChild(jump_smoke_emitter);
	AddChild(circle_smoke_emitter);
	AddChild(wall_slide_smoke_emitter);
}
