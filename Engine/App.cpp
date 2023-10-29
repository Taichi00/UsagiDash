#include "App.h"
#include "MainScene.h"
#include "CollisionTestScene.h"

App::App()
{
	SetWindowSize(1280, 720);
	SetWindowTitle(L"DirectX12 Game");
}

void App::Init()
{
	Game::Init();

	auto scene = LoadScene(new CollisionTestScene());
}
