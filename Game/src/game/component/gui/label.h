#pragma once

#include "game/component/gui/control.h"
#include "math/color.h"
#include "game/component/gui/text_property.h"
#include "game/component/gui/panel_property.h"
#include "game/component/gui/text.h"
#include <string>
#include <memory>

class Label : public Control
{
public:
	Label();
	Label(const std::string& text);
	Label(
		const std::string& text, 
		const TextProperty& text_prop, 
		const PanelProperty& panel_prop, 
		const Color& color,
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
	Text text_;
	//TextProperty text_prop_;
	PanelProperty panel_prop_;
	Color text_color_;
	bool fit_;

	std::string color_key_;
	std::string font_key_;
};