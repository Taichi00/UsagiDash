#include "input_icon_manager.h"
#include "game/game.h"
#include "util/string_methods.h"

InputIconManager::InputIconManager()
{
	resource_manager_ = Game::Get()->GetResourceManager();
}

void InputIconManager::Update()
{
	auto input_type = Input::CurrentInputType();
	if (input_type == current_input_type_)
		return;

	// 現在の入力種別の画像をダミーリソースにセットする
	for (const auto& map : action_dummy_map_)
	{
		auto& action = map.first;
		auto& dummy = map.second;

		if (action_image_map_[action].contains(input_type))
		{
			auto& image = action_image_map_[action][input_type];
			dummy->SetData(image->Data());
			dummy->SetSize(image->Size());
		}
	}

	current_input_type_ = input_type;
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

		auto& image = action_image_map_[action][current_input_type_];
		dummy->SetData(image->Data());
		dummy->SetSize(image->Size());

		action_dummy_map_[action] = dummy;
	}
}
