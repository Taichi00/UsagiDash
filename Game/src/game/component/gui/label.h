#pragma once

#include "game/component/gui/control.h"
#include "game/component/gui/element/label_element.h"
#include "game/component/gui/element/panel_element.h"
#include <string>

class Label : public Control
{
public:
	Label(
		const std::string& text,
		const TextProperty& text_prop
	);
	Label(
		const std::string& text, 
		const TextProperty& text_prop, 
		const PanelProperty& panel_prop, 
		const bool fit = false
	);

	~Label();

	bool Init() override;
	void Update(const float delta_time) override;
	void Draw2D() override;

	void SetText(const std::string& text);

private:
	void FitSize(); // テキストのサイズに合わせる

private:
	bool fit_;
};