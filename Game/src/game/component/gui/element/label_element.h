#pragma once

#include "game/component/gui/element/element.h"
#include "game/component/gui/text.h"
#include "math/color.h"
#include "math/vec.h"
#include <string>

class LabelElement : public Element
{
public:
	LabelElement();
	LabelElement(
		const Text& text,
		Control* control
	);
	~LabelElement();

	void Init() override;
	void Draw() override;

	// �������ύX����
	void SetText(const std::string& text);
	// �e�L�X�g���҂�����͂ދ�`�̑傫����Ԃ�
	Vec2 GetTextSize();

private:
	Text text_;
};