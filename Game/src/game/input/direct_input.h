#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#define KEY_MAX 256	// �L�[�ő吔

class Window;

class DirectInput
{
public:
	DirectInput(Window* window);
	~DirectInput();

	void Update();

	// ������
	bool GetKey(UINT index);
	// �������u��
	bool GetKeyDown(UINT index); 
	// �������u��
	bool GetKeyUp(UINT index);

	// �L�[�̏�ԁib1 = ������, b2 = �������u��, b3 = �������u�ԁj
	int GetKeyState(UINT index);

private:
	// �C���v�b�g�̐���
	HRESULT CreateInput(void);
	// �L�[�f�o�C�X�̐���
	HRESULT CreateKey(void);
	// �L�[�t�H�[�}�b�g�̃Z�b�g
	HRESULT SetKeyFormat(void);
	// �L�[�̋������x���̃Z�b�g
	HRESULT SetKeyCooperative(void);

private:
	// �E�B���h�E
	Window* window_;
	// �Q�ƌ���
	HRESULT result_;
	// �C���v�b�g
	LPDIRECTINPUT8 direct_input_;
	// �C���v�b�g�f�o�C�X
	LPDIRECTINPUTDEVICE8 direct_input_device_;
	// �L�[���
	BYTE keys_[KEY_MAX];
	// �O�̃L�[���
	BYTE prev_keys_[KEY_MAX];
};