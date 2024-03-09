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
	enum InputIconType
	{
		KEYBOARD,
		PAD_XBOX,
		PAD_NSW,
		PAD_PS,
	};

	struct InputIconInfo
	{
		InputIconType type; // �A�C�R�����
		std::wstring path; // �A�C�R���摜�̃p�X
	};

	struct InputIconImage
	{
		InputIconType type;
		std::shared_ptr<Bitmap> bitmap; // �A�C�R���摜
	};

	InputIconManager();
	~InputIconManager() {}

	void Update();

	void AddInputIcon(const std::string& action, const std::vector<InputIconInfo>& infos);

private:
	ResourceManager* resource_manager_;

	InputIconType current_icon_type_ = KEYBOARD;

	std::unordered_map<std::string, std::unordered_map<InputIconType, std::shared_ptr<Bitmap>>> action_image_map_;
	std::unordered_map<std::string, std::shared_ptr<Bitmap>> action_dummy_map_;
};