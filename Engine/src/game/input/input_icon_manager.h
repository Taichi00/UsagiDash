#pragma once

#include "game/resource/bitmap.h"
#include "game/input/input.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>

class ResourceManager;

class InputIconManager
{
public:
	struct InputIconInfo
	{
		Input::InputType type; // ���͎��
		std::wstring path; // �A�C�R���摜�̃p�X
	};

	struct InputIconImage
	{
		Input::InputType type;
		std::shared_ptr<Bitmap> bitmap; // �A�C�R���摜
	};

	InputIconManager();
	~InputIconManager() {}

	void Update();

	void AddInputIcon(const std::string& action, const std::vector<InputIconInfo>& infos);

private:
	ResourceManager* resource_manager_;

	Input::InputType current_input_type_ = Input::InputType::KEYBOARD;

	std::unordered_map<std::string, std::unordered_map<Input::InputType, std::shared_ptr<Bitmap>>> action_image_map_;
	std::unordered_map<std::string, std::shared_ptr<Bitmap>> action_dummy_map_;
};