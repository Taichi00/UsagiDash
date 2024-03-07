#pragma once

#define NOMINMAX
#include <windows.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class DirectInput;
class XInput;
class Window;

class Input
{
public:
	// ���͎��
	enum class InputType
	{
		KEYBOARD,
		GAMEPAD,
		MOUSE
	};

	enum Button
	{
		KEY_ESCAPE,
		KEY_RETURN,
		KEY_SPACE,
		KEY_LSHIFT,
		KEY_RSHIFT,

		KEY_UP,
		KEY_DOWN,
		KEY_LEFT,
		KEY_RIGHT,

		KEY_1,
		KEY_2,
		KEY_3,
		KEY_4,
		KEY_5,
		KEY_6,
		KEY_7,
		KEY_8,
		KEY_9,
		KEY_0,

		KEY_A,
		KEY_B,
		KEY_C,
		KEY_D,
		KEY_E,
		KEY_F,
		KEY_G,
		KEY_H,
		KEY_I,
		KEY_J,
		KEY_K,
		KEY_L,
		KEY_M,
		KEY_N,
		KEY_O,
		KEY_P,
		KEY_Q,
		KEY_R,
		KEY_S,
		KEY_T,
		KEY_U,
		KEY_V,
		KEY_W,
		KEY_X,
		KEY_Y,
		KEY_Z,

		PAD_UP,
		PAD_DOWN,
		PAD_LEFT,
		PAD_RIGHT,

		PAD_LB,
		PAD_RB,
		
		PAD_A,
		PAD_B,
		PAD_X,
		PAD_Y,

		PAD_LSTICK_UP,
		PAD_LSTICK_DOWN,
		PAD_LSTICK_LEFT,
		PAD_LSTICK_RIGHT,

		PAD_RSTICK_UP,
		PAD_RSTICK_DOWN,
		PAD_RSTICK_LEFT,
		PAD_RSTICK_RIGHT,

		PAD_LT,
		PAD_RT,
	};

	enum Axis
	{
		KEY_ARROW_X,
		KEY_ARROW_Y,
		KEY_WASD_X,
		KEY_WASD_Y,

		PAD_LSTICK_X,
		PAD_LSTICK_Y,
		PAD_RSTICK_X,
		PAD_RSTICK_Y,
	};

	struct ButtonActionInfo
	{
		InputType type; // ���͎��
		Button button; // �{�^��
	};

	struct AxisActionInfo
	{
		InputType type;
		Axis axis;
	};

private:
	Input(Window* window);
	~Input();

public:
	Input(const Input&) = delete;
	Input& operator =(const Input&) = delete;
	Input(Input&&) = delete;
	Input& operator =(Input&&) = delete;

	static Input* Get()
	{
		return instance_;
	}

	static void Create(Window* win);
	static void Destroy();

	void Update();

	void AddButtonAction(const std::string& name, const std::vector<ButtonActionInfo>& info_list);
	void AddAxisAction(const std::string& name, const std::vector<AxisActionInfo>& info_list);

	// �L�[����
	static bool GetKey(UINT index);
	// �g���K�[�̓���
	static bool GetKeyDown(UINT index);

	// �{�^��������
	static bool GetButton(const std::string& key);
	// �{�^�����������u��
	static bool GetButtonDown(const std::string& key);
	// �{�^���𗣂����u��
	static bool GetButtonUp(const std::string& key);
	// ���s�[�g
	static bool GetButtonRepeat(const std::string& key);

	// ���̒l
	static float GetAxis(const std::string& key);

	// ���݂̓��͎�ʂ��擾
	static InputType CurrentInputType() { return instance_->current_input_type_; }
	// ���͎�ʂ�ݒ�
	static void SetCurrentInputType(InputType type) { instance_->current_input_type_ = type; }

private:
	void CheckActions();

	void InitButtonMap();

private:
	// �E�B���h�E
	Window* window_;

	std::unique_ptr<DirectInput> direct_input_;
	std::unique_ptr<XInput> x_input_;

	// ���݂̓��͎��
	InputType current_input_type_;

	std::unordered_map<std::string, std::vector<ButtonActionInfo>> button_action_map_;
	std::unordered_map<std::string, std::vector<AxisActionInfo>> axis_action_map_;

	// button action �̏�ԁib1 = ������, b2 = �������u��, b3 = �������u��, b4 = �L�[���s�[�g�j
	std::unordered_map<std::string, int> button_action_state_;
	// axis action �̏��
	std::unordered_map<std::string, float> axis_action_state_;

	std::unordered_map<Button, int> button_map_;
	
	static Input* instance_;
};

