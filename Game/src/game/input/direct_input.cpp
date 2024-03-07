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

	repeat_start_ = 20;
	repeat_interval_ = 3;

	// 0�ŏ�����
	memset(&keys_, 0, sizeof(keys_));
	memset(&prev_keys_, 0, sizeof(prev_keys_));
	memset(&buttons_, 0, sizeof(buttons_));
	memset(&prev_buttons_, 0, sizeof(prev_buttons_));

	CreateInput();
	InitKeyDevice();
	InitPadDevice();
}

DirectInput::~DirectInput()
{
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

bool DirectInput::GetButton(int index)
{
	return buttons_[index];
}

bool DirectInput::GetButtonDown(int index)
{
	return buttons_[index] && !prev_buttons_[index];
}

bool DirectInput::GetButtonUp(int index)
{
	return !buttons_[index] && prev_buttons_[index];
}

bool DirectInput::GetButtonRepeat(int index)
{
	int b = buttons_[index];

	if (b == 1) return true;
	if (b <= repeat_start_) return b == repeat_start_;
	return (b - repeat_start_) % repeat_interval_ == 0;
}

int DirectInput::GetButtonState(int index)
{
	return
		(int)GetButton(index) |
		(int)GetButtonDown(index) << 1 |
		(int)GetButtonUp(index) << 2 |
		(int)GetButtonRepeat(index) << 3;
}

Vec2 DirectInput::GetLStick() const
{
	return left_stick_;
}

Vec2 DirectInput::GetRStick() const
{
	return right_stick_;
}

HRESULT DirectInput::CreateInput()
{
	return DirectInput8Create(GetModuleHandle(0), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)(&direct_input_), NULL);
}

HRESULT DirectInput::InitKeyDevice()
{
	// �f�o�C�X�̐���
	auto hr = direct_input_->CreateDevice(GUID_SysKeyboard, &key_device_, NULL);
	if (FAILED(hr)) 
		return hr;

	// �t�H�[�}�b�g�̐ݒ�
	hr = key_device_->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(hr)) 
		return hr;

	// �������x���̐ݒ�
	hr = key_device_->SetCooperativeLevel(window_->HWnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr)) 
		return hr;

	// ���̓f�o�C�X�ւ̃A�N�Z�X�������擾
	hr = key_device_->Acquire();

	return hr;
}

HRESULT DirectInput::InitPadDevice()
{
	// �ڑ�����Ă���Q�[���p�b�h���
	auto hr = direct_input_->EnumDevices(
		DI8DEVCLASS_GAMECTRL,
		GamepadFindCallBack,
		&pad_device_,
		DIEDFL_ATTACHEDONLY
	);
	if (FAILED(hr) || !pad_device_)
		return hr;

	// �t�H�[�}�b�g�̐ݒ�
	hr = pad_device_->SetDataFormat(&c_dfDIJoystick);
	if (FAILED(hr))
		return hr;

	// �����[�h���Βl���[�h�Ƃ��Đݒ�
	{
		DIPROPDWORD prop;
		ZeroMemory(&prop, sizeof(prop));
		prop.diph.dwSize = sizeof(prop);
		prop.diph.dwHeaderSize = sizeof(prop.diph);
		prop.diph.dwHow = DIPH_DEVICE;
		prop.diph.dwObj = 0;
		prop.dwData = DIPROPAXISMODE_ABS;

		hr = pad_device_->SetProperty(DIPROP_AXISMODE, &prop.diph);
		if (FAILED(hr))
			return hr;
	}

	// ���̒l�͈̔͐ݒ�
	/*{
		DIPROPRANGE prop;
		ZeroMemory(&prop, sizeof(prop));
		prop.diph.dwSize = sizeof(prop);
		prop.diph.dwHeaderSize = sizeof(prop.diph);
		prop.diph.dwHow = DIPH_BYOFFSET;
		prop.diph.dwObj = DIJOFS_X;
		prop.lMin = -DINPUT_STICK_MAX;
		prop.lMax = DINPUT_STICK_MAX;

		hr = pad_device_->SetProperty(DIPROP_RANGE, &prop.diph);
		if (FAILED(hr))
			return hr;

		prop.diph.dwObj = DIJOFS_Y;
		hr = pad_device_->SetProperty(DIPROP_RANGE, &prop.diph);
		if (FAILED(hr))
			return hr;
	}*/

	// �������[�h�̐ݒ�
	hr = pad_device_->SetCooperativeLevel(window_->HWnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr))
		return hr;

	// ����J�n
	hr = pad_device_->Acquire();
	if (FAILED(hr))
		return hr;

	// �|�[�����O�J�n
	hr = pad_device_->Poll();
	if (FAILED(hr))
		return hr;

	is_gamepad_connected_ = true;

	return S_OK;
}

