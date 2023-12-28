#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#define KEY_MAX 256	// �L�[�ő吔

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

	// �L�[����
	static bool GetKey(UINT index);
	// �g���K�[�̓���
	static bool GetKeyDown(UINT index);
	
private:
	bool _GetKey(UINT index);
	bool _GetKeyDown(UINT index);

private:
	// �C���v�b�g�̐���
	HRESULT CreateInput(void);
	// �L�[�f�o�C�X�̐���
	HRESULT CreateKey(void);
	// �L�[�t�H�[�}�b�g�̃Z�b�g
	HRESULT SetKeyFormat(void);
	// �L�[�̋������x���̃Z�b�g
	HRESULT SetKeyCooperative(void);

	// �E�B���h�E
	Window* m_pWindow;
	// �Q�ƌ���
	HRESULT m_result;
	// �C���v�b�g
	LPDIRECTINPUT8 m_input;
	// �C���v�b�g�f�o�C�X
	LPDIRECTINPUTDEVICE8 m_key;
	// �L�[���
	BYTE m_keys[KEY_MAX];
	// �O�̃L�[���
	BYTE m_olds[KEY_MAX];

	static Input* m_instance;
};

