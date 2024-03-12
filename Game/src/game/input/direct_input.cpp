#include "direct_input.h"
#include "engine/window.h"
#include <stdio.h>
#include <math.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

# define PI 3.14159265359

LPDIRECTINPUT8 DirectInput::direct_input_ = NULL;

DirectInput::DirectInput(Window* window)
{
	window_ = window;
	direct_input_ = nullptr;
	key_device_ = nullptr;

	is_gamepad_connected_ = false;

	repeat_start_ = 20;
	repeat_interval_ = 3;

	// 0で初期化
	memset(&keys_, 0, sizeof(keys_));
	memset(&prev_keys_, 0, sizeof(prev_keys_));
	memset(&pad_buttons_, 0, sizeof(pad_buttons_));
	memset(&prev_pad_buttons_, 0, sizeof(prev_pad_buttons_));

	CreateInput();
	InitKeyDevice();
	InitPadDevice();
	InitMouseDevice();
}

DirectInput::~DirectInput()
{
	if (mouse_device_)
	{
		mouse_device_->Unacquire();
		mouse_device_->Release();
	}

	if (pad_device_)
	{
		pad_device_->Unacquire();
		pad_device_->Release();
	}

	if (key_device_)
	{
		key_device_->Unacquire();
		key_device_->Release();
	}

	direct_input_->Release();
}

void DirectInput::Update()
{
	UpdateKey();
	UpdatePad();
	UpdateMouse();
}

void DirectInput::Refresh()
{
	key_device_ = nullptr;
	pad_device_ = nullptr;
	mouse_device_ = nullptr;

	is_gamepad_connected_ = false;

	InitKeyDevice();
	InitPadDevice();
	InitMouseDevice();

	memset(&keys_, 0, sizeof(keys_));
	memset(&prev_keys_, 0, sizeof(prev_keys_));
	memset(&pad_buttons_, 0, sizeof(pad_buttons_));
	memset(&prev_pad_buttons_, 0, sizeof(prev_pad_buttons_));
}

bool DirectInput::GetKey(UINT index)
{
	return keys_[index];
}

bool DirectInput::GetKeyDown(UINT index)
{
	return keys_[index] && !prev_keys_[index];
}

bool DirectInput::GetKeyUp(UINT index)
{
	return !keys_[index] && prev_keys_[index];
}

bool DirectInput::GetKeyRepeat(UINT index)
{
	int k = keys_[index];

	if (k == 1) return true;
	if (k <= repeat_start_) return k == repeat_start_;
	return (k - repeat_start_) % repeat_interval_ == 0;
}

int DirectInput::GetKeyState(UINT index)
{
	return 
		(int)GetKey(index) | 
		(int)GetKeyDown(index) << 1 | 
		(int)GetKeyUp(index) << 2 | 
		(int)GetKeyRepeat(index) << 3;
}

bool DirectInput::GetGamepadButton(int index)
{
	return pad_buttons_[index];
}

bool DirectInput::GetGamepadButtonDown(int index)
{
	return pad_buttons_[index] && !prev_pad_buttons_[index];
}

bool DirectInput::GetGamepadButtonUp(int index)
{
	return !pad_buttons_[index] && prev_pad_buttons_[index];
}

bool DirectInput::GetGamepadButtonRepeat(int index)
{
	int b = pad_buttons_[index];

	if (b == 1) return true;
	if (b <= repeat_start_) return b == repeat_start_;
	return (b - repeat_start_) % repeat_interval_ == 0;
}

int DirectInput::GetGamepadButtonState(int index)
{
	return
		(int)GetGamepadButton(index) |
		(int)GetGamepadButtonDown(index) << 1 |
		(int)GetGamepadButtonUp(index) << 2 |
		(int)GetGamepadButtonRepeat(index) << 3;
}

bool DirectInput::GetMouseButton(int index)
{
	return mouse_buttons_[index];
}

