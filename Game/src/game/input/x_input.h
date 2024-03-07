#pragma once

#include <windows.h>

#pragma comment(lib, "xinput.lib")
#include <Xinput.h>

#include "math/vec.h"
#include <unordered_map>

#define XINPUT_BUTTON_MAX 24
// �X�e�B�b�N���{�^�����͂Ƃ��ĔF������臒l
#define XINPUT_STICK_BUTTON_THRESHOLD 0.4

class XInput
{
public:
	enum Button
	{
		UP,
		DOWN,
		LEFT,
		RIGHT,
		START,
		BACK,
		LEFT_THUMB,
		RIGHT_THUMB,
		LB,
		RB,
		A,
		B,
		X,
		Y,
		LSTICK_UP,
		LSTICK_DOWN,
		LSTICK_LEFT,
		LSTICK_RIGHT,
		RSTICK_UP,
		RSTICK_DOWN,
		RSTICK_LEFT,
		RSTICK_RIGHT,
		LT,
		RT
	};

	XInput();
	~XInput();

	void Update();

	// ������
	bool GetButton(const int index) const;
	// �������u��
	bool GetButtonDown(const int index) const;
	// �������u��
	bool GetButtonUp(const int index) const;
	// ���s�[�g
	bool GetButtonRepeat(const int index) const;

	// �L�[�̏�ԁib1 = ������, b2 = �������u��, b3 = �������u��, b4 = ���s�[�g�j
	int GetButtonState(const int index) const;

	Vec2 GetLStick() const;
	Vec2 GetRStick() const;
	
	float GetLTrigger() const;
	float GetRTrigger() const;

	bool IsConnected() const { return is_connected_; }

private:
	Vec2 GetStickVec(const SHORT sx, const SHORT sy, const int dead_zone) const;
	float GetTriggerValue(const BYTE v, const int threshold) const;

	void UpdateButton();
	void InitButtonMap();

private:
	// �R���g���[���[�̏��
	XINPUT_STATE state_;

	// �{�^���̏��
	int buttons_[XINPUT_BUTTON_MAX];
	// �O�̏��
	int prev_buttons_[XINPUT_BUTTON_MAX];

	Vec2 left_stick_, prev_left_stick_;
	Vec2 right_stick_, prev_right_stick_;

	float left_trigger_, prev_left_trigger_;
	float right_trigger_, prev_right_trigger_;

	// �R���g���[���[���ڑ�����Ă��邩�ǂ���
	bool is_connected_;

	std::unordered_map<Button, int> button_map_;

	// ���s�[�g�J�n����
	int repeat_start_;
	// ���s�[�g�Ԋu
	int repeat_interval_;
};