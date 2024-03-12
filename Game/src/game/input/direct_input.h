#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "math/vec.h"
#include <unordered_map>

#define DINPUT_KEY_MAX 256 // キー最大数
#define DINPUT_PAD_BUTTON_MAX 24 // ゲームパッドのボタン最大数
#define DINPUT_MOUSE_BUTTON_MAX 3 // マウスのボタン最大数

#define DINPUT_STICK_MAX 32768
#define DINPUT_STICK_DEADZONE 4587

class Window;

class DirectInput
{
public:
	enum GamepadButton
	{
		PAD_A,
		PAD_B,
		PAD_X,
		PAD_Y,
		PAD_LB,
		PAD_RB,
		PAD_LT,
		PAD_RT,
		PAD_START,
		PAD_BACK,
		PAD_LEFT_THUMB,
		PAD_RIGHT_THUMB,
		
		PAD_UP,
		PAD_DOWN,
		PAD_LEFT,
		PAD_RIGHT,
		PAD_LSTICK_UP,
		PAD_LSTICK_DOWN,
		PAD_LSTICK_LEFT,
		PAD_LSTICK_RIGHT,
		PAD_RSTICK_UP,
		PAD_RSTICK_DOWN,
		PAD_RSTICK_LEFT,
		PAD_RSTICK_RIGHT,
	};

	enum GamepadAxis
	{
		PAD_LSTICK_X,
		PAD_LSTICK_Y,
		PAD_RSTICK_X,
		PAD_RSTICK_Y,
	};

	enum GamepadType
	{
		UNKNOWN,
		SWITCH_PRO_CONTROLLER,
		DUALSHOCK4,
		DUALSENSE
	};

	enum MouseButton
	{
		MOUSE_LEFT,
		MOUSE_RIGHT,
		MOUSE_CENTER,
	};

	enum MouseAxis
	{
		MOUSE_DELTA_X,
		MOUSE_DELTA_Y,
	};

	DirectInput(Window* window);
	~DirectInput();

	void Update();

	void Refresh();

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
	bool GetGamepadButton(int index);
	// ボタンを押した瞬間
	bool GetGamepadButtonDown(int index);
	// ボタンを離した瞬間
	bool GetGamepadButtonUp(int index);
	// ボタンリピート
	bool GetGamepadButtonRepeat(int index);
	// ボタンの状態（b1 = 押下中, b2 = 押した瞬間, b3 = 離した瞬間, b4 = キーリピート）
	int GetGamepadButtonState(int index);

	// ボタン押下中
	bool GetMouseButton(int index);
	// ボタンを押した瞬間
	bool GetMouseButtonDown(int index);
	// ボタンを離した瞬間
	bool GetMouseButtonUp(int index);
	// ボタンリピート
	bool GetMouseButtonRepeat(int index);
	// ボタンの状態（b1 = 押下中, b2 = 押した瞬間, b3 = 離した瞬間, b4 = キーリピート）
	int GetMouseButtonState(int index);

	// スティックの値を取得
	Vec2 GetLStick() const;
	Vec2 GetRStick() const;

	bool IsGamepadConnected() const { return is_gamepad_connected_; }

	GamepadType GetGamepadType() const { return pad_type_; }

	// マウスカーソルの位置を取得
	Vec2 GetCursorPosition() const;
	// マウスカーソルの移動量を取得
	Vec2 GetCursorDelta() const;

private:
	// インプットの生成
	HRESULT CreateInput();
	// キーデバイスの生成
	HRESULT InitKeyDevice();
	// ゲームパッドデバイスの生成
	HRESULT InitPadDevice();
	// マウスデバイスの生成
	HRESULT InitMouseDevice();

	// ゲームパッドが接続された際に実行されるコールバック関数
	static BOOL CALLBACK GamepadFindCallBack(LPCDIDEVICEINSTANCE instance, LPVOID ref);

	void UpdateKey();
	void UpdatePad();
	void UpdateMouse();

	Vec2 GetStickVec(const LONG sx, const LONG sy);

	void InitPadButtonMap();

private:
	enum VendorID
	{
		VID_SWITCH_PRO_CONTROLLER = 0x057e,
		VID_DUALSHOCK4_1X = 0x054c,
		VID_DUALSHOCK4_2X = 0x054c,
		VID_DUALSENSE = 0x54c,
	};

	enum ProductID
	{
		PID_SWITCH_PRO_CONTROLLER = 0x2009,
		PID_DUALSHOCK4_1X = 0x05c4,
		PID_DUALSHOCK4_2X = 0x09cc,
		PID_DUALSENSE = 0x0ce6,
	};

	// ウィンドウ
	Window* window_;
	// インプット
	static LPDIRECTINPUT8 direct_input_;
	// インプットデバイス
	LPDIRECTINPUTDEVICE8 key_device_;
	LPDIRECTINPUTDEVICE8 pad_device_;
	LPDIRECTINPUTDEVICE8 mouse_device_;
	// コールバック関数へ渡す配列
	LPVOID device_info_array_;
	// ゲームパッドの種別
	GamepadType pad_type_ = UNKNOWN;

	struct GamepadInfo
	{
		LPDIRECTINPUTDEVICE8 device;
		DWORD vendor_id;
		DWORD product_id;
	};

	// キー情報
	int keys_[DINPUT_KEY_MAX];
	int prev_keys_[DINPUT_KEY_MAX];

	// ゲームパッドのボタン情報
	int pad_buttons_[DINPUT_PAD_BUTTON_MAX];
	int prev_pad_buttons_[DINPUT_PAD_BUTTON_MAX];

	// スティック
	Vec2 left_stick_, prev_left_stick_;
	Vec2 right_stick_, prev_right_stick_;

	// リピート開始時間
	int repeat_start_;
	// リピート間隔
	int repeat_interval_;

	// ゲームパッドが接続されているかどうか
	bool is_gamepad_connected_ = false;

	// 現在のゲームパッドにおけるボタンマップ
	std::unordered_map<GamepadButton, int> pad_button_map_;
	std::unordered_map<GamepadAxis, int> pad_axis_map_;

	// マウスカーソルの位置
	Vec2 cursor_position_;
	// マウスカーソルの移動量
	Vec2 cursor_delta_;

	// マウスのボタン情報
	int mouse_buttons_[DINPUT_MOUSE_BUTTON_MAX];
	int prev_mouse_buttons_[DINPUT_MOUSE_BUTTON_MAX];
};