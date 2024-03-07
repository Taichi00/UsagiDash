#pragma once

#include <windows.h>

#pragma comment(lib, "xinput.lib")
#include <Xinput.h>

#include "math/vec.h"
#include <unordered_map>

#define XINPUT_BUTTON_MAX 24
// スティックをボタン入力として認識する閾値
#define XINPUT_STICK_BUTTON_THRESHOLD 0.4

class XInput
{
public:
	enum Button
	{
		UP,
		DOWN,
		LEFT,
		RIGHT,
		START,
		BACK,
		LEFT_THUMB,
		RIGHT_THUMB,
		LB,
		RB,
		A,
		B,
		X,
		Y,
		LSTICK_UP,
		LSTICK_DOWN,
		LSTICK_LEFT,
		LSTICK_RIGHT,
		RSTICK_UP,
		RSTICK_DOWN,
		RSTICK_LEFT,
		RSTICK_RIGHT,
		LT,
		RT
	};

	XInput();
	~XInput();

	void Update();

	// 押下中
	bool GetButton(const int index) const;
	// 押した瞬間
	bool GetButtonDown(const int index) const;
	// 離した瞬間
	bool GetButtonUp(const int index) const;
	// リピート
	bool GetButtonRepeat(const int index) const;

	// キーの状態（b1 = 押下中, b2 = 押した瞬間, b3 = 離した瞬間, b4 = リピート）
	int GetButtonState(const int index) const;

	Vec2 GetLStick() const;
	Vec2 GetRStick() const;
	
	float GetLTrigger() const;
	float GetRTrigger() const;

	bool IsConnected() const { return is_connected_; }

private:
	Vec2 GetStickVec(const SHORT sx, const SHORT sy, const int dead_zone) const;
	float GetTriggerValue(const BYTE v, const int threshold) const;

	void UpdateButton();
	void InitButtonMap();

private:
	// コントローラーの状態
	XINPUT_STATE state_;

	// ボタンの状態
	int buttons_[XINPUT_BUTTON_MAX];
	// 前の状態
	int prev_buttons_[XINPUT_BUTTON_MAX];

	Vec2 left_stick_, prev_left_stick_;
	Vec2 right_stick_, prev_right_stick_;

	float left_trigger_, prev_left_trigger_;
	float right_trigger_, prev_right_trigger_;

	// コントローラーが接続されているかどうか
	bool is_connected_;

	std::unordered_map<Button, int> button_map_;

	// リピート開始時間
	int repeat_start_;
	// リピート間隔
	int repeat_interval_;
};