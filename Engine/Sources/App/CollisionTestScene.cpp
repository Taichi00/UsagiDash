#include "CollisionTestScene.h"
#include "SphereMesh.h"
#include "MeshRenderer.h"
#include "Entity.h"
#include "Camera.h"
#include "GameCamera.h"
#include "FloatingPlayer.h"
#include "Animation.h"
#include "SharedStruct.h"
#include "AssimpLoader.h"
#include "SphereCollider.h"
#include "Rigidbody.h"
#include "FloorCollider.h"
#include "Animator.h"
#include "Player.h"
#include "MeshCollider.h"
#include "CapsuleMesh.h"
#include "CapsuleCollider.h"
#include "Physics.h"
#include "Quaternion.h"
#include "BillboardRenderer.h"
#include "Texture2D.h"
#include "ParticleEmitter.h"

Entity* enemy, *testSphere;
float angle = 0;
Vec3 direction = Vec3(1, 0, 0);

bool CollisionTestScene::Init()
{
	Scene::Init();

	Model model;
	CollisionModel collisionModel;
	std::vector<Animation*> animations;
	Collider* collider;

	Model playerModel;
	std::vector<Animation*> playerAnimations;
	AssimpLoader::Load(L"Assets/PlatformerPack/Character.gltf", playerModel, playerAnimations);

	float pbrColor[4] = { 0, 1, 0, 1 };
	auto smokeAlbedo = Texture2D::Get("Assets/smoke2_albedo.png");
	auto smokeNormal = Texture2D::Get("Assets/smoke2_normal.png");
	auto smokePbr = Texture2D::GetMono(pbrColor);

	auto runSmokeProp = ParticleEmitterProperty{};
	runSmokeProp.timeToLive = 50;
	runSmokeProp.timeToLiveRange = 10;
	runSmokeProp.spawnRate = 0.2;
	runSmokeProp.spawnRateRange = 0;
	runSmokeProp.positionType = PARTICAL_PROP_TYPE_EASING;
	runSmokeProp.positionEasing.type = PARTICLE_EASE_LINEAR;
	runSmokeProp.positionEasing.start = Vec3(0, 0.3, -1);
	runSmokeProp.positionEasing.startRange = Vec3(0.5, 0.1, 0.5);
	runSmokeProp.positionEasing.end = Vec3(0, 2, -2);
	runSmokeProp.positionEasing.endRange = Vec3(0.5, 0.8, 0.5);
	/*runSmokeProp.positionPVA.position = Vec3(0, 0.7, -1);
	runSmokeProp.positionPVA.positionRange = Vec3(0.5, 0.1, 0.2);
	runSmokeProp.positionPVA.velocity = Vec3(0, 0.01, -0.05);
	runSmokeProp.positionPVA.velocityRange = Vec3(0.01, 0.01, 0.01);*/
	runSmokeProp.rotationPVA.rotationRange = Vec3(0, 0, 3.14);
	runSmokeProp.rotationPVA.velocityRange = Vec3(0, 0, 0.1);
	runSmokeProp.scaleType = PARTICAL_PROP_TYPE_EASING;
	runSmokeProp.scaleEasing.type = PARTICLE_EASE_OUT_CUBIC;
	runSmokeProp.scaleEasing.keepAspect = true;
	runSmokeProp.scaleEasing.middleEnabled = true;
	runSmokeProp.scaleEasing.start = Vec3(0.1, 0.1, 0.1);
	runSmokeProp.scaleEasing.middle = Vec3(0.5, 0.5, 0.5);
	runSmokeProp.scaleEasing.middleRange = Vec3(0.1, 0.1, 0.1);
	runSmokeProp.scaleEasing.end = Vec3(0, 0, 0);
	runSmokeProp.sprite.albedoTexture = smokeAlbedo;
	runSmokeProp.sprite.normalTexture = smokeNormal;
	runSmokeProp.sprite.pbrTexture = smokePbr;

	auto jumpSmokeProp = ParticleEmitterProperty{};
	jumpSmokeProp.spawnCount = 5;
	jumpSmokeProp.timeToLive = 50;
	jumpSmokeProp.spawnRate = 0.5;
	jumpSmokeProp.positionPVA.position = Vec3(0, 0.5, 0);
	jumpSmokeProp.positionPVA.velocityRange = Vec3(0.01, 0.01, 0.01);
	jumpSmokeProp.rotationPVA.rotationRange = Vec3(0, 0, 3.14);
	jumpSmokeProp.rotationPVA.velocityRange = Vec3(0, 0, 0.1);
	jumpSmokeProp.scaleType = PARTICAL_PROP_TYPE_EASING;
	jumpSmokeProp.scaleEasing.type = PARTICLE_EASE_LINEAR;
	jumpSmokeProp.scaleEasing.start = Vec3(0.6, 0.6, 0.6);
	jumpSmokeProp.scaleEasing.end = Vec3(0, 0, 0);
	jumpSmokeProp.sprite.albedoTexture = smokeAlbedo;
	jumpSmokeProp.sprite.normalTexture = smokeNormal;
	jumpSmokeProp.sprite.pbrTexture = smokePbr;

	auto circleSmokeProp = ParticleEmitterProperty{};
	circleSmokeProp.spawnCount = 5;
	circleSmokeProp.spawnRate = 5;
	circleSmokeProp.timeToLive = 35;
	circleSmokeProp.timeToLiveRange = 15;
	circleSmokeProp.spawningMethod = PARTICAL_SPAWN_METHOD_CIRCLE;
	circleSmokeProp.spawningCircle.radius = 0.5;
	circleSmokeProp.spawningCircle.vertices = 5;
	circleSmokeProp.positionType = PARTICAL_PROP_TYPE_EASING;
	circleSmokeProp.positionEasing.type = PARTICLE_EASE_OUT_CUBIC;
	circleSmokeProp.positionEasing.start = Vec3(0, 0.3, 0);
	circleSmokeProp.positionEasing.end = Vec3(2, 0.8, 0);
	circleSmokeProp.positionEasing.endRange = Vec3(0.1, 0.1, 0.1);
	circleSmokeProp.rotationPVA.rotationRange = Vec3(0, 0, 3.14);
	circleSmokeProp.rotationPVA.velocityRange = Vec3(0, 0, 0.1);
	circleSmokeProp.scaleType = PARTICAL_PROP_TYPE_EASING;
	circleSmokeProp.scaleEasing.type = PARTICLE_EASE_OUT_CUBIC;
	circleSmokeProp.scaleEasing.keepAspect = true;
	circleSmokeProp.scaleEasing.middleEnabled = true;
	circleSmokeProp.scaleEasing.start = Vec3(0.1, 0.1, 0.1);
	circleSmokeProp.scaleEasing.middle = Vec3(0.4, 0.4, 0.4);
	//circleSmokeProp.scaleEasing.middleRange = Vec3(0.1, 0.1, 0.1);
	circleSmokeProp.scaleEasing.end = Vec3(0, 0, 0);
	circleSmokeProp.sprite.albedoTexture = smokeAlbedo;
	circleSmokeProp.sprite.normalTexture = smokeNormal;
	circleSmokeProp.sprite.pbrTexture = smokePbr;

	auto player = new Entity("Player");
	player->AddComponent(new MeshRenderer({ playerModel }));
	collider = (Collider*)player->AddComponent(new CapsuleCollider({ 1, 1 }));
	player->AddComponent(new Rigidbody({ collider, 1, true, false, 0.1 }));
	player->AddComponent(new Animator({ playerAnimations }));
	player->AddComponent(new Player({ 0.27, 0.02 }));
	CreateEntity(player);

	player->GetComponent<Animator>()->Play("Idle", 2.0f);
	collider->offset = Vec3(0, 1.5, 0);
	player->GetComponent<MeshRenderer>()->isShadowFromAbove = true;

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
	auto sphereModel = SphereMesh::Load(2, 0.72, 0, 0);
	auto capsuleModel = CapsuleMesh::Load(2, 2, 0.72, 0, 0);

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
		
		object->AddComponent(new MeshRenderer({ model }));
		object->AddComponent(new Rigidbody({ collider, 1, true, false, 0.5 }));
		CreateEntity(object);
		objects.push_back(object);

		object->transform->position = Vec3(4 * (i + 1), 0, 0);
	}


	AssimpLoader::Load(L"Assets/PlatformerPack/Enemy.gltf", model, animations);
	enemy = new Entity("Enemy");
	enemy->AddComponent(new MeshRenderer({ model }));
	collider = (Collider*)enemy->AddComponent(new SphereCollider({ 1.5 }));
	enemy->AddComponent(new Rigidbody({ collider, 1, true, false, 0.1 }));
	enemy->AddComponent(new Animator({ animations }));
	CreateEntity(enemy);

	enemy->GetComponent<Animator>()->Play("Walk", 2.0f);
	collider->offset = Vec3(0, 1.5, 0);
	enemy->transform->position = Vec3(0, 0, 5);
	enemy->transform->scale = Vec3(2, 2, 2);


	AssimpLoader::Load(L"Assets/PlatformerPack/Star.gltf", model, animations);
	testSphere = new Entity("Star");
	testSphere->AddComponent(new MeshRenderer({ model }));
	CreateEntity(testSphere);
	//testSphere->transform->scale = Vec3(0.3, 0.3, 0.3);


	AssimpLoader::Load(L"Assets/dog.glb", model, animations);
	auto pbrEntity = new Entity("PBR Entity");
	pbrEntity->AddComponent(new MeshRenderer({ model }));
	CreateEntity(pbrEntity);
	pbrEntity->transform->position = Vec3(0, -3, -13);
	//pbrEntity->transform->rotation = Quaternion::FromEuler(1.57, 0, 0);
	pbrEntity->transform->scale = Vec3(3, 3, 3);


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


	//AssimpLoader::Load(L"Assets/checker_plane.obj", model, animations);
	//auto plane = new Entity();
	//plane->AddComponent(new MeshRenderer({ model }));
	//collider = plane->AddComponent(new FloorCollider());
	//plane->AddComponent(new Rigidbody({ (Collider*)collider, 1, false, true, 0.5 }));
	//CreateEntity(plane);

	//plane->transform->scale = Vec3(2, 2, 2);
	//plane->transform->position = Vec3(0, -2, 0);
	//plane->GetComponent<MeshRenderer>()->SetOutlineWidth(0);

	AssimpLoader::Load(L"Assets/Map/CollisionTest.obj", model, animations);
	AssimpLoader::LoadCollision(L"Assets/Map/CollisionTest.obj", collisionModel);
	auto plane = new Entity("Map");
	plane->AddComponent(new MeshRenderer({ model }));
	collider = (Collider*)plane->AddComponent(new MeshCollider({ collisionModel }));
	plane->AddComponent(new Rigidbody({ collider, 1, false, true, 0.5 }));
	CreateEntity(plane);

	plane->transform->scale = Vec3(0.1, 0.1, 0.1);
	plane->transform->position = Vec3(0, -10, 0);
	plane->GetComponent<MeshRenderer>()->SetOutlineWidth(0);

	
	/*AssimpLoader::Load(L"Assets/mikuNT.glb", model, animations);
	auto miku = new Entity();
	miku->AddComponent(new MeshRenderer({ model }));
	collider = (Collider*)miku->AddComponent(new CapsuleCollider({ 2, 6 }));
	miku->AddComponent(new Rigidbody({ collider, 1, true, false, 0.5 }));
	CreateEntity(miku);

	miku->transform->scale = Vec3(5, 5, 5);
	miku->transform->position = Vec3(0, 0, 5);
	collider->offset = Vec3(0, 5, 0);
	miku->GetComponent<MeshRenderer>()->SetOutlineWidth(0);*/


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
}
