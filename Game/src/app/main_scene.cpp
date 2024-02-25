#include "app/main_scene.h"
#include "engine/shared_struct.h"
#include "game/animation.h"
#include "game/assimp_loader.h"
#include "game/entity.h"
#include "game/component/mesh_renderer.h"
#include "game/component/animator.h"
#include "game/component/sphere_mesh.h"
#include "game/component/camera.h"
#include "game/component/particle_emitter.h"
#include "game/resource/texture2d.h"
#include "game/component/rigidbody.h"
#include "game/component/collider/capsule_collider.h"
#include "game/component/collider/collider.h"
#include "game/component/collider/mesh_collider.h"
#include "game/resource/model.h"

#include "app/player.h"
#include "app/game_camera.h"

bool MainScene::Init()
{
	Scene::Init();

	std::shared_ptr<Model> model;
	std::vector<Animation*> animations;

	SetSkybox(L"Assets/grass2");

	auto playerModel = LoadResource<Model>(L"Assets/PlatformerPack/Character.gltf");

	float pbrColor[4] = { 0, 1, 0, 1 };
	std::shared_ptr smokeAlbedo = Texture2D::Load("Assets/smoke2_albedo.png");
	std::shared_ptr smokeNormal = Texture2D::Load("Assets/smoke2_normal.png");
	std::shared_ptr smokePbr = Texture2D::GetMono(pbrColor);

	auto runSmokeProp = ParticleEmitterProperty{};
	runSmokeProp.time_to_live = 50;
	runSmokeProp.time_to_live_range = 10;
	runSmokeProp.spawn_rate = 0.2f;
	runSmokeProp.spawn_rate_range = 0;
	runSmokeProp.position_type = PARTICAL_PROP_TYPE_EASING;
	runSmokeProp.position_easing.type = PARTICLE_EASE_LINEAR;
	runSmokeProp.position_easing.start = Vec3(0, 0.3f, -1);
	runSmokeProp.position_easing.start_range = Vec3(0.5f, 0.1f, 0.5f);
	runSmokeProp.position_easing.end = Vec3(0, 2, -2);
	runSmokeProp.position_easing.end_range = Vec3(0.5f, 0.8f, 0.5f);
	/*runSmokeProp.positionPVA.position = Vec3(0, 0.7, -1);
	runSmokeProp.positionPVA.positionRange = Vec3(0.5, 0.1, 0.2);
	runSmokeProp.positionPVA.velocity = Vec3(0, 0.01, -0.05);
	runSmokeProp.positionPVA.velocityRange = Vec3(0.01, 0.01, 0.01);*/
	runSmokeProp.rotation_pva.rotation_range = Vec3(0, 0, 3.14f);
	runSmokeProp.rotation_pva.velocity_range = Vec3(0, 0, 0.1f);
	runSmokeProp.scaleType = PARTICAL_PROP_TYPE_EASING;
	runSmokeProp.scale_easing.type = PARTICLE_EASE_OUT_CUBIC;
	runSmokeProp.scale_easing.keep_aspect = true;
	runSmokeProp.scale_easing.middle_enabled = true;
	runSmokeProp.scale_easing.start = Vec3(0.1f, 0.1f, 0.1f);
	runSmokeProp.scale_easing.middle = Vec3(0.5f, 0.5f, 0.5f);
	runSmokeProp.scale_easing.middle_range = Vec3(0.1f, 0.1f, 0.1f);
	runSmokeProp.scale_easing.end = Vec3(0, 0, 0);
	runSmokeProp.sprite.albedo_texture = smokeAlbedo;
	runSmokeProp.sprite.normal_texture = smokeNormal;
	runSmokeProp.sprite.pbr_texture = smokePbr;

	auto jumpSmokeProp = ParticleEmitterProperty{};
	jumpSmokeProp.spawn_count = 5;
	jumpSmokeProp.time_to_live = 50;
	jumpSmokeProp.spawn_rate = 0.5f;
	jumpSmokeProp.position_pva.position = Vec3(0, 0.5f, 0);
	jumpSmokeProp.position_pva.velocity_range = Vec3(0.01f, 0.01f, 0.01f);
	jumpSmokeProp.rotation_pva.rotation_range = Vec3(0, 0, 3.14f);
	jumpSmokeProp.rotation_pva.velocity_range = Vec3(0, 0, 0.1f);
	jumpSmokeProp.scaleType = PARTICAL_PROP_TYPE_EASING;
	jumpSmokeProp.scale_easing.type = PARTICLE_EASE_LINEAR;
	jumpSmokeProp.scale_easing.start = Vec3(0.6f, 0.6f, 0.6f);
	jumpSmokeProp.scale_easing.end = Vec3(0, 0, 0);
	jumpSmokeProp.sprite.albedo_texture = smokeAlbedo;
	jumpSmokeProp.sprite.normal_texture = smokeNormal;
	jumpSmokeProp.sprite.pbr_texture = smokePbr;

	auto circleSmokeProp = ParticleEmitterProperty{};
	circleSmokeProp.spawn_count = 5;
	circleSmokeProp.spawn_rate = 5;
	circleSmokeProp.time_to_live = 35;
	circleSmokeProp.time_to_live_range = 15;
	circleSmokeProp.spawning_method = PARTICAL_SPAWN_METHOD_CIRCLE;
	circleSmokeProp.spawning_circle.radius = 0.5f;
	circleSmokeProp.spawning_circle.vertices = 5;
	circleSmokeProp.position_type = PARTICAL_PROP_TYPE_EASING;
	circleSmokeProp.position_easing.type = PARTICLE_EASE_OUT_CUBIC;
	circleSmokeProp.position_easing.start = Vec3(0, 0.3f, 0);
	circleSmokeProp.position_easing.end = Vec3(2, 0.8f, 0);
	circleSmokeProp.position_easing.end_range = Vec3(0.1f, 0.1f, 0.1f);
	circleSmokeProp.rotation_pva.rotation_range = Vec3(0, 0, 3.14f);
	circleSmokeProp.rotation_pva.velocity_range = Vec3(0, 0, 0.1f);
	circleSmokeProp.scaleType = PARTICAL_PROP_TYPE_EASING;
	circleSmokeProp.scale_easing.type = PARTICLE_EASE_OUT_CUBIC;
	circleSmokeProp.scale_easing.keep_aspect = true;
	circleSmokeProp.scale_easing.middle_enabled = true;
	circleSmokeProp.scale_easing.start = Vec3(0.1f, 0.1f, 0.1f);
	circleSmokeProp.scale_easing.middle = Vec3(0.4f, 0.4f, 0.4f);
	//circleSmokeProp.scaleEasing.middleRange = Vec3(0.1, 0.1, 0.1);
	circleSmokeProp.scale_easing.end = Vec3(0, 0, 0);
	circleSmokeProp.sprite.albedo_texture = smokeAlbedo;
	circleSmokeProp.sprite.normal_texture = smokeNormal;
	circleSmokeProp.sprite.pbr_texture = smokePbr;

	auto player = new Entity("Player");
	player->AddComponent(new MeshRenderer(playerModel));
	auto collider = (Collider*)player->AddComponent(new CapsuleCollider({ 1, 1 }));
	player->AddComponent(new Rigidbody(1, true, false, 0.1f));
	player->AddComponent(new Animator());
	player->AddComponent(new Player({ 0.27f, 0.02f }));
	CreateEntity(player);

	player->GetComponent<Animator>()->Play("Idle", 2.0f);
	collider->offset = Vec3(0, 1.5f, 0);
	player->GetComponent<MeshRenderer>()->is_shadow_from_above = true;

	auto runSmokeEmitter = new Entity("Run Smoke Emitter");
	runSmokeEmitter->AddComponent(new ParticleEmitter(runSmokeProp));
	runSmokeEmitter->SetParent(player);
	CreateEntity(runSmokeEmitter);

	auto jumpSmokeEmitter = new Entity("Jump Smoke Emitter");
	jumpSmokeEmitter->AddComponent(new ParticleEmitter(jumpSmokeProp));
	jumpSmokeEmitter->SetParent(player);
	CreateEntity(jumpSmokeEmitter);

	auto circleSmokeEmitter = new Entity("Circle Smoke Emitter");
	circleSmokeEmitter->AddComponent(new ParticleEmitter(circleSmokeProp));
	circleSmokeEmitter->SetParent(player);
	CreateEntity(circleSmokeEmitter);


	auto plane = new Entity();
	plane->AddComponent(new MeshRenderer(LoadResource<Model>(L"Assets/PlatformerPack/Cube_Grass_Single.gltf")));
	collider = (Collider*)plane->AddComponent(new MeshCollider(LoadResource<CollisionModel>(L"Assets/PlatformerPack/Cube_Grass_Single.gltf")));
	plane->AddComponent(new Rigidbody(1000, false, true, 0.5));
	CreateEntity(plane);

	plane->transform->scale = Vec3(2, 2, 2);
	plane->transform->position = Vec3(0, -2, 0);


	model = SphereMesh::Load(1, 0.5, 0.5, 0.5);
	auto sphere = new Entity();
	sphere->AddComponent(new MeshRenderer({ model }));
	CreateEntity(sphere);

	sphere->transform->scale = Vec3(2, 2, 2);
	sphere->transform->position = Vec3(5, -2, 0);

	/*AssimpLoader::Load(L"Assets/MikuNT.glb", model, animations);
	auto miku = new Entity();
	miku->AddComponent(new MeshRenderer({ model }));
	CreateEntity(miku);
	miku->transform->scale = Vec3(5, 5, 5);
	miku->transform->position.y = -3;*/


	auto camera = new Entity();
	camera->AddComponent(new Camera());
	camera->AddComponent(new GameCamera({ player }));
	CreateEntity(camera);
	SetMainCamera(camera);

    return true;
}
