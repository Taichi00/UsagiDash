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

Entity* enemy;
Vec3 direction = Vec3(1, 0, 0);

bool CollisionTestScene::Init()
{
	Scene::Init();

	Model model;
	CollisionModel collisionModel;
	std::vector<Animation*> animations;
	Component* collider;

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
	collider = player->AddComponent(new CapsuleCollider({ 1, 1 }));
	player->AddComponent(new Rigidbody({ (Collider*)collider, 1, true, false, 0.1 }));
	player->AddComponent(new Animator({ animations }));
	player->AddComponent(new Player({ 0.15, 0.02 }));
	CreateEntity(player);

	player->GetComponent<Animator>()->Play("Idle", 2.0f);
	((Collider*)collider)->offset = Vec3(0, 1.5, 0);


	std::vector<Entity*> objects;
	auto sphereModel = SphereMesh::Load(2, 0.1, 0.1, 0.1);
	auto capsuleModel = CapsuleMesh::Load(2, 2, 0.8, 0.3, 0.3);

	for (int i = 0; i < 4; i++)
	{
		auto object = new Entity();
		
		if (i % 2 == 0)
		{
			collider = object->AddComponent(new CapsuleCollider({ 2, 2 }));
			model = capsuleModel;
		}
		else
		{
			collider = object->AddComponent(new SphereCollider({ 2 }));
			model = sphereModel;
		}
		
		object->AddComponent(new MeshRenderer({ model }));
		object->AddComponent(new Rigidbody({ (Collider*)collider, 1, true, false, 0.5 }));
		CreateEntity(object);
		objects.push_back(object);

		object->transform->position = Vec3(4 * (i + 1), 0, 0);
	}
	enemy = objects[0];


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
	collider = plane->AddComponent(new MeshCollider({ collisionModel }));
	plane->AddComponent(new Rigidbody({ (Collider*)collider, 1, false, true, 0.5 }));
	CreateEntity(plane);

	plane->transform->scale = Vec3(0.1, 0.1, 0.1);
	plane->transform->position = Vec3(0, -10, 0);
	plane->GetComponent<MeshRenderer>()->SetOutlineWidth(0);


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

	auto rigidbody = enemy->GetComponent<Rigidbody>();

	if (enemy->transform->position.x > 5)
	{
		direction = Vec3(-1, 0, 0);
	}
	else if (enemy->transform->position.x < -5)
	{
		direction = Vec3(1, 0, 0);
	}

	direction.z = -(enemy->transform->position.z - 10) * 0.3;

	rigidbody->velocity += direction * 0.01;
}
