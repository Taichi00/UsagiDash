#include "App.h"
#include "MainScene.h"
#include "CollisionTestScene.h"


void App::Init()
{
	SetWindowSize(1280, 720);
	//SetWindowSize(1920, 1080);
	SetWindowTitle(L"DirectX12 Game");

	Game::Init();

	auto scene = LoadScene(new CollisionTestScene());
}
