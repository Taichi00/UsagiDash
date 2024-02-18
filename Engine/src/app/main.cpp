#include "game/game.h"
#include "game/game_settings.h"
#include "app/collision_test_scene.h"
#include "app/main_scene.h"

int wmain(int argc, wchar_t** argv, wchar_t** envp)
{
	auto game = Game::Get();

	GameSettings settings;
	settings.window_width = 1280;
	settings.window_height = 720;
	settings.title = L"DirectX12 Game";
	settings.font_files = {
		L"Assets/font/Koruri-Light.ttf",
		L"Assets/font/Koruri-Regular.ttf",
		L"Assets/font/Koruri-Bold.ttf",
		L"Assets/font/Koruri-Semibold.ttf",
		L"Assets/font/Koruri-Extrabold.ttf",
	};

	game->Run(new CollisionTestScene(), settings);

	return 0;
}