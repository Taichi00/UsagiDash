#include "Input.h"
#include "Window.h"
#include <stdio.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

// 解放マクロ
//#define Release(X) { if ((X) != nullptr) (X)->Release(); (X) = nullptr; }

Input::Input(Window* win)
{
	m_pWindow = win;
	m_result = S_OK;
	m_input = nullptr;
	m_key = nullptr;

	memset(&m_keys, 0, sizeof(m_keys));
	memset(&m_olds, 0, sizeof(m_olds));

	CreateInput();
	CreateKey();
	SetKeyFormat();
	SetKeyCooperative();
}

Input::~Input()
{
	m_key->Unacquire();
	m_key->Release();
	m_input->Release();
}

// キー入力
bool Input::CheckKey(UINT index)
{
	bool flag = false;
	
	auto hr = m_key->GetDeviceState(sizeof(m_keys), &m_keys);
	if (FAILED(hr))
	{
		m_key->Acquire();
	}

	if (m_keys[index] & 0x80)
	{
		flag = true;
	}
	m_olds[index] = m_keys[index];

	return flag;
}

// トリガーの入力
bool Input::TriggerKey(UINT index)
{
	bool flag = false;
	
	auto hr = m_key->GetDeviceState(sizeof(m_keys), &m_keys);
	if (FAILED(hr))
	{
		m_key->Acquire();
	}

	if (m_keys[index] & 0x80 && !(m_olds[index] & 0x80))
	{
		flag = true;
	}
	m_olds[index] = m_keys[index];

	return flag;
}

// インプットの生成
HRESULT Input::CreateInput(void)
{
	m_result = DirectInput8Create(GetModuleHandle(0), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)(&m_input), NULL);

	return m_result;
}

// キーデバイスの生成
HRESULT Input::CreateKey(void)
{
	m_result = m_input->CreateDevice(GUID_SysKeyboard, &m_key, NULL);

	return m_result;
}

// キーフォーマットのセット
HRESULT Input::SetKeyFormat(void)
{
	m_result = m_key->SetDataFormat(&c_dfDIKeyboard);

	return m_result;
}

// キーの協調レベルのセット
HRESULT Input::SetKeyCooperative(void)
{
	m_result = m_key->SetCooperativeLevel(m_pWindow->HWnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	// 入力デバイスへのアクセス権利を取得
	m_key->Acquire();

	return m_result;
}


