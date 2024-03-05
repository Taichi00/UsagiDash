#include "game/game.h"
#include "game/game_settings.h"
#include "app/scene/level1_scene.h"
#include "app/scene/title_scene.h"
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
		L"assets/font/Koruri-Light.ttf",
		L"assets/font/Koruri-Regular.ttf",
		L"assets/font/Koruri-Bold.ttf",
		L"assets/font/Koruri-Semibold.ttf",
		L"assets/font/Koruri-Extrabold.ttf",
	};

	// アクションのキー設定
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

	// ボタンアイコンの設定
	settings.action_icons["jump"] = {
		{ Input::InputType::KEYBOARD, L"assets/image/button_icon/keyboard_space.png" },
		{ Input::InputType::GAMEPAD, L"assets/image/button_icon/xbox_button_a.png" }
	};
	settings.action_icons["crouch"] = {
		{ Input::InputType::KEYBOARD, L"assets/image/button_icon/keyboard_shift.png" },
		{ Input::InputType::GAMEPAD, L"assets/image/button_icon/xbox_lt.png" }
	};
	settings.action_icons["move"] = {
		{ Input::InputType::KEYBOARD, L"assets/image/button_icon/keyboard_arrows2.png" },
		{ Input::InputType::GAMEPAD, L"assets/image/button_icon/xbox_stick_l.png" }
	};
	settings.action_icons["camera"] = {
		{ Input::InputType::KEYBOARD, L"assets/image/button_icon/keyboard_wasd.png" },
		{ Input::InputType::GAMEPAD, L"assets/image/button_icon/xbox_stick_r.png" }
	};

	// レイヤーの設定
	settings.layers = {
		"player",
		"object",
		"map",
		"item",
		"event",
	};
	settings.collision_table = {
		{ "player", "map", true },
		{ "player", "object", true },
		{ "player", "item", true },
		{ "player", "event", true },
		{ "object", "map", true },
		{ "object", "object", true },
	};

	Game::Create();

	//Game::Get()->Run(new Level1Scene(), settings);
	Game::Get()->Run(new TitleScene(), settings);

	Game::Destroy();

	CoUninitialize();

	return 0;
}