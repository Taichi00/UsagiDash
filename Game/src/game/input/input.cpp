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

	InitButtonMap();

	// デフォルトキーマップ
	AddButtonAction("ok", {
		{ InputType::KEYBOARD, Button::KEY_RETURN },
		{ InputType::KEYBOARD, Button::KEY_SPACE },
		{ InputType::GAMEPAD, Button::PAD_A }
		});
	AddButtonAction("cancel", {
		{ InputType::KEYBOARD, Button::KEY_ESCAPE },
		{ InputType::GAMEPAD, Button::PAD_B }
		});
	AddButtonAction("up", {
		{ InputType::KEYBOARD, Button::KEY_UP },
		{ InputType::GAMEPAD, Button::PAD_UP },
		{ InputType::GAMEPAD, Button::PAD_LSTICK_UP },
		});
	AddButtonAction("down", {
		{ InputType::KEYBOARD, Button::KEY_DOWN },
		{ InputType::GAMEPAD, Button::PAD_DOWN },
		{ InputType::GAMEPAD, Button::PAD_LSTICK_DOWN },
		});
	AddButtonAction("left", {
		{ InputType::KEYBOARD, Button::KEY_LEFT },
		{ InputType::GAMEPAD, Button::PAD_LEFT },
		{ InputType::GAMEPAD, Button::PAD_LSTICK_LEFT },
		});
	AddButtonAction("right", {
		{ InputType::KEYBOARD, Button::KEY_RIGHT },
		{ InputType::GAMEPAD, Button::PAD_RIGHT },
		{ InputType::GAMEPAD, Button::PAD_LSTICK_RIGHT },
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

void Input::Refresh()
{
	instance_->x_input_->Refresh();
	instance_->direct_input_->Refresh();
}

void Input::SetActive(const bool flag)
{
	if (!flag && instance_->is_active_)
	{
		for (auto& state : instance_->button_action_state_)
		{
			state.second = 0;
		}
		for (auto& state : instance_->axis_action_state_)
		{
			state.second = 0;
		}
	}

	instance_->x_input_->SetActive(flag);
	instance_->direct_input_->SetActive(flag);

	instance_->is_active_ = flag;
}

void Input::Update()
{
	if (!is_active_)
		return;

	direct_input_->Update();
	x_input_->Update();

	CheckActions();

	CheckGamepadType();
}

void Input::AddButtonAction(const std::string& name, const std::vector<ButtonActionInfo>& info_list)
{
	button_action_map_[name] = info_list;
}

void Input::AddAxisAction(const std::string& name, const std::vector<AxisActionInfo>& info_list)
{
	axis_action_map_[name] = info_list;
}

bool Input::GetKey(const int index)
{
	return instance_->direct_input_->GetKey(index);
}

bool Input::GetKeyDown(const int index)
{
	return instance_->direct_input_->GetKeyDown(index);
}

bool Input::GetKeyUp(const int index)
{
	return instance_->direct_input_->GetKeyUp(index);
}

bool Input::GetKeyRepeat(const int index)
{
	return instance_->direct_input_->GetKeyRepeat(index);
}

bool Input::GetMouseButton(const int index)
{
	return instance_->direct_input_->GetMouseButton(index);
}

bool Input::GetMouseButtonDown(const int index)
{
	return instance_->direct_input_->GetMouseButtonDown(index);
}

bool Input::GetMouseButtonUp(const int index)
{
	return instance_->direct_input_->GetMouseButtonUp(index);
}

bool Input::GetMouseButtonRepeat(const int index)
{
	return instance_->direct_input_->GetMouseButtonRepeat(index);
}

bool Input::GetButton(const std::string& key)
{
	return instance_->button_action_state_[key] & 0b0001;
}

bool Input::GetButtonDown(const std::string& key)
{
	return instance_->button_action_state_[key] & 0b0010;
}

bool Input::GetButtonUp(const std::string& key)
{
	return instance_->button_action_state_[key] & 0b0100;
}

bool Input::GetButtonRepeat(const std::string& key)
{
	return instance_->button_action_state_[key] & 0b1000;
}

float Input::GetAxis(const std::string& key)
{
	return instance_->axis_action_state_[key];
}

Vec2 Input::GetCursorPosition()
{
	return instance_->direct_input_->GetCursorPosition();
}

Vec2 Input::GetCursorDelta()
{
	return instance_->direct_input_->GetCursorDelta();
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
				state |= direct_input_->GetKeyState(dinput_map_[input_info.button]);
				break;
			case InputType::GAMEPAD:
				if (x_input_->IsConnected())
				{
					state |= x_input_->GetButtonState(xinput_map_[input_info.button]);
				}
				else if (direct_input_->IsGamepadConnected())
				{
					state |= direct_input_->GetGamepadButtonState(dinput_map_[input_info.button]);
				}
				break;
			case InputType::MOUSE:
				state |= direct_input_->GetMouseButtonState(dinput_map_[input_info.button]);
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
					state = (float)(
						-(direct_input_->GetKey(dinput_map_[Button::KEY_LEFT]) & 1) + 
						 (direct_input_->GetKey(dinput_map_[Button::KEY_RIGHT]) & 1));
					break;
				case Axis::KEY_ARROW_Y:
					state = (float)(
						(direct_input_->GetKey(dinput_map_[Button::KEY_UP]) & 1) - 
						(direct_input_->GetKey(dinput_map_[Button::KEY_DOWN]) & 1));
					break;
				case Axis::KEY_WASD_X:
					state = (float)(
						-(direct_input_->GetKey(dinput_map_[Button::KEY_A]) & 1) + 
						 (direct_input_->GetKey(dinput_map_[Button::KEY_D]) & 1));
					break;
				case Axis::KEY_WASD_Y:
					state = (float)(
						(direct_input_->GetKey(dinput_map_[Button::KEY_W]) & 1) - 
						(direct_input_->GetKey(dinput_map_[Button::KEY_S]) & 1));
					break;
				}
				break;

			case InputType::GAMEPAD:
				if (x_input_->IsConnected())
				{
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
				}
				else if (direct_input_->IsGamepadConnected())
				{
					switch (input_info.axis)
					{
					case Axis::PAD_LSTICK_X:
						state = direct_input_->GetLStick().x;
						break;
					case Axis::PAD_LSTICK_Y:
						state = direct_input_->GetLStick().y;
						break;
					case Axis::PAD_RSTICK_X:
						state = direct_input_->GetRStick().x;
						break;
					case Axis::PAD_RSTICK_Y:
						state = direct_input_->GetRStick().y;
						break;
					}
				}
				break;

			case InputType::MOUSE:
				switch (input_info.axis)
				{
				case Axis::MOUSE_DELTA_X:
					state = direct_input_->GetCursorDelta().x;
					break;
				case Axis::MOUSE_DELTA_Y:
					state = direct_input_->GetCursorDelta().y;
					break;
				}
				break;
			}

			if (state)
			{
				current_input_type_ = input_info.type;
				break;
			}
		}

		if (current_input_type_ != Input::MOUSE)
		{
			axis_action_state_[map_info.first] = std::min(std::max(state, -1.0f), 1.0f);
		}
		else
		{
			axis_action_state_[map_info.first] = state;
		}
	}
}

