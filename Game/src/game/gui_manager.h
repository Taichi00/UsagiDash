#pragma once

#include <vector>

class ButtonBase;

class GUIManager
{
public:
	GUIManager();
	~GUIManager();

	void Update();

	// �{�^����ǉ�
	void AddButton(ButtonBase* button);

	// �{�^�����폜
	void RemoveButton(ButtonBase* button);

	// �{�^����I������
	void PickButton(ButtonBase* button);

private:
	void UpdateTarget();
	void CheckButtonPress();

private:
	// �{�^���̃��X�g
	std::vector<ButtonBase*> buttons_;

	// ���ݑI������Ă���{�^��
	ButtonBase* current_picked_button_ = nullptr;
	ButtonBase* prev_picked_button_ = nullptr;
};