bool DirectInput::GetMouseButtonDown(int index)
{
	return mouse_buttons_[index] && !prev_mouse_buttons_[index];
}

bool DirectInput::GetMouseButtonUp(int index)
{
	return !mouse_buttons_[index] && prev_mouse_buttons_[index];
}

bool DirectInput::GetMouseButtonRepeat(int index)
{
	int b = mouse_buttons_[index];

	if (b == 1) return true;
	if (b <= repeat_start_) return b == repeat_start_;
	return (b - repeat_start_) % repeat_interval_ == 0;
}

int DirectInput::GetMouseButtonState(int index)
{
	return
		(int)GetMouseButton(index) |
		(int)GetMouseButtonDown(index) << 1 |
		(int)GetMouseButtonUp(index) << 2 |
		(int)GetMouseButtonRepeat(index) << 3;
}

Vec2 DirectInput::GetLStick() const
{
	return left_stick_;
}

Vec2 DirectInput::GetRStick() const
{
	return right_stick_;
}

Vec2 DirectInput::GetCursorPosition() const
{
	return cursor_position_;
}

Vec2 DirectInput::GetCursorDelta() const
{
	return cursor_delta_;
}

HRESULT DirectInput::CreateInput()
{
	return DirectInput8Create(GetModuleHandle(0), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)(&direct_input_), NULL);
}

HRESULT DirectInput::InitKeyDevice()
{
	// デバイスの生成
	auto hr = direct_input_->CreateDevice(GUID_SysKeyboard, &key_device_, NULL);
	if (FAILED(hr)) 
		return hr;

	// フォーマットの設定
	hr = key_device_->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(hr)) 
		return hr;

	// 協調レベルの設定
	hr = key_device_->SetCooperativeLevel(window_->HWnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr)) 
		return hr;

	// 入力デバイスへのアクセス権利を取得
	hr = key_device_->Acquire();

	return hr;
}

HRESULT DirectInput::InitPadDevice()
{
	GamepadInfo info = {};

	// 接続されているゲームパッドを列挙
	auto hr = direct_input_->EnumDevices(
		DI8DEVCLASS_GAMECTRL,
		GamepadFindCallBack,
		&info,
		DIEDFL_ATTACHEDONLY
	);

	pad_device_ = info.device;
	
	if (FAILED(hr) || !pad_device_)
		return hr;

	// フォーマットの設定
	hr = pad_device_->SetDataFormat(&c_dfDIJoystick);
	if (FAILED(hr))
		return hr;

	// 軸モードを絶対値モードとして設定
	{
		DIPROPDWORD prop;
		prop.diph.dwSize = sizeof(prop);
		prop.diph.dwHeaderSize = sizeof(prop.diph);
		prop.diph.dwHow = DIPH_DEVICE;
		prop.diph.dwObj = 0;
		prop.dwData = DIPROPAXISMODE_ABS;

		hr = pad_device_->SetProperty(DIPROP_AXISMODE, &prop.diph);
		if (FAILED(hr))
			return hr;
	}

	// 協調モードの設定
	hr = pad_device_->SetCooperativeLevel(window_->HWnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr))
		return hr;

	// 制御開始
	hr = pad_device_->Acquire();
	if (FAILED(hr))
		return hr;

	// ポーリング開始
	hr = pad_device_->Poll();
	if (FAILED(hr))
		return hr;

	// ゲームパッドの種別を特定
	if (info.vendor_id == VID_SWITCH_PRO_CONTROLLER && info.product_id == PID_SWITCH_PRO_CONTROLLER)
	{
		pad_type_ = SWITCH_PRO_CONTROLLER;
	}
	else if ((info.vendor_id == VID_DUALSHOCK4_1X && info.product_id == PID_DUALSHOCK4_1X) ||
		     (info.vendor_id == VID_DUALSHOCK4_2X && info.product_id == PID_DUALSHOCK4_2X))
	{
		pad_type_ = DUALSHOCK4;
	}
	else if (info.vendor_id == VID_DUALSENSE && info.product_id == PID_DUALSENSE)
	{
		pad_type_ = DUALSENSE;
	}
	else
	{
		pad_type_ = UNKNOWN;
	}

	InitPadButtonMap();

	is_gamepad_connected_ = true;

	return S_OK;
}

