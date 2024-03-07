#include "x_input.h"
#include "game/input/input.h"
#include <stdio.h>

XInput::XInput()
{
	// state を初期化
	memset(&buttons_, 0, sizeof(buttons_));
	memset(&prev_buttons_, 0, sizeof(prev_buttons_));

	InitButtonMap();

	is_connected_ = false;

	left_stick_ = Vec2::Zero();
	right_stick_ = Vec2::Zero();
	prev_left_stick_ = Vec2::Zero();
	prev_right_stick_ = Vec2::Zero();

	left_trigger_ = 0.f;
	right_trigger_ = 0.f;
	prev_left_trigger_ = 0.f;
	prev_right_trigger_ = 0.f;

	repeat_start_ = 20;
	repeat_interval_ = 3;
}

XInput::~XInput()
{
}

void XInput::Update()
{
	prev_left_stick_ = left_stick_;
	prev_right_stick_ = right_stick_;
	prev_left_trigger_ = left_trigger_;
	prev_right_trigger_ = right_trigger_;

	memcpy(&prev_buttons_, &buttons_, sizeof(prev_buttons_));

	if (XInputGetState(0, &state_) != ERROR_SUCCESS)
	{
		// コントローラーが接続されていない
		is_connected_ = false;

		left_stick_ = Vec2::Zero();
		right_stick_ = Vec2::Zero();
		left_trigger_ = 0.f;
		right_trigger_ = 0.f;
		
		return;
	}

	// 接続時に input type を強制的に変更する
	if (!is_connected_)
		Input::SetCurrentInputType(Input::InputType::GAMEPAD);

	// コントローラーが接続されている
	is_connected_ = true;

	left_stick_ = GetStickVec(
		state_.Gamepad.sThumbLX,
		state_.Gamepad.sThumbLY,
		XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE
	);
	right_stick_ = GetStickVec(
		state_.Gamepad.sThumbRX,
		state_.Gamepad.sThumbRY,
		XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE
	);
	
	left_trigger_ = GetTriggerValue(
		state_.Gamepad.bLeftTrigger,
		XINPUT_GAMEPAD_TRIGGER_THRESHOLD
	);
	right_trigger_ = GetTriggerValue(
		state_.Gamepad.bRightTrigger,
		XINPUT_GAMEPAD_TRIGGER_THRESHOLD
	);

	UpdateButton();
}

bool XInput::GetButton(const int index) const
{
	return buttons_[index];
}

bool XInput::GetButtonDown(const int index) const
{
	return buttons_[index] && !(prev_buttons_[index]);
}

bool XInput::GetButtonUp(const int index) const
{
	return !(buttons_[index]) && prev_buttons_[index];
}

bool XInput::GetButtonRepeat(const int index) const
{
	int b = buttons_[index];

	if (b == 1) return true;
	if (b <= repeat_start_) return b == repeat_start_;
	return (b - repeat_start_) % repeat_interval_ == 0;
}

int XInput::GetButtonState(const int index) const
{
	return 
		GetButton(index) << 0 | 
		GetButtonDown(index) << 1 | 
		GetButtonUp(index) << 2 |
		GetButtonRepeat(index) << 3;
}

Vec2 XInput::GetLStick() const
{
	return left_stick_;
}

Vec2 XInput::GetRStick() const
{
	return right_stick_;
}

float XInput::GetLTrigger() const
{
	return left_trigger_;
}

float XInput::GetRTrigger() const
{
	return right_trigger_;
}

Vec2 XInput::GetStickVec(const SHORT sx, const SHORT sy, const int dead_zone) const
{
	auto len = std::sqrt(sx * sx + sy * sy);

	Vec2 v = Vec2::Zero();
	if (len >= dead_zone)
	{
		float max = 32768.f;
		auto x = std::abs(sx);
		auto y = std::abs(sy);

		v.x = sx < 0 ? -x / max : x / (max - 1);
		v.y = sy < 0 ? -y / max : y / (max - 1);
	}
	return v;
}

float XInput::GetTriggerValue(const BYTE v, const int threshold) const
{
	float n = 0.f;
	if (v >= threshold)
	{
		n = (v - threshold) / 256.f;
	}
	return n;
}

void XInput::UpdateButton()
{
	for (int i = 0; i < XINPUT_BUTTON_MAX; i++)
	{
		bool pressed = false;

		if (button_map_.contains((Button)i))
		{
			pressed = state_.Gamepad.wButtons & button_map_[(Button)i];
		}
		else
		{
			switch (i)
			{
			case Button::LSTICK_UP:
				pressed = left_stick_.y > XINPUT_STICK_BUTTON_THRESHOLD;
				break;
			case Button::LSTICK_DOWN:
				pressed = left_stick_.y < -XINPUT_STICK_BUTTON_THRESHOLD;
				break;
			case Button::LSTICK_LEFT:
				pressed = left_stick_.x < -XINPUT_STICK_BUTTON_THRESHOLD;
				break;
			case Button::LSTICK_RIGHT:
				pressed = left_stick_.x > XINPUT_STICK_BUTTON_THRESHOLD;
				break;
			case Button::RSTICK_UP:
				pressed = right_stick_.y > XINPUT_STICK_BUTTON_THRESHOLD;
				break;
			case Button::RSTICK_DOWN:
				pressed = right_stick_.y < -XINPUT_STICK_BUTTON_THRESHOLD;
				break;
			case Button::RSTICK_LEFT:
				pressed = right_stick_.x < -XINPUT_STICK_BUTTON_THRESHOLD;
				break;
			case Button::RSTICK_RIGHT:
				pressed = right_stick_.x > XINPUT_STICK_BUTTON_THRESHOLD;
				break;
			case Button::LT:
				pressed = left_trigger_ > 0;
				break;
			case Button::RT:
				pressed = right_trigger_ > 0;
				break;
			}
		}

		buttons_[i] = pressed ? buttons_[i] + 1 : 0;
	}
}

void XInput::InitButtonMap()
{
	button_map_ =
	{
		{ UP, XINPUT_GAMEPAD_DPAD_UP },
		{ DOWN, XINPUT_GAMEPAD_DPAD_DOWN },
		{ LEFT, XINPUT_GAMEPAD_DPAD_LEFT },
		{ RIGHT, XINPUT_GAMEPAD_DPAD_RIGHT },
		{ START, XINPUT_GAMEPAD_START },
		{ BACK, XINPUT_GAMEPAD_BACK },
		{ LEFT_THUMB, XINPUT_GAMEPAD_LEFT_THUMB },
		{ RIGHT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB },
		{ LB, XINPUT_GAMEPAD_LEFT_SHOULDER },
		{ RB, XINPUT_GAMEPAD_RIGHT_SHOULDER },
		{ A, XINPUT_GAMEPAD_A },
		{ B, XINPUT_GAMEPAD_B },
		{ X, XINPUT_GAMEPAD_X },
		{ Y, XINPUT_GAMEPAD_Y },
	};
}
