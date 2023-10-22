#include "App.h"
#include "MainScene.h"

App::App()
{
	SetWindowSize(800, 600);
	SetWindowTitle(L"DirectX12 Game");
}

void App::Init()
{
	Game::Init();

	auto scene = LoadScene(new MainScene());
}