HRESULT DirectInput::InitMouseDevice()
{
	// デバイスの生成
	auto hr = direct_input_->CreateDevice(GUID_SysMouse, &mouse_device_, NULL);
	if (FAILED(hr))
		return hr;

	// フォーマットの設定
	hr = mouse_device_->SetDataFormat(&c_dfDIMouse);
	if (FAILED(hr))
		return hr;

	// 相対値モードに設定
	DIPROPDWORD prop;
	prop.diph.dwSize = sizeof(prop);
	prop.diph.dwHeaderSize = sizeof(prop.diph);
	prop.diph.dwHow = DIPH_DEVICE;
	prop.diph.dwObj = 0;
	prop.dwData = DIPROPAXISMODE_REL;

	hr = mouse_device_->SetProperty(DIPROP_AXISMODE, &prop.diph);
	if (FAILED(hr))
		return hr;

	// 協調レベルの設定
	hr = mouse_device_->SetCooperativeLevel(window_->HWnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr))
		return hr;

	// 入力デバイスへのアクセス権利を取得
	hr = mouse_device_->Acquire();

	return hr;
}

BOOL DirectInput::GamepadFindCallBack(LPCDIDEVICEINSTANCE instance, LPVOID ref)
{
	auto info = (GamepadInfo*)ref;

	// デバイスの生成
	direct_input_->CreateDevice(instance->guidInstance, &info->device, NULL);

	// vendorID と productID の取得
	DIPROPDWORD prop;
	prop.diph.dwSize = sizeof(prop);
	prop.diph.dwHeaderSize = sizeof(prop.diph);
	prop.diph.dwObj = 0;
	prop.diph.dwHow = DIPH_DEVICE;

	info->device->GetProperty(DIPROP_VIDPID, &prop.diph);

	info->vendor_id = LOWORD(prop.dwData);
	info->product_id = HIWORD(prop.dwData);

	return DIENUM_STOP;
}

void DirectInput::UpdateKey()
{
	if (!key_device_)
		return;

	// 前フレームのキー情報をコピー
	memcpy(&prev_keys_, &keys_, sizeof(prev_keys_));

	BYTE keys[DINPUT_KEY_MAX] = {};

	// 現在のキー情報を取得
	auto hr = key_device_->GetDeviceState(sizeof(keys), &keys);
	if (FAILED(hr))
	{
		// フォーカスが失われているので再び取得
		key_device_->Acquire();

		hr = key_device_->GetDeviceState(sizeof(keys), &keys);
		if (FAILED(hr))
			return;
	}

	// キー情報を更新
	for (int i = 0; i < DINPUT_KEY_MAX; i++)
	{
		keys_[i] = keys[i] & 0x80 ? keys_[i] + 1 : 0;
	}
}

