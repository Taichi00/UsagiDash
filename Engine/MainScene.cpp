#include "MainScene.h"
#include "SharedStruct.h"
#include "Animation.h"
#include "AssimpLoader.h"
#include "Entity.h"
#include "MeshRenderer.h"
#include "Animator.h"
#include "SphereMesh.h"
#include "Camera.h"

#include "Player.h"
#include "GameCamera.h"

bool MainScene::Init()
{
	Scene::Init();

	Model model;
	std::vector<Animation*> animations;

	AssimpLoader::Load(L"Assets/PlatformerPack/Character.gltf", model, animations);
	auto player = new Entity();
	player->AddComponent(new MeshRenderer({ model }));
	player->AddComponent(new Animator({ animations }));
	player->AddComponent(new Player({ 0.1, 0.01 }));
	CreateEntity(player);

	player->GetComponent<Animator>()->Play("Idle", 2.0f);


	AssimpLoader::Load(L"Assets/PlatformerPack/Cube_Grass_Single.gltf", model, animations);
	auto plane = new Entity();
	plane->AddComponent(new MeshRenderer({ model }));
	CreateEntity(plane);

	plane->transform->scale = Vec3(2, 2, 2);
	plane->transform->position = Vec3(0, -2, 0);


	model = SphereMesh::Load(40, 20);
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
