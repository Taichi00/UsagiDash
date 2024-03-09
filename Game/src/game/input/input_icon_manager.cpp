#include "input_icon_manager.h"
#include "game/game.h"
#include "util/string_methods.h"

InputIconManager::InputIconManager()
{
	resource_manager_ = Game::Get()->GetResourceManager();
}

void InputIconManager::Update()
{
	InputIconType icon_type = KEYBOARD;

	auto input_type = Input::CurrentInputType();

	// ゲームパッドの場合はメーカーによってアイコンを変える
	if (input_type == Input::GAMEPAD)
	{
		auto pad_type = Input::GetGamepadType();
		switch (pad_type)
		{
		case Input::XBOX_CONTROLLER:
		case Input::UNKNOWN:
			icon_type = PAD_XBOX;
			break;
		case Input::SWITCH_PRO_CONTROLLER:
			icon_type = PAD_NSW;
			break;
		case Input::DUALSHOCK4:
		case Input::DUALSENSE:
			icon_type = PAD_PS;
			break;
		}
	}

	// 現在のアイコン種別と同じならそのまま
	if (icon_type == current_icon_type_)
		return;

	// 現在の入力種別の画像をダミーリソースにセットする
	for (const auto& map : action_dummy_map_)
	{
		auto& action = map.first;
		auto& dummy = map.second;

		if (action_image_map_[action].contains(icon_type))
		{
			auto& image = action_image_map_[action][icon_type];
			dummy->SetData(image->Data());
			dummy->SetSize(image->Size());
		}
	}

	current_icon_type_ = icon_type;
}

void InputIconManager::AddInputIcon(const std::string& action, const std::vector<InputIconInfo>& infos)
{
	for (auto& info : infos)
	{
		auto bitmap = resource_manager_->Load<Bitmap>(info.path);

		action_image_map_[action][info.type] = bitmap;
	}

	if (!action_dummy_map_.contains(action))
	{
		auto dummy = std::make_shared<Bitmap>();

		// ダミーのリソースを追加する
		resource_manager_->Add<Bitmap>(StringMethods::GetWideString("input_icon_" + action), dummy);
		dummy->SetName(StringMethods::GetWideString("input_" + action));

		auto& image = action_image_map_[action][current_icon_type_];
		dummy->SetData(image->Data());
		dummy->SetSize(image->Size());

		action_dummy_map_[action] = dummy;
	}
}
