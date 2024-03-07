#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "math/vec.h"

#define DINPUT_KEY_MAX 256 // �L�[�ő吔
#define DINPUT_BUTTON_MAX 24 // �Q�[���p�b�h�̃{�^���ő吔
#define DINPUT_STICK_MAX 32768
#define DINPUT_STICK_DEADZONE 4587

class Window;

class DirectInput
{
public:
	enum Button
	{
		A,
		B,
		X,
		Y,
		LB,
		RB,
		LT,
		RT,
		START,
		BACK,

		UP,
		DOWN,
		LEFT,
		RIGHT,
		LEFT_THUMB,
		RIGHT_THUMB,
		LSTICK_UP,
		LSTICK_DOWN,
		LSTICK_LEFT,
		LSTICK_RIGHT,
		RSTICK_UP,
		RSTICK_DOWN,
		RSTICK_LEFT,
		RSTICK_RIGHT,
	};

	DirectInput(Window* window);
	~DirectInput();

	void Update();

	// ������
	bool GetKey(UINT index);
	// �������u��
	bool GetKeyDown(UINT index); 
	// �������u��
	bool GetKeyUp(UINT index);
	// �L�[���s�[�g
	bool GetKeyRepeat(UINT index);
	// �L�[�̏�ԁib1 = ������, b2 = �������u��, b3 = �������u��, b4 = �L�[���s�[�g�j
	int GetKeyState(UINT index);

	// �{�^��������
	bool GetButton(int index);
	// �{�^�����������u��
	bool GetButtonDown(int index);
	// �{�^���𗣂����u��
	bool GetButtonUp(int index);
	// �{�^�����s�[�g
	bool GetButtonRepeat(int index);
	// �{�^���̏�ԁib1 = ������, b2 = �������u��, b3 = �������u��, b4 = �L�[���s�[�g�j
	int GetButtonState(int index);

	// �X�e�B�b�N�̒l���擾
	Vec2 GetLStick() const;
	Vec2 GetRStick() const;

	bool IsGamepadConnected() const { return is_gamepad_connected_; }

private:
	// �C���v�b�g�̐���
	HRESULT CreateInput();
	// �L�[�f�o�C�X�̐���
	HRESULT InitKeyDevice();
	// �Q�[���p�b�h�f�o�C�X�̐���
	HRESULT InitPadDevice();

	// �Q�[���p�b�h���ڑ����ꂽ�ۂɎ��s�����R�[���o�b�N�֐�
	static BOOL CALLBACK GamepadFindCallBack(LPCDIDEVICEINSTANCE instance, LPVOID ref);

	void UpdateKey();
	void UpdatePad();

	Vec2 GetStickVec(const LONG sx, const LONG sy);

private:
	// �E�B���h�E
	Window* window_;
	// �C���v�b�g
	static LPDIRECTINPUT8 direct_input_;
	// �C���v�b�g�f�o�C�X
	LPDIRECTINPUTDEVICE8 key_device_;
	LPDIRECTINPUTDEVICE8 pad_device_;
	// �R�[���o�b�N�֐��֓n���z��
	LPVOID device_info_array_;

	// �L�[���
	int keys_[DINPUT_KEY_MAX];
	int prev_keys_[DINPUT_KEY_MAX];

	// �Q�[���p�b�h�̃{�^�����
	int buttons_[DINPUT_BUTTON_MAX];
	int prev_buttons_[DINPUT_BUTTON_MAX];

	// �X�e�B�b�N
	Vec2 left_stick_, prev_left_stick_;
	Vec2 right_stick_, prev_right_stick_;

	// ���s�[�g�J�n����
	int repeat_start_;
	// ���s�[�g�Ԋu
	int repeat_interval_;

	// �Q�[���p�b�h���ڑ�����Ă��邩�ǂ���
	bool is_gamepad_connected_ = false;
};