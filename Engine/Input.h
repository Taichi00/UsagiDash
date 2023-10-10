#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#define KEY_MAX 256	// キー最大数

class Window;

class Input
{
public:
	Input(Window* win);
	~Input();

	// キー入力
	bool CheckKey(UINT index);
	// トリガーの入力
	bool TriggerKey(UINT index);
	
private:
	// インプットの生成
	HRESULT CreateInput(void);
	// キーデバイスの生成
	HRESULT CreateKey(void);
	// キーフォーマットのセット
	HRESULT SetKeyFormat(void);
	// キーの協調レベルのセット
	HRESULT SetKeyCooperative(void);

	// ウィンドウ
	Window* m_pWindow;
	// 参照結果
	HRESULT m_result;
	// インプット
	LPDIRECTINPUT8 m_input;
	// インプットデバイス
	LPDIRECTINPUTDEVICE8 m_key;
	// キー情報
	BYTE m_keys[KEY_MAX];
	// 前のキー情報
	BYTE m_olds[KEY_MAX];
};