#pragma once

#include <windows.h>

#pragma comment(lib, "xinput.lib")
#include <Xinput.h>

#include "math/vec.h"

class XInput
{
public:
	XInput();
	~XInput();

	void Update();

	// 押下中
	bool GetButton(const int button) const;
	// 押した瞬間
	bool GetButtonDown(const int button) const;
	// 離した瞬間
	bool GetButtonUp(const int button) const;

	// キーの状態（b1 = 押下中, b2 = 押した瞬間, b3 = 離した瞬間）
	int GetButtonState(const int button) const;

	Vec2 GetLStick() const;
	int GetLStickUpState() const;
	int GetLStickDownState() const;
	int GetLStickLeftState() const;
	int GetLStickRightState() const;

	Vec2 GetRStick() const;
	int GetRStickUpState() const;
	int GetRStickDownState() const;
	int GetRStickLeftState() const;
	int GetRStickRightState() const;

	float GetLTrigger() const;
	int GetLTriggerState() const;

	float GetRTrigger() const;
	int GetRTriggerState() const;

private:
	Vec2 GetStickVec(const SHORT sx, const SHORT sy, const int dead_zone) const;
	float GetTriggerValue(const BYTE v, const int threshold) const;

private:
	// コントローラーの状態
	XINPUT_STATE state_;
	// 前の状態
	XINPUT_STATE prev_state_;

	Vec2 left_stick_, prev_left_stick_;
	Vec2 right_stick_, prev_right_stick_;

	float left_trigger_, prev_left_trigger_;
	float right_trigger_, prev_right_trigger_;

	// コントローラーが接続されているかどうか
	bool is_connected_;
};