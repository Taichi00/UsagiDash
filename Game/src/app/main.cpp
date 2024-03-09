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
	settings.button_actions["menu"] = {
		{ Input::InputType::KEYBOARD, Input::Button::KEY_ESCAPE },
		{ Input::InputType::GAMEPAD, Input::Button::PAD_START }
	};

	// ボタンアイコンの設定
	settings.action_icons["jump"] = {
		{ InputIconManager::KEYBOARD, L"assets/image/button_icon/keyboard/keyboard_space.png" },
		{ InputIconManager::PAD_XBOX, L"assets/image/button_icon/xbox/xbox_button_a.png" },
		{ InputIconManager::PAD_NSW,  L"assets/image/button_icon/switch/switch_button_a.png" },
		{ InputIconManager::PAD_PS,   L"assets/image/button_icon/playstation/playstation_button_circle.png" },
	};
	settings.action_icons["crouch"] = {
		{ InputIconManager::KEYBOARD, L"assets/image/button_icon/keyboard/keyboard_shift.png" },
		{ InputIconManager::PAD_XBOX, L"assets/image/button_icon/xbox/xbox_lt.png" },
		{ InputIconManager::PAD_NSW,  L"assets/image/button_icon/switch/switch_button_zl.png" },
		{ InputIconManager::PAD_PS,   L"assets/image/button_icon/playstation/playstation_trigger_l2.png" },
	};
	settings.action_icons["move"] = {
		{ InputIconManager::KEYBOARD, L"assets/image/button_icon/keyboard/keyboard_arrows2.png" },
		{ InputIconManager::PAD_XBOX, L"assets/image/button_icon/xbox/xbox_stick_l.png" },
		{ InputIconManager::PAD_NSW,  L"assets/image/button_icon/switch/switch_stick_l.png" },
		{ InputIconManager::PAD_PS,   L"assets/image/button_icon/playstation/playstation_stick_l.png" },
	};
	settings.action_icons["camera"] = {
		{ InputIconManager::KEYBOARD, L"assets/image/button_icon/keyboard/keyboard_wasd.png" },
		{ InputIconManager::PAD_XBOX, L"assets/image/button_icon/xbox/xbox_stick_r.png" },
		{ InputIconManager::PAD_NSW,  L"assets/image/button_icon/switch/switch_stick_r.png" },
		{ InputIconManager::PAD_PS,   L"assets/image/button_icon/playstation/playstation_stick_r.png" },
	};
	settings.action_icons["ok"] = {
		{ InputIconManager::KEYBOARD, L"assets/image/button_icon/keyboard/keyboard_space.png" },
		{ InputIconManager::PAD_XBOX, L"assets/image/button_icon/xbox/xbox_button_a.png" },
		{ InputIconManager::PAD_NSW,  L"assets/image/button_icon/switch/switch_button_a.png" },
		{ InputIconManager::PAD_PS,   L"assets/image/button_icon/playstation/playstation_button_circle.png" },
	};
	settings.action_icons["menu"] = {
		{ InputIconManager::KEYBOARD, L"assets/image/button_icon/keyboard/keyboard_escape.png" },
		{ InputIconManager::PAD_XBOX, L"assets/image/button_icon/xbox/xbox_button_menu.png" },
		{ InputIconManager::PAD_NSW,  L"assets/image/button_icon/switch/switch_button_plus.png" },
		{ InputIconManager::PAD_PS,   L"assets/image/button_icon/playstation/playstation_button_options.png" },
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