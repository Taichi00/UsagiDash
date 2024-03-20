#pragma once

#include <vector>

class Control;
class ButtonBase;

class GUIManager
{
public:
	GUIManager();
	~GUIManager();

	void Update();

	void Add(Control* control);
	void Remove(Control* control);

	// �{�^����ǉ�
	void AddButton(ButtonBase* button);

	// �{�^�����폜
	void RemoveButton(ButtonBase* button);

	// �{�^����I������
	void PickButton(ButtonBase* button);

	// z index �Ń\�[�g�������X�g���擾����
	std::vector<Control*> GetDrawList();

private:
	void UpdateTarget();
	void CheckButtonPress();

private:
	// control �̃��X�g
	std::vector<Control*> controls_;
	// �{�^���̃��X�g
	std::vector<ButtonBase*> buttons_;

	// ���ݑI������Ă���{�^��
	ButtonBase* current_picked_button_ = nullptr;
	ButtonBase* prev_picked_button_ = nullptr;
};