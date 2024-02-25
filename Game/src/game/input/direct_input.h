#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#define KEY_MAX 256	// キー最大数

class Window;

class DirectInput
{
public:
	DirectInput(Window* window);
	~DirectInput();

	void Update();

	// 押下中
	bool GetKey(UINT index);
	// 押した瞬間
	bool GetKeyDown(UINT index); 
	// 離した瞬間
	bool GetKeyUp(UINT index);

	// キーの状態（b1 = 押下中, b2 = 押した瞬間, b3 = 離した瞬間）
	int GetKeyState(UINT index);

private:
	// インプットの生成
	HRESULT CreateInput(void);
	// キーデバイスの生成
	HRESULT CreateKey(void);
	// キーフォーマットのセット
	HRESULT SetKeyFormat(void);
	// キーの協調レベルのセット
	HRESULT SetKeyCooperative(void);

private:
	// ウィンドウ
	Window* window_;
	// 参照結果
	HRESULT result_;
	// インプット
	LPDIRECTINPUT8 direct_input_;
	// インプットデバイス
	LPDIRECTINPUTDEVICE8 direct_input_device_;
	// キー情報
	BYTE keys_[KEY_MAX];
	// 前のキー情報
	BYTE prev_keys_[KEY_MAX];
};