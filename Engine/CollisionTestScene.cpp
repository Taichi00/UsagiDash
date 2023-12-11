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

	/*model = CapsuleMesh::Load(2, 2, 1, 0.3, 0.3);
	auto player = new Entity();
	player->AddComponent(new MeshRenderer({ model }));
	collider = player->AddComponent(new CapsuleCollider({ 2, 2 }));
	player->AddComponent(new Rigidbody({(Collider*)collider, 1, true, false, 0.1 }));
	player->AddComponent(new FloatingPlayer({ 0.15, 0.02 }));
	CreateEntity(player);*/

	AssimpLoader::Load(L"Assets/PlatformerPack/Character.gltf", model, animations);
	auto player = new Entity();
	player->AddComponent(new MeshRenderer({ model }));
	collider = (Collider*)player->AddComponent(new CapsuleCollider({ 1, 1 }));
	player->AddComponent(new Rigidbody({ collider, 1, true, false, 0.1 }));
	player->AddComponent(new Animator({ animations }));
	player->AddComponent(new Player({ 0.22, 0.02 }));
	CreateEntity(player);

	player->GetComponent<Animator>()->Play("Idle", 2.0f);
	collider->offset = Vec3(0, 1.5, 0);


	std::vector<Entity*> objects;
	auto sphereModel = SphereMesh::Load(2, 0.1, 0.1, 0.1);
	auto capsuleModel = CapsuleMesh::Load(2, 2, 0.8, 0.3, 0.2);

	for (int i = 0; i < 4; i++)
	{
		auto object = new Entity();
		
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
	enemy = objects[0];


	AssimpLoader::Load(L"Assets/PlatformerPack/Star.gltf", model, animations);
	testSphere = new Entity();
	testSphere->AddComponent(new MeshRenderer({ model }));
	CreateEntity(testSphere);
	//testSphere->transform->scale = Vec3(0.3, 0.3, 0.3);


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
	auto plane = new Entity();
	plane->AddComponent(new MeshRenderer({ model }));
	collider = (Collider*)plane->AddComponent(new MeshCollider({ collisionModel }));
	plane->AddComponent(new Rigidbody({ collider, 1, false, true, 0.5 }));
	CreateEntity(plane);

	plane->transform->scale = Vec3(0.1, 0.1, 0.1);
	plane->transform->position = Vec3(0, -10, 0);
	plane->GetComponent<MeshRenderer>()->SetOutlineWidth(0);

	/*
	AssimpLoader::Load(L"Assets/mikuNT.glb", model, animations);
	auto miku = new Entity();
	miku->AddComponent(new MeshRenderer({ model }));
	collider = (Collider*)miku->AddComponent(new CapsuleCollider({ 2, 6 }));
	miku->AddComponent(new Rigidbody({ collider, 1, true, false, 0.5 }));
	CreateEntity(miku);

	miku->transform->scale = Vec3(5, 5, 5);
	miku->transform->position = Vec3(0, 0, 5);
	collider->offset = Vec3(0, 5, 0);
	miku->GetComponent<MeshRenderer>()->SetOutlineWidth(0);*/


	auto camera = new Entity();
	camera->AddComponent(new Camera());
	camera->AddComponent(new GameCamera({ player }));
	CreateEntity(camera);
	SetMainCamera(camera);

	SetSkybox(L"Assets/puresky.dds");

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
