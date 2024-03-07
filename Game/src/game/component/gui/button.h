#pragma once

#include "game/component/gui/button_base.h"
#include "game/component/gui/panel_property.h"
#include "game/component/gui/element/label_element.h"
#include "game/component/gui/element/panel_element.h"
#include <functional>
#include <string>

class LabelElement;

class Button : public ButtonBase
{
public:
	Button(
		const std::string& text,
		const TextProperty& text_prop, 
		const PanelProperty& panel_prop,
		const std::function<void()>& function,
		const bool fit = true
	);
	~Button();

	bool Init() override;
	void Update(const float delta_time) override;
	void Draw2D() override;

	// �{�^���������ꂽ�Ƃ�
	virtual void OnPressed() override;
	// �J�[�\�����������Ƃ�
	virtual void OnHovered() override;
	// �J�[�\�����O�ꂽ�Ƃ�
	virtual void OnUnhovered() override;

private:
	void FitSize();

private:
	bool fit_;
};