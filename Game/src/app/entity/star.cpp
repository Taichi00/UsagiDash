#include "star.h"
#include "game/component/mesh_renderer.h"
#include "game/game.h"
#include "game/component/collider/sphere_collider.h"
#include "app/component/pause_behavior.h"
#include "app/component/star_controller.h"
#include "game/component/particle_emitter.h"
#include "game/component/audio/audio_source.h"
#include "game/component/animator.h"

Star::Star() : Entity("star", "star", "item")
{
	auto game = Game::Get();

	ParticleEmitterProperty confetti_prop = {};
	{
		float normal_color[] = { 0.5f, 0.5f, 1, 1 };
		float pbr_color[] = { 1, 0.2f, 0, 1 };
		std::shared_ptr albedo = game->LoadResource<Texture2D>(L"assets/effect/twinkle.png");
		std::shared_ptr normal = Texture2D::GetMono(normal_color);
		std::shared_ptr pbr = Texture2D::GetMono(pbr_color);

		confetti_prop.time_to_live = 100;
		confetti_prop.time_to_live_range = 20;
		confetti_prop.spawn_count = 0;
		confetti_prop.spawn_rate = 0.5f;
		confetti_prop.spawn_rate_range = 0;
		confetti_prop.position_type = PROP_TYPE_EASING;
		confetti_prop.position_easing.type = EASE_OUT_CUBIC;
		confetti_prop.position_easing.start = Vec3(0, 0, 0);
		confetti_prop.position_easing.start_range = Vec3(3, 3, 3);
		confetti_prop.position_easing.end = Vec3(0, 0, 0);
		confetti_prop.position_easing.end_range = Vec3(6, 6, 6);
		confetti_prop.rotation_type = PROP_TYPE_PVA;
		confetti_prop.rotation_pva.rotation_range = Vec3(3.14f, 3.14f, 3.14f);
		confetti_prop.rotation_pva.velocity_range = Vec3(30, 30, 30);
		confetti_prop.scale_type = PROP_TYPE_EASING;
		confetti_prop.scale_easing.keep_aspect = true;
		confetti_prop.scale_easing.type = EASE_IN_CUBIC;
		confetti_prop.scale_easing.start = Vec3(0.3f, 0.3f, 0.3f);
		confetti_prop.scale_easing.start_range = Vec3(0.1f, 0.1f, 0.1f);
		confetti_prop.scale_easing.end = Vec3(0, 0, 0);
		confetti_prop.color_type = PROP_TYPE_PVA;
		confetti_prop.color_mode = COLOR_MODE_RGBA;
		confetti_prop.color_pva.color = Color(1, 1, 1, 1);
		confetti_prop.color_pva.color_range = Color(0, 0, 0, 0);
		confetti_prop.particle_type = PARTICLE_TYPE_MESH;
		confetti_prop.mesh.albedo_texture = albedo;
		confetti_prop.mesh.normal_texture = normal;
		confetti_prop.mesh.pbr_texture = pbr;
	}

	AddComponent(new MeshRenderer(game->LoadResource<Model>(L"assets/model/item/Star.gltf")));
	AddComponent(new SphereCollider(4));
	AddComponent(new Animator());

	auto confetti = AddComponent<ParticleEmitter>(new ParticleEmitter(confetti_prop));

	auto audio_twinkle = AddComponent<AudioSource>(
		game->LoadResource<Audio>(L"assets/se/magical-background-6892.wav"), 1.0f, 1.0f, 80.0f
	);
	auto audio_pick = AddComponent<AudioSource>(
		game->LoadResource<Audio>(L"assets/se/DM-CGS-08.wav"), 0.5f
	);

	AddComponent(new StarController(confetti, audio_twinkle, audio_pick));
	AddComponent(new PauseBehavior());
}
