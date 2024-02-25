#include "game/input/input.h"
#include "engine/window.h"
#include "game/input/direct_input.h"
#include "game/input/x_input.h"

Input* Input::instance_ = nullptr;

Input::Input(Window* window)
{
	window_ = window;

	direct_input_ = std::make_unique<DirectInput>(window_);
	x_input_ = std::make_unique<XInput>();

	current_input_type_ = InputType::KEYBOARD;

	// デフォルトキーマップ
	AddButtonAction("ok", {
		{ InputType::KEYBOARD, Button::KEY_RETURN },
		{ InputType::GAMEPAD, Button::PAD_A }
		});
	AddButtonAction("cancel", {
		{ InputType::KEYBOARD, Button::KEY_ESCAPE },
		{ InputType::GAMEPAD, Button::PAD_B }
		});
	AddButtonAction("up", {
		{ InputType::KEYBOARD, Button::KEY_UP },
		{ InputType::GAMEPAD, Button::PAD_UP }
		});
	AddButtonAction("down", {
		{ InputType::KEYBOARD, Button::KEY_DOWN },
		{ InputType::GAMEPAD, Button::PAD_DOWN }
		});
	AddButtonAction("left", {
		{ InputType::KEYBOARD, Button::KEY_LEFT },
		{ InputType::GAMEPAD, Button::PAD_LEFT }
		});
	AddButtonAction("right", {
		{ InputType::KEYBOARD, Button::KEY_RIGHT },
		{ InputType::GAMEPAD, Button::PAD_RIGHT }
		});

	AddAxisAction("horizontal", {
		{ InputType::KEYBOARD, Axis::KEY_ARROW_X },
		{ InputType::GAMEPAD, Axis::PAD_LSTICK_X },
		});
	AddAxisAction("vertical", {
		{ InputType::KEYBOARD, Axis::KEY_ARROW_Y },
		{ InputType::GAMEPAD, Axis::PAD_LSTICK_Y },
		});
}

Input::~Input()
{
	
}

void Input::Create(Window* win)
{
	if (!instance_)
	{
		instance_ = new Input(win);
	}
}

void Input::Destroy()
{
	delete instance_;
	instance_ = nullptr;
}

void Input::Update()
{
	direct_input_->Update();
	x_input_->Update();

	CheckActions();
}

void Input::AddButtonAction(const std::string& name, const std::vector<ButtonActionInfo>& info_list)
{
	button_action_map_[name] = info_list;
}

void Input::AddAxisAction(const std::string& name, const std::vector<AxisActionInfo>& info_list)
{
	axis_action_map_[name] = info_list;
}

// キー入力
bool Input::GetKey(UINT index)
{
	return instance_->direct_input_->GetKey(index);
}

// トリガーの入力
bool Input::GetKeyDown(UINT index)
{
	return instance_->direct_input_->GetKeyDown(index);
}

bool Input::GetButton(const std::string& key)
{
	return instance_->button_action_state_[key] & 0b001;
}

bool Input::GetButtonDown(const std::string& key)
{
	return instance_->button_action_state_[key] & 0b010;
}

bool Input::GetButtonUp(const std::string& key)
{
	return instance_->button_action_state_[key] & 0b100;
}

float Input::GetAxis(const std::string& key)
{
	return instance_->axis_action_state_[key];
}

void Input::CheckActions()
{
	// button action
	for (const auto& map_info : button_action_map_)
	{
		int state = 0;

		for (const auto& input_info : map_info.second)
		{
			switch (input_info.type)
			{
			case InputType::KEYBOARD:
				state |= direct_input_->GetKeyState(input_info.button);
				break;
			case InputType::GAMEPAD:
				switch (input_info.button)
				{
				case PAD_LSTICK_UP:
					state |= x_input_->GetLStickUpState();
					break;
				case PAD_LSTICK_DOWN:
					state |= x_input_->GetLStickDownState();
					break;
				case PAD_LSTICK_LEFT:
					state |= x_input_->GetLStickLeftState();
					break;
				case PAD_LSTICK_RIGHT:
					state |= x_input_->GetLStickRightState();
					break;
				case PAD_RSTICK_UP:
					state |= x_input_->GetRStickUpState();
					break;
				case PAD_RSTICK_DOWN:
					state |= x_input_->GetRStickDownState();
					break;
				case PAD_RSTICK_LEFT:
					state |= x_input_->GetRStickLeftState();
					break;
				case PAD_RSTICK_RIGHT:
					state |= x_input_->GetRStickRightState();
					break;
				case PAD_LT:
					state |= x_input_->GetLTriggerState();
					break;
				case PAD_RT:
					state |= x_input_->GetRTriggerState();
					break;
				default:
					state |= x_input_->GetButtonState(input_info.button);
					break;
				}
				break;
			case InputType::MOUSE:
				break;
			}

			if (state & 1)
			{
				current_input_type_ = input_info.type;
				break;
			}
		}

		button_action_state_[map_info.first] = state;
	}

	// axis action
	for (const auto& map_info : axis_action_map_)
	{
		float state = 0.f;

		for (const auto& input_info : map_info.second)
		{
			switch (input_info.type)
			{
			case InputType::KEYBOARD:
				switch (input_info.axis)
				{
				case Axis::KEY_ARROW_X:
					state = (float)(-(direct_input_->GetKey(Button::KEY_LEFT) & 1) + (direct_input_->GetKey(Button::KEY_RIGHT) & 1));
					break;
				case Axis::KEY_ARROW_Y:
					state = (float)((direct_input_->GetKey(Button::KEY_UP) & 1) - (direct_input_->GetKey(Button::KEY_DOWN) & 1));
					break;
				case Axis::KEY_WASD_X:
					state = (float)(-(direct_input_->GetKey(Button::KEY_A) & 1) + (direct_input_->GetKey(Button::KEY_D) & 1));
					break;
				case Axis::KEY_WASD_Y:
					state = (float)((direct_input_->GetKey(Button::KEY_W) & 1) - (direct_input_->GetKey(Button::KEY_S) & 1));
					break;
				}
				break;

			case InputType::GAMEPAD:
				switch (input_info.axis)
				{
				case Axis::PAD_LSTICK_X:
					state = x_input_->GetLStick().x;
					break;
				case Axis::PAD_LSTICK_Y:
					state = x_input_->GetLStick().y;
					break;
				case Axis::PAD_RSTICK_X:
					state = x_input_->GetRStick().x;
					break;
				case Axis::PAD_RSTICK_Y:
					state = x_input_->GetRStick().y;
					break;
				}
				break;

			case InputType::MOUSE:
				break;
			}

			if (state)
			{
				current_input_type_ = input_info.type;
				break;
			}
		}

		axis_action_state_[map_info.first] = state;
	}
}
