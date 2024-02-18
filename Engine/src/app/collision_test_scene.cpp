#include "app/collision_test_scene.h"
#include "app/entity/coin.h"
#include "app/floating_player.h"
#include "app/game_camera.h"
#include "app/main_scene.h"
#include "app/player.h"
#include "engine/shared_struct.h"
#include "game/animation.h"
#include "game/assimp_loader.h"
#include "game/collision_manager.h"
#include "game/component/animator.h"
#include "game/component/billboard_renderer.h"
#include "game/component/camera.h"
#include "game/component/capsule_mesh.h"
#include "game/component/collider/capsule_collider.h"
#include "game/component/collider/floor_collider.h"
#include "game/component/collider/mesh_collider.h"
#include "game/component/collider/sphere_collider.h"
#include "game/component/gui/control.h"
#include "game/component/gui/label.h"
#include "game/component/gui/panel.h"
#include "game/component/mesh_renderer.h"
#include "game/component/particle_emitter.h"
#include "game/component/rigidbody.h"
#include "game/component/sphere_mesh.h"
#include "game/entity.h"
#include "game/game.h"
#include "game/input.h"
#include "game/physics.h"
#include "game/resource/texture2d.h"
#include "math/quaternion.h"
#include "math/rect.h"
#include "app/component/map_loader.h"
#include <memory>
#include <random>

Entity* player, *enemy, *testSphere, *movingObj;
float angle = 0;
Vec3 direction = Vec3(1, 0, 0);

