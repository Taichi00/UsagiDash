#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "math/vec.h"

#define DINPUT_KEY_MAX 256 // キー最大数
#define DINPUT_BUTTON_MAX 24 // ゲームパッドのボタン最大数
#define DINPUT_STICK_MAX 32768
#define DINPUT_STICK_DEADZONE 4587

class Window;

class DirectInput
{
public:
	enum Button
	{
		A,
		B,
		X,
		Y,
		LB,
		RB,
		LT,
		RT,
		START,
		BACK,

		UP,
		DOWN,
		LEFT,
		RIGHT,
		LEFT_THUMB,
		RIGHT_THUMB,
		LSTICK_UP,
		LSTICK_DOWN,
		LSTICK_LEFT,
		LSTICK_RIGHT,
		RSTICK_UP,
		RSTICK_DOWN,
		RSTICK_LEFT,
		RSTICK_RIGHT,
	};

	DirectInput(Window* window);
	~DirectInput();

	void Update();

	// 押下中
	bool GetKey(UINT index);
	// 押した瞬間
	bool GetKeyDown(UINT index); 
	// 離した瞬間
	bool GetKeyUp(UINT index);
	// キーリピート
	bool GetKeyRepeat(UINT index);
	// キーの状態（b1 = 押下中, b2 = 押した瞬間, b3 = 離した瞬間, b4 = キーリピート）
	int GetKeyState(UINT index);

	// ボタン押下中
	bool GetButton(int index);
	// ボタンを押した瞬間
	bool GetButtonDown(int index);
	// ボタンを離した瞬間
	bool GetButtonUp(int index);
	// ボタンリピート
	bool GetButtonRepeat(int index);
	// ボタンの状態（b1 = 押下中, b2 = 押した瞬間, b3 = 離した瞬間, b4 = キーリピート）
	int GetButtonState(int index);

	// スティックの値を取得
	Vec2 GetLStick() const;
	Vec2 GetRStick() const;

	bool IsGamepadConnected() const { return is_gamepad_connected_; }

private:
	// インプットの生成
	HRESULT CreateInput();
	// キーデバイスの生成
	HRESULT InitKeyDevice();
	// ゲームパッドデバイスの生成
	HRESULT InitPadDevice();

	// ゲームパッドが接続された際に実行されるコールバック関数
	static BOOL CALLBACK GamepadFindCallBack(LPCDIDEVICEINSTANCE instance, LPVOID ref);

	void UpdateKey();
	void UpdatePad();

	Vec2 GetStickVec(const LONG sx, const LONG sy);

private:
	// ウィンドウ
	Window* window_;
	// インプット
	static LPDIRECTINPUT8 direct_input_;
	// インプットデバイス
	LPDIRECTINPUTDEVICE8 key_device_;
	LPDIRECTINPUTDEVICE8 pad_device_;
	// コールバック関数へ渡す配列
	LPVOID device_info_array_;

	// キー情報
	int keys_[DINPUT_KEY_MAX];
	int prev_keys_[DINPUT_KEY_MAX];

	// ゲームパッドのボタン情報
	int buttons_[DINPUT_BUTTON_MAX];
	int prev_buttons_[DINPUT_BUTTON_MAX];

	// スティック
	Vec2 left_stick_, prev_left_stick_;
	Vec2 right_stick_, prev_right_stick_;

	// リピート開始時間
	int repeat_start_;
	// リピート間隔
	int repeat_interval_;

	// ゲームパッドが接続されているかどうか
	bool is_gamepad_connected_ = false;
};