#include "game/game.h"
#include "app/collision_test_scene.h"
#include "app/main_scene.h"

int wmain(int argc, wchar_t** argv, wchar_t** envp)
{
	auto game = Game::Get();

	game->SetWindowSize(1280, 720);
	//game->SetWindowSize(1920, 1080);
	game->SetWindowTitle(L"DirectX12 Game");

	game->Run(new CollisionTestScene());

	return 0;
}