void DirectInput::UpdatePad()
{
	if (!pad_device_)
		return;

	// 前フレームの入力情報をコピー
	memcpy(&prev_pad_buttons_, &pad_buttons_, sizeof(prev_pad_buttons_));

	DIJOYSTATE pad_state = {};

	//  現在の入力情報を取得
	auto hr = pad_device_->GetDeviceState(sizeof(pad_state), &pad_state);
	if (FAILED(hr))
	{
		pad_device_->Acquire();

		hr = pad_device_->GetDeviceState(sizeof(pad_state), &pad_state);
		if (FAILED(hr))
			return;
	}
	
	// ボタンの入力を取得
	for (auto& map : pad_button_map_)
	{
		pad_buttons_[map.first] = pad_state.rgbButtons[map.second] & 0x80 ? pad_buttons_[map.second] + 1 : 0;
	}
	
	// 十字ボタンの入力を取得
	if (pad_state.rgdwPOV[0] != 0xFFFFFFFF)
	{
		float rad = pad_state.rgdwPOV[0] / 100.0f * (float)PI / 180.0f;
		float x = sinf(rad);
		float y = cosf(rad);

		pad_buttons_[PAD_LEFT]  = x < 0 ? pad_buttons_[PAD_LEFT]  + 1 : 0;
		pad_buttons_[PAD_RIGHT] = x > 0 ? pad_buttons_[PAD_RIGHT] + 1 : 0;
		pad_buttons_[PAD_UP]    = y > 0 ? pad_buttons_[PAD_UP]    + 1 : 0;
		pad_buttons_[PAD_DOWN]  = y < 0 ? pad_buttons_[PAD_DOWN]  + 1 : 0;
	}
	else
	{
		pad_buttons_[PAD_LEFT]  = 0;
		pad_buttons_[PAD_RIGHT] = 0;
		pad_buttons_[PAD_UP]    = 0;
		pad_buttons_[PAD_DOWN]  = 0;
	}
	
	// axis 要素へのポインタを取得
	LONG* axis_ptr[6] =
	{
		&pad_state.lX,
		&pad_state.lY,
		&pad_state.lZ,
		&pad_state.lRx,
		&pad_state.lRy,
		&pad_state.lRz,
	};

	// スティックの入力を取得
	left_stick_  = GetStickVec(*axis_ptr[pad_axis_map_[PAD_LSTICK_X]], *axis_ptr[pad_axis_map_[PAD_LSTICK_Y]]);
	right_stick_ = GetStickVec(*axis_ptr[pad_axis_map_[PAD_RSTICK_X]], *axis_ptr[pad_axis_map_[PAD_RSTICK_Y]]);
	
	pad_buttons_[PAD_LSTICK_LEFT]  = left_stick_.x  < 0 ? pad_buttons_[PAD_LSTICK_LEFT]  + 1 : 0;
	pad_buttons_[PAD_LSTICK_RIGHT] = left_stick_.x  > 0 ? pad_buttons_[PAD_LSTICK_RIGHT] + 1 : 0;
	pad_buttons_[PAD_LSTICK_UP]    = left_stick_.y  > 0 ? pad_buttons_[PAD_LSTICK_UP]    + 1 : 0;
	pad_buttons_[PAD_LSTICK_DOWN]  = left_stick_.y  < 0 ? pad_buttons_[PAD_LSTICK_DOWN]  + 1 : 0;
	
	pad_buttons_[PAD_RSTICK_LEFT]  = right_stick_.x < 0 ? pad_buttons_[PAD_RSTICK_LEFT]  + 1 : 0;
	pad_buttons_[PAD_RSTICK_RIGHT] = right_stick_.x > 0 ? pad_buttons_[PAD_RSTICK_RIGHT] + 1 : 0;
	pad_buttons_[PAD_RSTICK_UP]    = right_stick_.y > 0 ? pad_buttons_[PAD_RSTICK_UP]    + 1 : 0;
	pad_buttons_[PAD_RSTICK_DOWN]  = right_stick_.y < 0 ? pad_buttons_[PAD_RSTICK_DOWN]  + 1 : 0;
}

void DirectInput::UpdateMouse()
{
	if (!mouse_device_)
		return;

	// 前フレームの入力情報をコピー
	memcpy(&prev_mouse_buttons_, &mouse_buttons_, sizeof(prev_mouse_buttons_));

	DIMOUSESTATE state;

	// マウスの状態を取得
	auto hr = mouse_device_->GetDeviceState(sizeof(state), &state);
	if (FAILED(hr))
	{
		mouse_device_->Acquire();

		hr = mouse_device_->GetDeviceState(sizeof(state), &state);
		if (FAILED(hr))
			return;
	}

	// マウスカーソルの位置を取得
	POINT p = {};
	if (GetCursorPos(&p))
	{
		if (ScreenToClient(window_->HWnd(), &p))
		{
			cursor_position_ = Vec2((float)p.x, (float)p.y);
		}
	}

	// マウスカーソルの移動量を取得
	cursor_delta_ = Vec2((float)state.lX, (float)-state.lY);

	// ボタンの入力情報を取得
	for (int i = 0; i < DINPUT_MOUSE_BUTTON_MAX; i++)
	{
		mouse_buttons_[i] = state.rgbButtons[i] & 0x80 ? mouse_buttons_[i] + 1 : 0;
	}
}

