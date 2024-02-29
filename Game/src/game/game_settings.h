#pragma once

#include "game/input/input.h"
#include "game/input/input_icon_manager.h"
#include <string>
#include <vector>

struct GameSettings
{
	std::wstring title = L"Game";
	unsigned int window_width = 1280;
	unsigned int window_height = 720;
	unsigned int fullscreen_width = 1920;
	unsigned int fullscreen_height = 1080;
	std::vector<std::wstring> font_files = {};

	std::map<std::string, std::vector<Input::ButtonActionInfo>> button_actions = {};
	std::map<std::string, std::vector<Input::AxisActionInfo>> axis_actions = {};

	std::map<std::string, std::vector<InputIconManager::InputIconInfo>> action_icons = {};
};