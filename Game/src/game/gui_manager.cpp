#include "gui_manager.h"
#include "game/input/input.h"
#include "math/vec.h"
#include "game/component/gui/button_base.h"
#include "game/game.h"
#include "game/entity.h"
#include "engine/engine.h"
#include "engine/engine2d.h"
#include <algorithm>

GUIManager::GUIManager()
{
}

GUIManager::~GUIManager()
{
}

void GUIManager::Update()
{
	if (buttons_.empty())
	{
		current_picked_button_ = nullptr;
		prev_picked_button_ = nullptr;
		return;
	}

	UpdateTarget();

	if (current_picked_button_)
		CheckButtonPress();
}

void GUIManager::Add(Control* control)
{
	controls_.push_back(control);
}

void GUIManager::Remove(Control* control)
{
	auto it = std::find(controls_.begin(), controls_.end(), control);

	if (it != controls_.end())
	{
		controls_.erase(it);
	}
}

void GUIManager::AddButton(ButtonBase* button)
{
	buttons_.push_back(button);

	if (!current_picked_button_)
	{
		current_picked_button_ = button;
	}
}

void GUIManager::RemoveButton(ButtonBase* button)
{
	auto it = std::find(buttons_.begin(), buttons_.end(), button);

	if (it != buttons_.end())
	{
		buttons_.erase(it);

		if (current_picked_button_ == button)
			current_picked_button_ = nullptr;
		if (prev_picked_button_ == button)
			prev_picked_button_ = nullptr;
	}
}

void GUIManager::PickButton(ButtonBase* button)
{
	current_picked_button_ = button;
}

std::vector<Control*> GUIManager::GetDrawList()
{
	struct ControlInfo
	{
		Control* control = nullptr;
		int z_index = 0;

		bool operator<(const ControlInfo& info) const
		{
			return z_index < info.z_index;
		}
	};

	std::vector<ControlInfo> infos;
	for (auto control : controls_)
	{
		infos.push_back({ control, control->ZIndex() });
	}

	// ソートする
	std::sort(infos.begin(), infos.end());

	std::vector<Control*> list;
	for (auto& info : infos)
	{
		list.push_back(info.control);
	}

	return list;
}

void GUIManager::UpdateTarget()
{
	// キー入力
	Vec2 input;
	if (Input::GetButtonRepeat("up"))
	{
		input.y = -1;
	}
	else if (Input::GetButtonRepeat("down")) {
		input.y = 1;
	}
	if (Input::GetButtonRepeat("left")) {
		input.x = -1;
	}
	else if (Input::GetButtonRepeat("right")) {
		input.x = 1;
	}
	
	if (input != Vec2::Zero())
	{
		auto input_normal = input.Normalized();

		// カーソルの現在位置を取得
		Vec2 current_pos;
		if (current_picked_button_)
		{
			current_pos = current_picked_button_->WorldPosition();
		}
		else
		{
			// ボタンが選択されていなければ左上のボタンが選択されるようにする
			current_pos = Vec2(0, 0);
			input_normal = Vec2(1, 1).Normalized();
		}
		
		float min_distance = 1000000;
		ButtonBase* new_picked_button = nullptr;

		for (auto button : buttons_)
		{
			if (!button->GetEntity()->IsActive())
				continue;

			if (!button->enabled)
				continue;

			auto pos = button->WorldPosition();
			auto v = pos - current_pos;

			// 入力方向で一番近いボタンを選択する
			float dot = Vec2::Dot(v.Normalized(), input_normal);
			if (dot > 0.5f)
			{
				auto distance = v.Length();
				if (distance < min_distance)
				{
					min_distance = distance;
					new_picked_button = button;
				}
			}
		}

		if (new_picked_button)
		{
			current_picked_button_ = new_picked_button;
		}
	}

	// エンティティやコンポーネントがアクティブでなければ選択を外す
	if (current_picked_button_)
	{
		if (!current_picked_button_->enabled || !current_picked_button_->GetEntity()->IsActive())
		{
			current_picked_button_ = nullptr;
		}
	}

	// イベント通知
	if (current_picked_button_ != prev_picked_button_)
	{
		if (prev_picked_button_)
			prev_picked_button_->OnUnhovered();

		if (current_picked_button_)
			current_picked_button_->OnHovered();
	}

	prev_picked_button_ = current_picked_button_;
}

void GUIManager::CheckButtonPress()
{
	if (Input::GetButtonDown("ok"))
	{
		current_picked_button_->OnPressed();
	}
}