Vec2 DirectInput::GetStickVec(const LONG sx, const LONG sy)
{
	auto ssx = sx - DINPUT_STICK_MAX;
	auto ssy = sy - DINPUT_STICK_MAX;

	auto len = std::sqrt(ssx * ssx + ssy * ssy);

	Vec2 v = Vec2::Zero();
	if (len >= DINPUT_STICK_DEADZONE)
	{
		float max = DINPUT_STICK_MAX * 0.7f;
		auto x = std::abs(ssx);
		auto y = std::abs(ssy);

		v.x = ssx < 0 ? -x / max : x / (max - 1);
		v.y = ssy < 0 ? -y / max : y / (max - 1); 

		v.x = std::min(std::max(v.x, -1.0f), 1.0f);
		v.y = -std::min(std::max(v.y, -1.0f), 1.0f);
	}
	return v;
}

void DirectInput::InitPadButtonMap()
{
	switch (pad_type_)
	{
	case SWITCH_PRO_CONTROLLER:
		pad_button_map_ =
		{
			{ PAD_A, 1 },
			{ PAD_B, 0 },
			{ PAD_X, 3 },
			{ PAD_Y, 2 },
			{ PAD_LB, 4 },
			{ PAD_RB, 5 },
			{ PAD_LT, 6 },
			{ PAD_RT, 7 },
			{ PAD_START, 9 },
			{ PAD_BACK, 8 },
			{ PAD_LEFT_THUMB, 10 },
			{ PAD_RIGHT_THUMB, 11 },
		};
		pad_axis_map_ =
		{
			{ PAD_LSTICK_X, 0 },
			{ PAD_LSTICK_Y, 1 },
			{ PAD_RSTICK_X, 3 },
			{ PAD_RSTICK_Y, 4 },
		};
		break;

	case DUALSHOCK4:
	case DUALSENSE:
		pad_button_map_ =
		{
			{ PAD_A, 1 },
			{ PAD_B, 0 },
			{ PAD_X, 3 },
			{ PAD_Y, 2 },
			{ PAD_LB, 4 },
			{ PAD_RB, 5 },
			{ PAD_LT, 6 },
			{ PAD_RT, 7 },
			{ PAD_START, 9 },
			{ PAD_BACK, 8 },
			{ PAD_LEFT_THUMB, 10 },
			{ PAD_RIGHT_THUMB, 11 },
		};
		pad_axis_map_ =
		{
			{ PAD_LSTICK_X, 0 },
			{ PAD_LSTICK_Y, 1 },
			{ PAD_RSTICK_X, 2 },
			{ PAD_RSTICK_Y, 5 },
		};
		break;

	case UNKNOWN:
		pad_button_map_ =
		{
			{ PAD_A, 0 },
			{ PAD_B, 1 },
			{ PAD_X, 2 },
			{ PAD_Y, 3 },
			{ PAD_LB, 4 },
			{ PAD_RB, 5 },
			{ PAD_LT, 6 },
			{ PAD_RT, 7 },
			{ PAD_START, 8 },
			{ PAD_BACK, 9 },
			{ PAD_LEFT_THUMB, 10 },
			{ PAD_RIGHT_THUMB, 11 },
		};
		pad_axis_map_ =
		{
			{ PAD_LSTICK_X, 0 },
			{ PAD_LSTICK_Y, 1 },
			{ PAD_RSTICK_X, 3 },
			{ PAD_RSTICK_Y, 4 },
		};
		break;
	}
}