bool CollisionTestScene::Init()
{
	Scene::Init();

	SetSkybox(L"Assets/skybox/puresky/");

	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<float> frand(-1, 1);

	auto playerModel = LoadResource<Model>(L"Assets/PlatformerPack/Character2.glb");

	float pbrColor[4] = { 0, 1, 0, 1 };
	std::shared_ptr smokeAlbedo = Texture2D::Load(L"Assets/smoke2_albedo.png");
	std::shared_ptr smokeNormal = Texture2D::Load(L"Assets/smoke2_normal.png");
	std::shared_ptr smokePbr = Texture2D::GetMono(pbrColor);

	auto runSmokeProp = ParticleEmitterProperty{};
	runSmokeProp.time_to_live = 50;
	runSmokeProp.time_to_live_range = 10;
	runSmokeProp.spawn_rate = 0.2;
	runSmokeProp.spawn_rate_range = 0;
	runSmokeProp.position_type = PARTICAL_PROP_TYPE_EASING;
	runSmokeProp.position_easing.type = PARTICLE_EASE_LINEAR;
	runSmokeProp.position_easing.start = Vec3(0, 0.3, -1);
	runSmokeProp.position_easing.start_range = Vec3(0.5, 0.1, 0.5);
	runSmokeProp.position_easing.end = Vec3(0, 2, -2);
	runSmokeProp.position_easing.end_range = Vec3(0.5, 0.8, 0.5);
	runSmokeProp.rotation_pva.rotation_range = Vec3(0, 0, 3.14);
	runSmokeProp.rotation_pva.velocity_range = Vec3(0, 0, 0.1);
	runSmokeProp.scaleType = PARTICAL_PROP_TYPE_EASING;
	runSmokeProp.scale_easing.type = PARTICLE_EASE_OUT_CUBIC;
	runSmokeProp.scale_easing.keep_aspect = true;
	runSmokeProp.scale_easing.middle_enabled = true;
	runSmokeProp.scale_easing.start = Vec3(0.1, 0.1, 0.1);
	runSmokeProp.scale_easing.middle = Vec3(0.5, 0.5, 0.5);
	runSmokeProp.scale_easing.middle_range = Vec3(0.1, 0.1, 0.1);
	runSmokeProp.scale_easing.end = Vec3(0, 0, 0);
	runSmokeProp.sprite.albedo_texture = smokeAlbedo;
	runSmokeProp.sprite.normal_texture = smokeNormal;
	runSmokeProp.sprite.pbr_texture = smokePbr;

	auto jumpSmokeProp = ParticleEmitterProperty{};
	jumpSmokeProp.spawn_count = 5;
	jumpSmokeProp.time_to_live = 50;
	jumpSmokeProp.spawn_rate = 0.5;
	jumpSmokeProp.position_pva.position = Vec3(0, 0.5, 0);
	jumpSmokeProp.position_pva.velocity_range = Vec3(0.01, 0.01, 0.01);
	jumpSmokeProp.rotation_pva.rotation_range = Vec3(0, 0, 3.14);
	jumpSmokeProp.rotation_pva.velocity_range = Vec3(0, 0, 0.1);
	jumpSmokeProp.scaleType = PARTICAL_PROP_TYPE_EASING;
	jumpSmokeProp.scale_easing.type = PARTICLE_EASE_LINEAR;
	jumpSmokeProp.scale_easing.start = Vec3(0.6, 0.6, 0.6);
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
	circleSmokeProp.spawning_circle.radius = 0.5;
	circleSmokeProp.spawning_circle.vertices = 5;
	circleSmokeProp.position_type = PARTICAL_PROP_TYPE_EASING;
	circleSmokeProp.position_easing.type = PARTICLE_EASE_OUT_CUBIC;
	circleSmokeProp.position_easing.start = Vec3(0, 0.3, 0);
	circleSmokeProp.position_easing.end = Vec3(2, 0.8, 0);
	circleSmokeProp.position_easing.end_range = Vec3(0.1, 0.1, 0.1);
	circleSmokeProp.rotation_pva.rotation_range = Vec3(0, 0, 3.14);
	circleSmokeProp.rotation_pva.velocity_range = Vec3(0, 0, 0.1);
	circleSmokeProp.scaleType = PARTICAL_PROP_TYPE_EASING;
	circleSmokeProp.scale_easing.type = PARTICLE_EASE_OUT_CUBIC;
	circleSmokeProp.scale_easing.keep_aspect = true;
	circleSmokeProp.scale_easing.middle_enabled = true;
	circleSmokeProp.scale_easing.start = Vec3(0.1, 0.1, 0.1);
	circleSmokeProp.scale_easing.middle = Vec3(0.4, 0.4, 0.4);
	//circleSmokeProp.scaleEasing.middleRange = Vec3(0.1, 0.1, 0.1);
	circleSmokeProp.scale_easing.end = Vec3(0, 0, 0);
	circleSmokeProp.sprite.albedo_texture = smokeAlbedo;
	circleSmokeProp.sprite.normal_texture = smokeNormal;
	circleSmokeProp.sprite.pbr_texture = smokePbr;


	player = new Entity("Player", "player");
	player->AddComponent(new MeshRenderer(playerModel));
	player->AddComponent(new CapsuleCollider({ 1, 1 }));
	player->AddComponent(new Rigidbody(1, true, false, 0));
	player->AddComponent(new Animator());
	player->AddComponent(new Player(18, 1.2));

	player->GetComponent<Animator>()->Play("Idle", 2.0f);
	player->GetComponent<Collider>()->offset = Vec3(0, 1.5, 0);
	player->GetComponent<MeshRenderer>()->is_shadow_from_above = true;

	auto runSmokeEmitter = new Entity("Run Smoke Emitter");
	runSmokeEmitter->AddComponent(new ParticleEmitter(runSmokeProp));

	auto jumpSmokeEmitter = new Entity("Jump Smoke Emitter");
	jumpSmokeEmitter->AddComponent(new ParticleEmitter(jumpSmokeProp));

	auto circleSmokeEmitter = new Entity("Circle Smoke Emitter");
	circleSmokeEmitter->AddComponent(new ParticleEmitter(circleSmokeProp));

	player->AddChild(runSmokeEmitter);
	player->AddChild(jumpSmokeEmitter);
	player->AddChild(circleSmokeEmitter);
	CreateEntity(player);


	auto camera = new Entity("Camera");
	camera->AddComponent(new Camera());
	camera->AddComponent(new GameCamera(player));
	CreateEntity(camera);
	SetMainCamera(camera);


	std::vector<Entity*> objects;
	std::shared_ptr sphereModel = SphereMesh::Load(1, 0.72, 0, 0);
	std::shared_ptr capsuleModel = CapsuleMesh::Load(2, 2, 0.72, 0, 0);

	for (int i = 0; i < 20; i++)
	{
		std::shared_ptr<Model> model;
		auto object = new Entity("Object " + std::to_string(i + 1));
		/*
		if (i % 2 == 1)
		{
			object->AddComponent(new CapsuleCollider({ 2, 2 }));
			model = capsuleModel;
		}
		else
		{*/
			object->AddComponent(new SphereCollider(1));
			model = sphereModel;
		//}
		
		object->AddComponent(new MeshRenderer(model));
		object->AddComponent(new Rigidbody(0.5, true, false, 0.1));
		CreateEntity(object);
		objects.push_back(object);

		auto pos = Vec3(frand(mt) * 20, frand(mt) * 10 + 10, frand(mt) * 20);
		//object->transform->position = Vec3(10 * (i + 1), 10, 0);
		object->transform->position = pos;
	}


	movingObj = new Entity("Rift");
	movingObj->AddComponent(new MeshRenderer(LoadResource<Model>(L"Assets/rift.obj")));
	movingObj->AddComponent(new MeshCollider(LoadResource<CollisionModel>(L"Assets/rift.obj")));
	movingObj->AddComponent(new Rigidbody(5, false, true, 0.1));

	movingObj->transform->position = Vec3(0, -10, 25);
	movingObj->transform->scale = Vec3(10, 10, 10);
	movingObj->GetComponent<Collider>()->scale = movingObj->transform->scale;
	CreateEntity(movingObj);


	enemy = new Entity("Enemy");
	enemy->AddComponent(new MeshRenderer(LoadResource<Model>(L"Assets/PlatformerPack/Enemy.gltf")));
	enemy->AddComponent(new SphereCollider(1.5));
	enemy->AddComponent(new Rigidbody(0.5, true, false, 0.1));
	enemy->AddComponent(new Animator());

	enemy->GetComponent<Collider>()->offset = Vec3(0, 1.5, 0);
	enemy->transform->position = Vec3(0, 0, 5);
	enemy->transform->scale = Vec3(2, 2, 2);
	CreateEntity(enemy);
	enemy->GetComponent<Animator>()->Play("Walk", 2.0f);


	/*auto pbrEntity = new Entity("PBR Entity");
	pbrEntity->AddComponent(new MeshRenderer(LoadResource<Model>(L"Assets/DamagedHelmet.glb")));
	pbrEntity->transform->position = Vec3(0, -3, -13);
	pbrEntity->transform->scale = Vec3(5, 5, 5);
	CreateEntity(pbrEntity);*/


	for (int i = 0; i < 0; i++)
	{
		auto pos = Vec3(frand(mt) * 20, -6, frand(mt) * 20);
		CreateEntity(new Coin("Coin" + std::to_string(i)))->transform->position = pos;
	}
	

	auto map = new Entity("Map", "map");
	map->AddComponent(new MeshRenderer(LoadResource<Model>(L"Assets/Map/level1.obj")));
	map->AddComponent(new MeshCollider(LoadResource<CollisionModel>(L"Assets/Map/level1.obj")));
	map->AddComponent(new Rigidbody(1, false, true, 0.1));

	map->transform->scale = Vec3(0.1, 0.1, 0.1);
	map->transform->position = Vec3(0, -10, 0);
	map->GetComponent<Collider>()->scale = map->transform->scale;
	map->GetComponent<MeshRenderer>()->SetOutlineWidth(0);
	CreateEntity(map);


	auto map_loader = new Entity("Map Loader");
	map_loader->AddComponent(new MapLoader(L"Assets/Map/level1.map"));

	map_loader->transform->scale = Vec3(0.1, 0.1, 0.1);
	map_loader->transform->position = Vec3(0, -10, 0);
	CreateEntity(map_loader);


	{
		Control* control;

		auto panel = new Entity();
		panel->AddComponent(new Panel(Vec2(-120, 80), Vec2(130, 10), Vec2(0.5, 0.5), Vec2(1, 0), Color(1, 1, 1, 0.5)));
		control = panel->GetComponent<Control>();
		control->SetRotation(4);
		
		{
			auto coin_label = new Entity("Coin Label");
			coin_label->AddComponent(new Label("023", "Koruri", 34, Label::FontWeight::EXTRA_BOLD, Color(1, 1, 1)));
			control = coin_label->GetComponent<Control>();
			control->SetTransform(Vec2(50, 0), Vec2(100, 50), Vec2(0.5, 1), Vec2(0.5, 0.5));

			coin_label->SetParent(panel);
		}

		CreateEntity(panel);
	}
	
	return true;
}

