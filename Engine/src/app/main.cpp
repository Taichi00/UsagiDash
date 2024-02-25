#include "game/game.h"
#include "game/game_settings.h"
#include "app/collision_test_scene.h"
#include "app/main_scene.h"
#include "engine/comptr.h"

int wmain(int argc, wchar_t** argv, wchar_t** envp)
{
	// COMコンポーネントの初期化
	if (FAILED(CoInitializeEx(0, COINIT_MULTITHREADED)))
	{
		return -1;
	}

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
	settings.button_actions["jump"] = {
		{ Input::InputType::KEYBOARD, Input::Button::KEY_SPACE },
		{ Input::InputType::GAMEPAD, Input::Button::PAD_A }
	};
	settings.button_actions["crouch"] = {
		{ Input::InputType::KEYBOARD, Input::Button::KEY_LSHIFT },
		{ Input::InputType::GAMEPAD, Input::Button::PAD_LT },
		{ Input::InputType::GAMEPAD, Input::Button::PAD_RT },
	};
	settings.axis_actions["camera_horizontal"] = {
		{ Input::InputType::KEYBOARD, Input::Axis::KEY_WASD_X },
		{ Input::InputType::GAMEPAD, Input::Axis::PAD_RSTICK_X }
	};
	settings.axis_actions["camera_vertical"] = {
		{ Input::InputType::KEYBOARD, Input::Axis::KEY_WASD_Y },
		{ Input::InputType::GAMEPAD, Input::Axis::PAD_RSTICK_Y }
	};

	Game::Create();

	Game::Get()->Run(new CollisionTestScene(), settings);

	Game::Destroy();

	CoUninitialize();

	return 0;
}