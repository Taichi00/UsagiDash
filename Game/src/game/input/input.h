#pragma once

#define NOMINMAX
#include <windows.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "game/input/direct_input.h"
#include "game/input/x_input.h"

class Window;

class Input
{
public:
	// 入力種別
	enum class InputType
	{
		KEYBOARD,
		GAMEPAD,
		MOUSE
	};

	enum Button
	{
		KEY_ESCAPE = DIK_ESCAPE,
		KEY_RETURN = DIK_RETURN,
		KEY_SPACE = DIK_SPACE,
		KEY_LSHIFT = DIK_LSHIFT,
		KEY_RSHIFT = DIK_RSHIFT,

		KEY_UP = DIK_UP,
		KEY_DOWN = DIK_DOWN,
		KEY_LEFT = DIK_LEFT,
		KEY_RIGHT = DIK_RIGHT,

		KEY_1 = DIK_1,
		KEY_2 = DIK_2,
		KEY_3 = DIK_3,
		KEY_4 = DIK_4,
		KEY_5 = DIK_5,
		KEY_6 = DIK_6,
		KEY_7 = DIK_7,
		KEY_8 = DIK_8,
		KEY_9 = DIK_9,
		KEY_0 = DIK_0,

		KEY_A = DIK_A,
		KEY_B = DIK_B,
		KEY_C = DIK_C,
		KEY_D = DIK_D,
		KEY_E = DIK_E,
		KEY_F = DIK_F,
		KEY_G = DIK_G,
		KEY_H = DIK_H,
		KEY_I = DIK_I,
		KEY_J = DIK_J,
		KEY_K = DIK_K,
		KEY_L = DIK_L,
		KEY_M = DIK_M,
		KEY_N = DIK_N,
		KEY_O = DIK_O,
		KEY_P = DIK_P,
		KEY_Q = DIK_Q,
		KEY_R = DIK_R,
		KEY_S = DIK_S,
		KEY_T = DIK_T,
		KEY_U = DIK_U,
		KEY_V = DIK_V,
		KEY_W = DIK_W,
		KEY_X = DIK_X,
		KEY_Y = DIK_Y,
		KEY_Z = DIK_Z,

		PAD_UP = XINPUT_GAMEPAD_DPAD_UP,
		PAD_DOWN = XINPUT_GAMEPAD_DPAD_DOWN,
		PAD_LEFT = XINPUT_GAMEPAD_DPAD_LEFT,
		PAD_RIGHT = XINPUT_GAMEPAD_DPAD_RIGHT,

		PAD_LB = XINPUT_GAMEPAD_LEFT_SHOULDER,
		PAD_RB = XINPUT_GAMEPAD_RIGHT_SHOULDER,
		
		PAD_A = XINPUT_GAMEPAD_A,
		PAD_B = XINPUT_GAMEPAD_B,
		PAD_X = XINPUT_GAMEPAD_X,
		PAD_Y = XINPUT_GAMEPAD_Y,

		PAD_LSTICK_UP,
		PAD_LSTICK_DOWN,
		PAD_LSTICK_LEFT,
		PAD_LSTICK_RIGHT,

		PAD_RSTICK_UP,
		PAD_RSTICK_DOWN,
		PAD_RSTICK_LEFT,
		PAD_RSTICK_RIGHT,

		PAD_LT,
		PAD_RT,
	};

	enum Axis
	{
		KEY_ARROW_X,
		KEY_ARROW_Y,
		KEY_WASD_X,
		KEY_WASD_Y,

		PAD_LSTICK_X,
		PAD_LSTICK_Y,
		PAD_RSTICK_X,
		PAD_RSTICK_Y,
	};

	struct ButtonActionInfo
	{
		InputType type; // 入力種別
		Button button; // ボタン
	};

	struct AxisActionInfo
	{
		InputType type;
		Axis axis;
	};

private:
	Input(Window* window);
	~Input();

public:
	Input(const Input&) = delete;
	Input& operator =(const Input&) = delete;
	Input(Input&&) = delete;
	Input& operator =(Input&&) = delete;

	static Input* Get()
	{
		return instance_;
	}

	static void Create(Window* win);
	static void Destroy();

	void Update();

	void AddButtonAction(const std::string& name, const std::vector<ButtonActionInfo>& info_list);
	void AddAxisAction(const std::string& name, const std::vector<AxisActionInfo>& info_list);

	// キー入力
	static bool GetKey(UINT index);
	// トリガーの入力
	static bool GetKeyDown(UINT index);

	// ボタン押下中
	static bool GetButton(const std::string& key);
	// ボタンを押した瞬間
	static bool GetButtonDown(const std::string& key);
	// ボタンを離した瞬間
	static bool GetButtonUp(const std::string& key);

	// 軸の値
	static float GetAxis(const std::string& key);

	// 現在の入力種別を取得
	static InputType CurrentInputType() { return instance_->current_input_type_; }
	// 入力種別を設定
	static void SetCurrentInputType(InputType type) { instance_->current_input_type_ = type; }

private:
	void CheckActions();

private:
	// ウィンドウ
	Window* window_;

	std::unique_ptr<DirectInput> direct_input_;
	std::unique_ptr<XInput> x_input_;

	// 現在の入力種別
	InputType current_input_type_;

	std::unordered_map<std::string, std::vector<ButtonActionInfo>> button_action_map_;
	std::unordered_map<std::string, std::vector<AxisActionInfo>> axis_action_map_;

	// button action の状態（b1 = 押下中, b2 = 押した瞬間, b3 = 離した瞬間）
	std::unordered_map<std::string, int> button_action_state_;
	// axis action の状態
	std::unordered_map<std::string, float> axis_action_state_;
	
	static Input* instance_;
};