void CollisionTestScene::Update(const float delta_time)
{
	Scene::Update(delta_time);

	/*auto rigidbody = enemy->GetComponent<Rigidbody>();

	if (enemy->transform->position.x > 5)
	{
		direction = Vec3(-1, 0, 0);
	}
	else if (enemy->transform->position.x < -5)
	{
		direction = Vec3(1, 0, 0);
	}

	direction.z = -(enemy->transform->position.z - 10) * 0.3;

	rigidbody->velocity += direction * 0.01;*/

	auto rigidbody = movingObj->GetComponent<Rigidbody>();
	
	//rigidbody->velocity.x = sin(angle) * 20 - rigidbody->transform->position.x;
	//rigidbody->velocity.y = cos(angle) * 20 - rigidbody->transform->position.y;
	if (rigidbody->transform->position.y < -13)
	{
		rigidbody->velocity.y *= -60 * delta_time;
		rigidbody->transform->position.y = -13;
	}
	if (rigidbody->transform->position.y > 7)
	{
		rigidbody->velocity.y *= -60 * delta_time;
		rigidbody->transform->position.y = 7;
	}
	if (rigidbody->velocity.y >= 0)
	{
		rigidbody->velocity.y += 1.2 * delta_time;
	}
	else
	{
		rigidbody->velocity.y = -6.0 * delta_time;
	}
	//rigidbody->velocity.x += (0 - movingObj->transform->position.x) * 1.8 * delta_time;
	//rigidbody->velocity.y += (-10 - movingObj->transform->position.y) * 0.03;
	//rigidbody->velocity.z += (25 - movingObj->transform->position.z) * 1.8 * delta_time;


	if (Input::GetKeyDown(DIK_R))
	{
		Game::Get()->LoadScene(new CollisionTestScene());
	}

	if (Input::GetKeyDown(DIK_F))
	{
		Game::Get()->ToggleFullscreen();
	}
}
