#include "App.h"
#include "Engine.h"
#include "Entity.h"
#include "MeshEntity.h"
#include "Camera.h"
#include "Vec.h"
#include "Animator.h"
#include "MeshRenderer.h"
#include "AssimpLoader.h"
#include <DirectXMath.h>

Entity* camera;
Entity* miku, * alicia, * wolfarl, * man;
float x = 0.0f, y = 0.0f, z = 0.0f;
float cameraAngle = 0, cameraDistance = 150;
float cameraAngleV = 0, cameraDistanceV = 0;

App::App()
{
	SetWindowSize(640, 800);
	SetWindowTitle(L"DirectX12 Game");
}

void App::Init()
{
	Game::Init();

	camera = new Entity();
	camera->AddComponent(new Camera());
	CreateEntity(camera);
	camera->SetPosition(0, 130, 150);
	camera->GetComponent<Camera>()->SetFocusPosition(0, 140, 0);
	SetMainCamera(camera);


	Model model;
	std::vector<Animation*> animations;

	AssimpLoader::Load(L"Assets/Fox.glb", model, animations);
	man = new Entity();
	man->AddComponent(new MeshRenderer({ model }));
	man->AddComponent(new Animator({ animations }));
	CreateEntity(man);
	
	//man->SetScale(100, 100, 100);
	man->GetComponent<Animator>()->Play("Run", 2.0f);

	auto man2 = new Entity();
	man2->AddComponent(new MeshRenderer({ model }));
	//man2->AddComponent(new Animator({ animations }));
	CreateEntity(man2);
	man2->SetPosition(50, 0, 0);
	man2->SetRotation(0, 1.57, 0);
	//man2->GetComponent<Animator>()->Play("Walk", 1.0f);

	AssimpLoader::Load(L"Assets/checker_plane.obj", model, animations);
	auto plane = new Entity();
	plane->AddComponent(new MeshRenderer({ model }));
	CreateEntity(plane);

	plane->SetScale(100, 100, 100);

}

void App::Update()
{
	Game::Update();

	if (CheckInput(0, DIK_RIGHT))
	{
		cameraAngleV += 0.002;
	}
	else if (CheckInput(0, DIK_LEFT))
	{
		cameraAngleV += -0.002;
	}
	else
	{
		cameraAngleV *= 0.92;
	}
	cameraAngleV = min(max(cameraAngleV, -0.04), 0.04);
	cameraAngle += cameraAngleV;

	if (CheckInput(0, DIK_UP))
	{
		cameraDistanceV += -0.2;
	}
	else if (CheckInput(0, DIK_DOWN))
	{
		cameraDistanceV += 0.2;
	}
	else
	{
		cameraDistanceV *= 0.92;
	}
	cameraDistanceV = min(max(cameraDistanceV, -4), 4);
	cameraDistance += cameraDistanceV;

	//cameraAngle += 0.01;
	x = sin(cameraAngle) * cameraDistance;
	y = -10 * ((cameraDistance - 30) / 120) + 140;
	z = cos(cameraAngle) * cameraDistance;

	camera->SetPosition(x, y, z);

	y = -50 * ((cameraDistance - 30) / 120) + 140;

	camera->GetComponent<Camera>()->SetFocusPosition(0, y, 0);
	
	//miku->SetRotation(0, y, 0);
	//wolfarl->SetRotation(0, y, 0);
	//alicia->SetRotation(-3.14 / 2, 0, -cameraAngle);
}
