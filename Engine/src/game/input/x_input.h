#pragma once

#include <windows.h>

#pragma comment(lib, "xinput.lib")
#include <Xinput.h>

#include "math/vec.h"

class XInput
{
public:
	XInput();
	~XInput();

	void Update();

	// ������
	bool GetButton(const int button) const;
	// �������u��
	bool GetButtonDown(const int button) const;
	// �������u��
	bool GetButtonUp(const int button) const;

	// �L�[�̏�ԁib1 = ������, b2 = �������u��, b3 = �������u�ԁj
	int GetButtonState(const int button) const;

	Vec2 GetLStick() const;
	int GetLStickUpState() const;
	int GetLStickDownState() const;
	int GetLStickLeftState() const;
	int GetLStickRightState() const;

	Vec2 GetRStick() const;
	int GetRStickUpState() const;
	int GetRStickDownState() const;
	int GetRStickLeftState() const;
	int GetRStickRightState() const;

	float GetLTrigger() const;
	int GetLTriggerState() const;

	float GetRTrigger() const;
	int GetRTriggerState() const;

private:
	Vec2 GetStickVec(const SHORT sx, const SHORT sy, const int dead_zone) const;
	float GetTriggerValue(const BYTE v, const int threshold) const;

private:
	// �R���g���[���[�̏��
	XINPUT_STATE state_;
	// �O�̏��
	XINPUT_STATE prev_state_;

	Vec2 left_stick_, prev_left_stick_;
	Vec2 right_stick_, prev_right_stick_;

	float left_trigger_, prev_left_trigger_;
	float right_trigger_, prev_right_trigger_;

	// �R���g���[���[���ڑ�����Ă��邩�ǂ���
	bool is_connected_;
};