void Input::CheckGamepadType()
{
	if (x_input_->IsConnected())
	{
		pad_type_ = XBOX_CONTROLLER;
	}
	else if (direct_input_->IsGamepadConnected())
	{
		switch (direct_input_->GetGamepadType())
		{
		case DirectInput::SWITCH_PRO_CONTROLLER:
			pad_type_ = SWITCH_PRO_CONTROLLER;
			break;
		case DirectInput::DUALSHOCK4:
			pad_type_ = DUALSHOCK4;
			break;
		case DirectInput::DUALSENSE:
			pad_type_ = DUALSENSE;
			break;
		case DirectInput::UNKNOWN:
			pad_type_ = UNKNOWN;
			break;
		}
	}
}

void Input::InitButtonMap()
{
	dinput_map_ =
	{
		{ KEY_ESCAPE, DIK_ESCAPE },
		{ KEY_RETURN, DIK_RETURN },
		{ KEY_SPACE, DIK_SPACE },
		{ KEY_LSHIFT, DIK_LSHIFT },
		{ KEY_RSHIFT, DIK_RSHIFT },

		{ KEY_UP, DIK_UP },
		{ KEY_DOWN, DIK_DOWN },
		{ KEY_LEFT, DIK_LEFT },
		{ KEY_RIGHT, DIK_RIGHT },

		{ KEY_1, DIK_1 },
		{ KEY_2, DIK_2 },
		{ KEY_3, DIK_3 },
		{ KEY_4, DIK_4 },
		{ KEY_5, DIK_5 },
		{ KEY_6, DIK_6 },
		{ KEY_7, DIK_7 },
		{ KEY_8, DIK_8 },
		{ KEY_9, DIK_9 },
		{ KEY_0, DIK_0 },

		{ KEY_A, DIK_A },
		{ KEY_B, DIK_B },
		{ KEY_C, DIK_C },
		{ KEY_D, DIK_D },
		{ KEY_E, DIK_E },
		{ KEY_F, DIK_F },
		{ KEY_G, DIK_G },
		{ KEY_H, DIK_H },
		{ KEY_I, DIK_I },
		{ KEY_J, DIK_J },
		{ KEY_K, DIK_K },
		{ KEY_L, DIK_L },
		{ KEY_M, DIK_M },
		{ KEY_N, DIK_N },
		{ KEY_O, DIK_O },
		{ KEY_P, DIK_P },
		{ KEY_Q, DIK_Q },
		{ KEY_R, DIK_R },
		{ KEY_S, DIK_S },
		{ KEY_T, DIK_T },
		{ KEY_U, DIK_U },
		{ KEY_V, DIK_V },
		{ KEY_W, DIK_W },
		{ KEY_X, DIK_X },
		{ KEY_Y, DIK_Y },
		{ KEY_Z, DIK_Z },

		{ PAD_UP, DirectInput::PAD_UP },
		{ PAD_DOWN, DirectInput::PAD_DOWN },
		{ PAD_LEFT, DirectInput::PAD_LEFT },
		{ PAD_RIGHT, DirectInput::PAD_RIGHT },

		{ PAD_LB, DirectInput::PAD_LB },
		{ PAD_RB, DirectInput::PAD_RB },

		{ PAD_A, DirectInput::PAD_A },
		{ PAD_B, DirectInput::PAD_B },
		{ PAD_X, DirectInput::PAD_X },
		{ PAD_Y, DirectInput::PAD_Y },

		{ PAD_LSTICK_UP, DirectInput::PAD_LSTICK_UP },
		{ PAD_LSTICK_DOWN, DirectInput::PAD_LSTICK_DOWN },
		{ PAD_LSTICK_LEFT, DirectInput::PAD_LSTICK_LEFT },
		{ PAD_LSTICK_RIGHT, DirectInput::PAD_LSTICK_RIGHT },
		
		{ PAD_RSTICK_UP, DirectInput::PAD_RSTICK_UP },
		{ PAD_RSTICK_DOWN, DirectInput::PAD_RSTICK_DOWN },
		{ PAD_RSTICK_LEFT, DirectInput::PAD_RSTICK_LEFT },
		{ PAD_RSTICK_RIGHT, DirectInput::PAD_RSTICK_RIGHT },
		
		{ PAD_LT, DirectInput::PAD_LT },
		{ PAD_RT, DirectInput::PAD_RT },

		{ PAD_START, DirectInput::PAD_START },
		{ PAD_BACK, DirectInput::PAD_BACK },

		{ MOUSE_LEFT, DirectInput::MOUSE_LEFT },
		{ MOUSE_RIGHT, DirectInput::MOUSE_RIGHT },
		{ MOUSE_CENTER, DirectInput::MOUSE_CENTER },
	};

	xinput_map_ = 
	{
		{ PAD_UP, XInput::UP },
		{ PAD_DOWN, XInput::DOWN },
		{ PAD_LEFT, XInput::LEFT },
		{ PAD_RIGHT, XInput::RIGHT },

		{ PAD_LB, XInput::LB },
		{ PAD_RB, XInput::RB },

		{ PAD_A, XInput::A },
		{ PAD_B, XInput::B },
		{ PAD_X, XInput::X },
		{ PAD_Y, XInput::Y },

		{ PAD_LSTICK_UP, XInput::LSTICK_UP },
		{ PAD_LSTICK_DOWN, XInput::LSTICK_DOWN },
		{ PAD_LSTICK_LEFT, XInput::LSTICK_LEFT },
		{ PAD_LSTICK_RIGHT, XInput::LSTICK_RIGHT },

		{ PAD_RSTICK_UP, XInput::RSTICK_UP },
		{ PAD_RSTICK_DOWN, XInput::RSTICK_DOWN },
		{ PAD_RSTICK_LEFT, XInput::RSTICK_LEFT },
		{ PAD_RSTICK_RIGHT, XInput::RSTICK_RIGHT },

		{ PAD_LT, XInput::LT },
		{ PAD_RT, XInput::RT },

		{ PAD_START, XInput::START },
		{ PAD_BACK, XInput::BACK },
	};
}
