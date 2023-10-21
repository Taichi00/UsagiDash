#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#define KEY_MAX 256	// キー最大数

class Window;

class Input
{
private:
	Input(Window* win);
	~Input();

public:
	Input(const Input&) = delete;
	Input& operator =(const Input&) = delete;
	Input(Input&&) = delete;
	Input& operator =(Input&&) = delete;

	static void Create(Window* win);
	static void Destroy();

	// キー入力
	static bool GetKey(UINT index);
	// トリガーの入力
	static bool GetKeyDown(UINT index);
	
private:
	bool _GetKey(UINT index);
	bool _GetKeyDown(UINT index);

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

	static Input* m_instance;
};

