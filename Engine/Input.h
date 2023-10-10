#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#define KEY_MAX 256	// �L�[�ő吔

class Window;

class Input
{
public:
	Input(Window* win);
	~Input();

	// �L�[����
	bool CheckKey(UINT index);
	// �g���K�[�̓���
	bool TriggerKey(UINT index);
	
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
};