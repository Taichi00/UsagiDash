#pragma once

#include "game/component/gui/control.h"
#include <functional>

class ButtonBase : public Control
{
public:
	ButtonBase();
	virtual ~ButtonBase();

	// �����ꂽ�u��
	virtual void OnPressed();
	// �J�[�\�������킹��ꂽ�u��
	virtual void OnHovered();
	// �J�[�\�����O�ꂽ�u��
	virtual void OnUnhovered();

protected:
	// �{�^���������ꂽ�Ƃ��Ɏ��s����֐�
	std::function<void()> function_;
};