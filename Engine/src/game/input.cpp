#include "game/input.h"
#include "engine/window.h"
#include <stdio.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

// 解放マクロ
//#define Release(X) { if ((X) != nullptr) (X)->Release(); (X) = nullptr; }

Input* Input::instance_ = nullptr;

Input::Input(Window* win)
{
	window_ = win;
	result_ = S_OK;
	input_ = nullptr;
	key_ = nullptr;

	memset(&keys_, 0, sizeof(keys_));
	memset(&olds_, 0, sizeof(olds_));

	CreateInput();
	CreateKey();
	SetKeyFormat();
	SetKeyCooperative();
}

Input::~Input()
{
	key_->Unacquire();
	key_->Release();
	input_->Release();
}

void Input::Create(Window* win)
{
	if (!instance_)
	{
		instance_ = new Input(win);
	}
}

void Input::Destroy()
{
	delete instance_;
	instance_ = nullptr;
}

// キー入力
bool Input::GetKey(UINT index)
{
	return instance_->_GetKey(index);
}

// トリガーの入力
bool Input::GetKeyDown(UINT index)
{
	return instance_->_GetKeyDown(index);
}

bool Input::_GetKey(UINT index)
{
	bool flag = false;

	auto hr = key_->GetDeviceState(sizeof(keys_), &keys_);
	if (FAILED(hr))
	{
		key_->Acquire();
	}

	if (keys_[index] & 0x80)
	{
		flag = true;
	}
	olds_[index] = keys_[index];

	return flag;
}

bool Input::_GetKeyDown(UINT index)
{
	bool flag = false;

	auto hr = key_->GetDeviceState(sizeof(keys_), &keys_);
	if (FAILED(hr))
	{
		key_->Acquire();
	}

	if (keys_[index] & 0x80 && !(olds_[index] & 0x80))
	{
		flag = true;
	}
	olds_[index] = keys_[index];

	return flag;
}

// インプットの生成
HRESULT Input::CreateInput(void)
{
	result_ = DirectInput8Create(GetModuleHandle(0), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)(&input_), NULL);

	return result_;
}

// キーデバイスの生成
HRESULT Input::CreateKey(void)
{
	result_ = input_->CreateDevice(GUID_SysKeyboard, &key_, NULL);

	return result_;
}

// キーフォーマットのセット
HRESULT Input::SetKeyFormat(void)
{
	result_ = key_->SetDataFormat(&c_dfDIKeyboard);

	return result_;
}

// キーの協調レベルのセット
HRESULT Input::SetKeyCooperative(void)
{
	result_ = key_->SetCooperativeLevel(window_->HWnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	// 入力デバイスへのアクセス権利を取得
	key_->Acquire();

	return result_;
}


