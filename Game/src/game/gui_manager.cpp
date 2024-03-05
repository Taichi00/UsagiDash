#include "gui_manager.h"
#include "game/input/input.h"
#include "math/vec.h"
#include "game/component/gui/button_base.h"

GUIManager::GUIManager()
{
}

GUIManager::~GUIManager()
{
}

void GUIManager::Update()
{
	UpdateTarget();

	if (current_picked_button_)
		CheckButtonPress();
}

void GUIManager::AddButton(ButtonBase* button)
{
	buttons_.push_back(button);

	if (!current_picked_button_)
		current_picked_button_ = button;
}

void GUIManager::RemoveButton(ButtonBase* button)
{
	auto it = std::find(buttons_.begin(), buttons_.end(), button);

	if (it != buttons_.end())
	{
		buttons_.erase(it);
	}
}

void GUIManager::UpdateTarget()
{
	Vec2 input_dir = Vec2(Input::GetAxis("horizontal"), -Input::GetAxis("vertical"));

	if (input_dir == Vec2::Zero())
		return;

	auto input_dir_normal = input_dir.Normalized();
	auto current_pos = current_picked_button_->WorldPosition();

	float min_distance = 1000000;
	ButtonBase* new_picked_button = nullptr;

	for (auto button : buttons_)
	{
		auto pos = button->WorldPosition();
		auto v = pos - current_pos;

		float dot = Vec2::Dot(v.Normalized(), input_dir_normal);
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

void GUIManager::CheckButtonPress()
{
	if (Input::GetButtonDown("ok"))
	{
		current_picked_button_->OnPressed();
	}
}
