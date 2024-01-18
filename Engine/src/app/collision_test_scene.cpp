#include "app/collision_test_scene.h"
#include "game/component/sphere_mesh.h"
#include "game/component/mesh_renderer.h"
#include "game/entity.h"
#include "game/component/camera.h"
#include "app/game_camera.h"
#include "app/floating_player.h"
#include "game/animation.h"
#include "engine/shared_struct.h"
#include "game/assimp_loader.h"
#include "game/component/collider/sphere_collider.h"
#include "game/component/rigidbody.h"
#include "game/component/collider/floor_collider.h"
#include "game/component/animator.h"
#include "app/player.h"
#include "game/component/collider/mesh_collider.h"
#include "game/component/capsule_mesh.h"
#include "game/component/collider/capsule_collider.h"
#include "game/physics.h"
#include "math/quaternion.h"
#include "game/component/billboard_renderer.h"
#include "engine/texture2d.h"
#include "game/component/particle_emitter.h"
#include "game/input.h"
#include "game/game.h"
#include "app/main_scene.h"
#include "game/label.h"
#include <memory>

Entity* enemy, *testSphere, *movingObj;
float angle = 0;
Vec3 direction = Vec3(1, 0, 0);

bool CollisionTestScene::Init()
{
	Scene::Init();

	std::shared_ptr<Model> model;
	CollisionModel collisionModel;
	std::vector<Animation*> animations;
	Collider* collider;

	auto playerModel = LoadResource<Model>("Assets/PlatformerPack/Character.gltf");

	float pbrColor[4] = { 0, 1, 0, 1 };
	std::shared_ptr smokeAlbedo = Texture2D::Load("Assets/smoke2_albedo.png");
	std::shared_ptr smokeNormal = Texture2D::Load("Assets/smoke2_normal.png");
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


	auto player = new Entity("Player");
	player->AddComponent(new MeshRenderer(playerModel));
	collider = (Collider*)player->AddComponent(new CapsuleCollider({ 1, 1 }));
	player->AddComponent(new Rigidbody({ collider, 1, true, false, 0.1 }));
	player->AddComponent(new Animator());
	player->AddComponent(new Player({ 0.27, 0.02 }));
	CreateEntity(player);

	player->GetComponent<Animator>()->Play("Idle", 2.0f);
	collider->offset = Vec3(0, 1.5, 0);
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


	std::vector<Entity*> objects;
	std::shared_ptr sphereModel = SphereMesh::Load(2, 0.72, 0, 0);
	std::shared_ptr capsuleModel = CapsuleMesh::Load(2, 2, 0.72, 0, 0);

	for (int i = 0; i < 4; i++)
	{
		auto object = new Entity("Object " + std::to_string(i + 1));
		
		if (i % 2 == 0)
		{
			collider = (Collider*)object->AddComponent(new CapsuleCollider({ 2, 2 }));
			model = capsuleModel;
		}
		else
		{
			collider = (Collider*)object->AddComponent(new SphereCollider({ 2 }));
			model = sphereModel;
		}
		
		object->AddComponent(new MeshRenderer(model));
		object->AddComponent(new Rigidbody({ collider, 1, true, false, 0.5 }));
		CreateEntity(object);
		objects.push_back(object);

		object->transform->position = Vec3(4 * (i + 1), 0, 0);
	}

	//sphereModel = SphereMesh::Load(10, 0.72, 0, 0);
	AssimpLoader::LoadCollision(L"Assets/rift.obj", collisionModel);
	movingObj = new Entity("Rift");
	movingObj->AddComponent(new MeshRenderer(LoadResource<Model>("Assets/rift.obj")));
	collider = (Collider*)movingObj->AddComponent(new MeshCollider({ collisionModel }));
	movingObj->AddComponent(new Rigidbody({ collider, 1000, false, true, 0.5 }));
	CreateEntity(movingObj);
	movingObj->transform->position = Vec3(0, -10, -60);
	movingObj->transform->scale = Vec3(10, 10, 10);


	enemy = new Entity("Enemy");
	enemy->AddComponent(new MeshRenderer(LoadResource<Model>("Assets/PlatformerPack/Enemy.gltf")));
	collider = (Collider*)enemy->AddComponent(new SphereCollider({ 1.5 }));
	enemy->AddComponent(new Rigidbody({ collider, 1, true, false, 0.1 }));
	enemy->AddComponent(new Animator());
	CreateEntity(enemy);

	enemy->GetComponent<Animator>()->Play("Walk", 2.0f);
	collider->offset = Vec3(0, 1.5, 0);
	enemy->transform->position = Vec3(0, 0, 5);
	enemy->transform->scale = Vec3(2, 2, 2);


	testSphere = new Entity("Star");
	testSphere->AddComponent(new MeshRenderer(LoadResource<Model>("Assets/PlatformerPack/Star.gltf")));
	CreateEntity(testSphere);
	//testSphere->transform->scale = Vec3(0.3, 0.3, 0.3);


	auto pbrEntity = new Entity("PBR Entity");
	pbrEntity->AddComponent(new MeshRenderer(LoadResource<Model>("Assets/DamagedHelmet.glb")));
	CreateEntity(pbrEntity);
	pbrEntity->transform->position = Vec3(0, -3, -13);
	pbrEntity->transform->scale = Vec3(5, 5, 5);


	/*auto billboard = new Entity("Billboard");
	billboard->AddComponent(new BillboardRenderer(albedo, normal));
	CreateEntity(billboard);

	billboard->transform->position = Vec3(0, -4, 0);
	billboard->transform->scale = Vec3(0.6, 0.6, 0.6);*/


	/*auto smokeProp = ParticleEmitterProperty{};
	smokeProp.spawnRate = 1;
	smokeProp.positionPVA.velocityRange = Vec3(0.3, 0.3, 0.3);
	smokeProp.rotationPVA.velocityRange = Vec3(0, 0, 0.1);
	smokeProp.scalePVA.velocity = Vec3(-0.01, -0.01, -0.01);
	smokeProp.sprite.albedoTexture = smokeAlbedo;
	smokeProp.sprite.normalTexture = smokeNormal;
	smokeProp.sprite.pbrTexture = smokePbr;

	auto smokeEmitter = new Entity("Smoke Emitter");
	smokeEmitter->AddComponent(new ParticleEmitter(smokeProp));
	CreateEntity(smokeEmitter);*/

	AssimpLoader::LoadCollision(L"Assets/Map/level1.obj", collisionModel);
	auto plane = new Entity("Map");
	plane->AddComponent(new MeshRenderer(LoadResource<Model>("Assets/Map/level1.obj")));
	collider = (Collider*)plane->AddComponent(new MeshCollider({ collisionModel }));
	plane->AddComponent(new Rigidbody({ collider, 1, false, true, 0.5 }));
	CreateEntity(plane);

	plane->transform->scale = Vec3(0.1, 0.1, 0.1);
	plane->transform->position = Vec3(0, -10, 0);
	plane->GetComponent<MeshRenderer>()->SetOutlineWidth(0);


	auto label = new Entity("Label");
	label->AddComponent(new Label("Label Entity •¶Žš—ñ‚Ì•\Ž¦", "Source Han Sans VF", 32));
	CreateEntity(label);


	auto camera = new Entity("Camera");
	camera->AddComponent(new Camera());
	camera->AddComponent(new GameCamera({ player }));
	CreateEntity(camera);
	SetMainCamera(camera);

	SetSkybox("Assets/puresky");

	return true;
}

void CollisionTestScene::Update()
{
	Scene::Update();

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
	
	rigidbody->velocity.x = (sin(angle) - sin(angle - 0.02)) * 20;
	rigidbody->velocity.y += (-10 - movingObj->transform->position.y) * 0.03;
	rigidbody->velocity.z += (-60 - movingObj->transform->position.z) * 0.03;

	auto origin = Vec3(-10, -6, 10);
	auto direction = Quaternion::FromEuler(0, angle, 0) * Vec3(1, 0, 0);
	float distance = 10;
	RaycastHit hit;

	if (Physics::Raycast(origin, direction, distance, hit))
	{
		distance = hit.distance;
	}

	testSphere->transform->position = origin + direction * distance;
	testSphere->transform->rotation = Quaternion::FromEuler(0, angle * 5, 0);

	angle += 0.02;

	if (Input::GetKeyDown(DIK_END))
	{
		Game::Get()->LoadScene(new CollisionTestScene());
	}

	if (Input::GetKeyDown(DIK_F))
	{
		Game::Get()->GetEngine()->ResizeWindow(1920, 1080);
	}

}
