#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "math/vec.h"
#include <unordered_map>

#define DINPUT_KEY_MAX 256 // �L�[�ő吔
#define DINPUT_PAD_BUTTON_MAX 24 // �Q�[���p�b�h�̃{�^���ő吔
#define DINPUT_MOUSE_BUTTON_MAX 3 // �}�E�X�̃{�^���ő吔

#define DINPUT_STICK_MAX 32768
#define DINPUT_STICK_DEADZONE 4587

class Window;

class DirectInput
{
public:
	enum GamepadButton
	{
		PAD_A,
		PAD_B,
		PAD_X,
		PAD_Y,
		PAD_LB,
		PAD_RB,
		PAD_LT,
		PAD_RT,
		PAD_START,
		PAD_BACK,
		PAD_LEFT_THUMB,
		PAD_RIGHT_THUMB,
		
		PAD_UP,
		PAD_DOWN,
		PAD_LEFT,
		PAD_RIGHT,
		PAD_LSTICK_UP,
		PAD_LSTICK_DOWN,
		PAD_LSTICK_LEFT,
		PAD_LSTICK_RIGHT,
		PAD_RSTICK_UP,
		PAD_RSTICK_DOWN,
		PAD_RSTICK_LEFT,
		PAD_RSTICK_RIGHT,
	};

	enum GamepadAxis
	{
		PAD_LSTICK_X,
		PAD_LSTICK_Y,
		PAD_RSTICK_X,
		PAD_RSTICK_Y,
	};

	enum GamepadType
	{
		UNKNOWN,
		SWITCH_PRO_CONTROLLER,
		DUALSHOCK4,
		DUALSENSE
	};

	enum MouseButton
	{
		MOUSE_LEFT,
		MOUSE_RIGHT,
		MOUSE_CENTER,
	};

	enum MouseAxis
	{
		MOUSE_DELTA_X,
		MOUSE_DELTA_Y,
	};

	DirectInput(Window* window);
	~DirectInput();

	void Update();

	void Refresh();

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
	bool GetGamepadButton(int index);
	// �{�^�����������u��
	bool GetGamepadButtonDown(int index);
	// �{�^���𗣂����u��
	bool GetGamepadButtonUp(int index);
	// �{�^�����s�[�g
	bool GetGamepadButtonRepeat(int index);
	// �{�^���̏�ԁib1 = ������, b2 = �������u��, b3 = �������u��, b4 = �L�[���s�[�g�j
	int GetGamepadButtonState(int index);

	// �{�^��������
	bool GetMouseButton(int index);
	// �{�^�����������u��
	bool GetMouseButtonDown(int index);
	// �{�^���𗣂����u��
	bool GetMouseButtonUp(int index);
	// �{�^�����s�[�g
	bool GetMouseButtonRepeat(int index);
	// �{�^���̏�ԁib1 = ������, b2 = �������u��, b3 = �������u��, b4 = �L�[���s�[�g�j
	int GetMouseButtonState(int index);

	// �X�e�B�b�N�̒l���擾
	Vec2 GetLStick() const;
	Vec2 GetRStick() const;

	bool IsGamepadConnected() const { return is_gamepad_connected_; }

	GamepadType GetGamepadType() const { return pad_type_; }

	// �}�E�X�J�[�\���̈ʒu���擾
	Vec2 GetCursorPosition() const;
	// �}�E�X�J�[�\���̈ړ��ʂ��擾
	Vec2 GetCursorDelta() const;

private:
	// �C���v�b�g�̐���
	HRESULT CreateInput();
	// �L�[�f�o�C�X�̐���
	HRESULT InitKeyDevice();
	// �Q�[���p�b�h�f�o�C�X�̐���
	HRESULT InitPadDevice();
	// �}�E�X�f�o�C�X�̐���
	HRESULT InitMouseDevice();

	// �Q�[���p�b�h���ڑ����ꂽ�ۂɎ��s�����R�[���o�b�N�֐�
	static BOOL CALLBACK GamepadFindCallBack(LPCDIDEVICEINSTANCE instance, LPVOID ref);

	void UpdateKey();
	void UpdatePad();
	void UpdateMouse();

	Vec2 GetStickVec(const LONG sx, const LONG sy);

	void InitPadButtonMap();

private:
	enum VendorID
	{
		VID_SWITCH_PRO_CONTROLLER = 0x057e,
		VID_DUALSHOCK4_1X = 0x054c,
		VID_DUALSHOCK4_2X = 0x054c,
		VID_DUALSENSE = 0x54c,
	};

	enum ProductID
	{
		PID_SWITCH_PRO_CONTROLLER = 0x2009,
		PID_DUALSHOCK4_1X = 0x05c4,
		PID_DUALSHOCK4_2X = 0x09cc,
		PID_DUALSENSE = 0x0ce6,
	};

	// �E�B���h�E
	Window* window_;
	// �C���v�b�g
	static LPDIRECTINPUT8 direct_input_;
	// �C���v�b�g�f�o�C�X
	LPDIRECTINPUTDEVICE8 key_device_;
	LPDIRECTINPUTDEVICE8 pad_device_;
	LPDIRECTINPUTDEVICE8 mouse_device_;
	// �R�[���o�b�N�֐��֓n���z��
	LPVOID device_info_array_;
	// �Q�[���p�b�h�̎��
	GamepadType pad_type_ = UNKNOWN;

	struct GamepadInfo
	{
		LPDIRECTINPUTDEVICE8 device;
		DWORD vendor_id;
		DWORD product_id;
	};

	// �L�[���
	int keys_[DINPUT_KEY_MAX];
	int prev_keys_[DINPUT_KEY_MAX];

	// �Q�[���p�b�h�̃{�^�����
	int pad_buttons_[DINPUT_PAD_BUTTON_MAX];
	int prev_pad_buttons_[DINPUT_PAD_BUTTON_MAX];

	// �X�e�B�b�N
	Vec2 left_stick_, prev_left_stick_;
	Vec2 right_stick_, prev_right_stick_;

	// ���s�[�g�J�n����
	int repeat_start_;
	// ���s�[�g�Ԋu
	int repeat_interval_;

	// �Q�[���p�b�h���ڑ�����Ă��邩�ǂ���
	bool is_gamepad_connected_ = false;

	// ���݂̃Q�[���p�b�h�ɂ�����{�^���}�b�v
	std::unordered_map<GamepadButton, int> pad_button_map_;
	std::unordered_map<GamepadAxis, int> pad_axis_map_;

	// �}�E�X�J�[�\���̈ʒu
	Vec2 cursor_position_;
	// �}�E�X�J�[�\���̈ړ���
	Vec2 cursor_delta_;

	// �}�E�X�̃{�^�����
	int mouse_buttons_[DINPUT_MOUSE_BUTTON_MAX];
	int prev_mouse_buttons_[DINPUT_MOUSE_BUTTON_MAX];
};