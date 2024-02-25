#include "x_input.h"
#include "game/input/input.h"
#include <stdio.h>

XInput::XInput()
{
	// state を初期化
	memset(&state_, 0, sizeof(XINPUT_STATE));
	memset(&prev_state_, 0, sizeof(XINPUT_STATE));

	is_connected_ = false;

	left_stick_ = Vec2::Zero();
	right_stick_ = Vec2::Zero();
	prev_left_stick_ = Vec2::Zero();
	prev_right_stick_ = Vec2::Zero();

	left_trigger_ = 0.f;
	right_trigger_ = 0.f;
	prev_left_trigger_ = 0.f;
	prev_right_trigger_ = 0.f;
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

	memcpy(&prev_state_, &state_, sizeof(prev_state_));

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
}

bool XInput::GetButton(const int button) const
{
	return state_.Gamepad.wButtons & button;
}

bool XInput::GetButtonDown(const int button) const
{
	return state_.Gamepad.wButtons & button && !(prev_state_.Gamepad.wButtons & button);
}

bool XInput::GetButtonUp(const int button) const
{
	return !(state_.Gamepad.wButtons & button) && prev_state_.Gamepad.wButtons & button;
}

int XInput::GetButtonState(const int button) const
{
	return GetButton(button) << 0 | GetButtonDown(button) << 1 | GetButtonUp(button) << 2;
}

Vec2 XInput::GetLStick() const
{
	return left_stick_;
}

int XInput::GetLStickUpState() const
{
	bool curr = left_stick_.y > 0.f;
	bool prev = prev_left_stick_.y > 0.f;
	return curr << 0 | (curr & !prev) << 1 | (!curr & prev) << 2;
}

int XInput::GetLStickDownState() const
{
	bool curr = left_stick_.y < 0.f;
	bool prev = prev_left_stick_.y < 0.f;
	return curr << 0 | (curr & !prev) << 1 | (!curr & prev) << 2;
}

int XInput::GetLStickLeftState() const
{
	bool curr = left_stick_.x < 0.f;
	bool prev = prev_left_stick_.x < 0.f;
	return curr << 0 | (curr & !prev) << 1 | (!curr & prev) << 2;
}

int XInput::GetLStickRightState() const
{
	bool curr = left_stick_.x > 0.f;
	bool prev = prev_left_stick_.x > 0.f;
	return curr << 0 | (curr & !prev) << 1 | (!curr & prev) << 2;
}

Vec2 XInput::GetRStick() const
{
	return right_stick_;
}

int XInput::GetRStickUpState() const
{
	bool curr = right_stick_.y > 0.f;
	bool prev = prev_right_stick_.y > 0.f;
	return curr << 0 | (curr & !prev) << 1 | (!curr & prev) << 2;
}

int XInput::GetRStickDownState() const
{
	bool curr = right_stick_.y < 0.f;
	bool prev = prev_right_stick_.y < 0.f;
	return curr << 0 | (curr & !prev) << 1 | (!curr & prev) << 2;
}

int XInput::GetRStickLeftState() const
{
	bool curr = right_stick_.x < 0.f;
	bool prev = prev_right_stick_.x < 0.f;
	return curr << 0 | (curr & !prev) << 1 | (!curr & prev) << 2;
}

int XInput::GetRStickRightState() const
{
	bool curr = right_stick_.x > 0.f;
	bool prev = prev_right_stick_.x > 0.f;
	return curr << 0 | (curr & !prev) << 1 | (!curr & prev) << 2;
}

float XInput::GetLTrigger() const
{
	return left_trigger_;
}

int XInput::GetLTriggerState() const
{
	bool curr = left_trigger_ > 0.f;
	bool prev = prev_left_trigger_ > 0.f;
	return curr << 0 | (curr & !prev) << 1 | (!curr & prev) << 2;
}

float XInput::GetRTrigger() const
{
	return right_trigger_;
}

int XInput::GetRTriggerState() const
{
	bool curr = right_trigger_ > 0.f;
	bool prev = prev_right_trigger_ > 0.f;
	return curr << 0 | (curr & !prev) << 1 | (!curr & prev) << 2;
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
