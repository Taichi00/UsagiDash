#include "app/app.h"
#include "app/main_scene.h"
#include "app/collision_test_scene.h"


void App::Init()
{
	SetWindowSize(1280, 720);
	//SetWindowSize(1920, 1080);
	SetWindowTitle(L"DirectX12 Game");

	Game::Init();

	auto scene = LoadScene(new CollisionTestScene());
}