BOOL DirectInput::GamepadFindCallBack(LPCDIDEVICEINSTANCE instance, LPVOID ref)
{
	direct_input_->CreateDevice(instance->guidInstance, (LPDIRECTINPUTDEVICE8*)ref, NULL);
	
	return DIENUM_STOP;
}

void DirectInput::UpdateKey()
{
	if (!key_device_)
		return;

	// �O�t���[���̃L�[�����R�s�[
	memcpy(&prev_keys_, &keys_, sizeof(prev_keys_));

	BYTE keys[DINPUT_KEY_MAX] = {};

	// ���݂̃L�[�����擾
	auto hr = key_device_->GetDeviceState(sizeof(keys), &keys);
	if (FAILED(hr))
	{
		// �t�H�[�J�X�������Ă���̂ōĂю擾
		key_device_->Acquire();

		hr = key_device_->GetDeviceState(sizeof(keys), &keys);
		if (FAILED(hr))
			return;
	}

	// �L�[�����X�V
	for (int i = 0; i < DINPUT_KEY_MAX; i++)
	{
		if (keys[i] & 0x80)
		{
			keys_[i]++;
		}
		else
		{
			keys_[i] = 0;
		}
	}
}

void DirectInput::UpdatePad()
{
	if (!pad_device_)
		return;

	// �O�t���[���̓��͏����R�s�[
	memcpy(&prev_buttons_, &buttons_, sizeof(prev_buttons_));

	DIJOYSTATE pad_data = {};

	//  ���݂̓��͏����擾
	auto hr = pad_device_->GetDeviceState(sizeof(pad_data), &pad_data);
	if (FAILED(hr))
	{
		pad_device_->Acquire();

		hr = pad_device_->GetDeviceState(sizeof(pad_data), &pad_data);
		if (FAILED(hr))
			return;
	}
	
	// �{�^��10�܂ŁiA~BACK�j�̓��͂��擾
	for (int i = 0; i < 10; i++)
	{
		buttons_[i] = pad_data.rgbButtons[i] & 0x80 ? buttons_[i] + 1 : 0;
	}

	// �\���{�^���̓��͂��擾
	if (pad_data.rgdwPOV[0] != 0xFFFFFFFF)
	{
		float rad = pad_data.rgdwPOV[0] / 100.0f * (float)PI / 180.0f;
		float x = sinf(rad);
		float y = cosf(rad);

		buttons_[LEFT] = x < 0 ? buttons_[LEFT] + 1 : 0;
		buttons_[RIGHT] = x > 0 ? buttons_[RIGHT] + 1 : 0;
		buttons_[UP] = y > 0 ? buttons_[UP] + 1 : 0;
		buttons_[DOWN] = y < 0 ? buttons_[DOWN] + 1 : 0;
	}
	else
	{
		buttons_[LEFT] = 0;
		buttons_[RIGHT] = 0;
		buttons_[UP] = 0;
		buttons_[DOWN] = 0;
	}

	// �X�e�B�b�N�̓��͂��擾
	left_stick_ = GetStickVec(pad_data.lX, pad_data.lY);
	right_stick_ = GetStickVec(pad_data.lRx, pad_data.lRy);
	printf("%s\n", left_stick_.GetString().c_str());
	buttons_[LSTICK_LEFT]  = left_stick_.x < 0 ? buttons_[LSTICK_LEFT]  + 1 : 0;
	buttons_[LSTICK_RIGHT] = left_stick_.x > 0 ? buttons_[LSTICK_RIGHT] + 1 : 0;
	buttons_[LSTICK_UP]    = left_stick_.y > 0 ? buttons_[LSTICK_UP]    + 1 : 0;
	buttons_[LSTICK_DOWN]  = left_stick_.y < 0 ? buttons_[LSTICK_DOWN]  + 1 : 0;

	buttons_[RSTICK_LEFT]  = right_stick_.x < 0 ? buttons_[RSTICK_LEFT]  + 1 : 0;
	buttons_[RSTICK_RIGHT] = right_stick_.x > 0 ? buttons_[RSTICK_RIGHT] + 1 : 0;
	buttons_[RSTICK_UP]    = right_stick_.y > 0 ? buttons_[RSTICK_UP]    + 1 : 0;
	buttons_[RSTICK_DOWN]  = right_stick_.y < 0 ? buttons_[RSTICK_DOWN]  + 1 : 0;
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
