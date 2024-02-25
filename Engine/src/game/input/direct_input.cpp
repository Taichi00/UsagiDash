#include "direct_input.h"
#include "engine/window.h"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

DirectInput::DirectInput(Window* window)
{
	window_ = window;
	result_ = S_OK;
	direct_input_ = nullptr;
	direct_input_device_ = nullptr;

	// 0で初期化
	memset(&keys_, 0, sizeof(keys_));
	memset(&prev_keys_, 0, sizeof(prev_keys_));

	CreateInput();
	CreateKey();
	SetKeyFormat();
	SetKeyCooperative();
}

DirectInput::~DirectInput()
{
	direct_input_device_->Unacquire();
	direct_input_device_->Release();
	direct_input_->Release();
}

void DirectInput::Update()
{
	// 前フレームのキー情報をコピー
	memcpy(&prev_keys_, &keys_, sizeof(prev_keys_));

	// 現在のキー情報を取得
	auto hr = direct_input_device_->GetDeviceState(sizeof(keys_), &keys_);
	if (FAILED(hr))
	{
		// フォーカスが失われているので再び取得
		direct_input_device_->Acquire();
		direct_input_device_->GetDeviceState(sizeof(keys_), &keys_);
	}
}

bool DirectInput::GetKey(UINT index)
{
	return keys_[index] & 0x80;
}

bool DirectInput::GetKeyDown(UINT index)
{
	return keys_[index] & 0x80 && !(prev_keys_[index] & 0x80);
}

bool DirectInput::GetKeyUp(UINT index)
{
	return !(keys_[index] & 0x80) && prev_keys_[index] & 0x80;
}

int DirectInput::GetKeyState(UINT index)
{
	return GetKey(index) | GetKeyDown(index) << 1 | GetKeyUp(index) << 2;
}

HRESULT DirectInput::CreateInput(void)
{
	result_ = DirectInput8Create(GetModuleHandle(0), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)(&direct_input_), NULL);

	return result_;
}

HRESULT DirectInput::CreateKey(void)
{
	result_ = direct_input_->CreateDevice(GUID_SysKeyboard, &direct_input_device_, NULL);

	return result_;
}

HRESULT DirectInput::SetKeyFormat(void)
{
	result_ = direct_input_device_->SetDataFormat(&c_dfDIKeyboard);

	return result_;
}

HRESULT DirectInput::SetKeyCooperative(void)
{
	result_ = direct_input_device_->SetCooperativeLevel(window_->HWnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	// 入力デバイスへのアクセス権利を取得
	direct_input_device_->Acquire();

	return result_;
}
