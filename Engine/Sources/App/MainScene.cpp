#include "MainScene.h"
#include "SharedStruct.h"
#include "Animation.h"
#include "AssimpLoader.h"
#include "Entity.h"
#include "MeshRenderer.h"
#include "Animator.h"
#include "SphereMesh.h"
#include "Camera.h"
#include "ParticleEmitter.h"
#include "Texture2D.h"
#include "Rigidbody.h"
#include "CapsuleCollider.h"
#include "Collider.h"
#include "MeshCollider.h"
#include "Model.h"

#include "Player.h"
#include "GameCamera.h"

bool MainScene::Init()
{
	Scene::Init();

	std::shared_ptr<Model> model;
	std::vector<Animation*> animations;

	auto playerModel = LoadResource<Model>("Assets/PlatformerPack/Character.gltf");

	float pbrColor[4] = { 0, 1, 0, 1 };
	std::shared_ptr smokeAlbedo = Texture2D::Load("Assets/smoke2_albedo.png");
	std::shared_ptr smokeNormal = Texture2D::Load("Assets/smoke2_normal.png");
	std::shared_ptr smokePbr = Texture2D::GetMono(pbrColor);

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
	player->AddComponent(new MeshRenderer(playerModel));
	auto collider = (Collider*)player->AddComponent(new CapsuleCollider({ 1, 1 }));
	player->AddComponent(new Rigidbody({ collider, 1, true, false, 0.1 }));
	player->AddComponent(new Animator());
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


	CollisionModel collisionModel;
	AssimpLoader::LoadCollision(L"Assets/PlatformerPack/Cube_Grass_Single.gltf", collisionModel);
	auto plane = new Entity();
	plane->AddComponent(new MeshRenderer(LoadResource<Model>("Assets/PlatformerPack/Cube_Grass_Single.gltf")));
	collider = (Collider*)plane->AddComponent(new MeshCollider({ collisionModel }));
	plane->AddComponent(new Rigidbody({ collider, 1000, false, true, 0.5 }));
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
