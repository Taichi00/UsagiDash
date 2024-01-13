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
	Window* window_;
	// 参照結果
	HRESULT result_;
	// インプット
	LPDIRECTINPUT8 input_;
	// インプットデバイス
	LPDIRECTINPUTDEVICE8 key_;
	// キー情報
	BYTE keys_[KEY_MAX];
	// 前のキー情報
	BYTE olds_[KEY_MAX];

	static Input* instance_;
};

