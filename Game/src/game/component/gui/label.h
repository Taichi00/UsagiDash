#pragma once

#include "game/component/gui/control.h"
#include "math/color.h"
#include "game/component/gui/text_property.h"
#include <string>
#include <memory>

class Label : public Control
{
public:
	Label();
	Label(const std::string& text);
	Label(const std::string& text, const TextProperty& prop, const Color& color);

	~Label();

	bool Init();
	void Draw2D();

	void SetText(const std::string& text);

private:
	void FitSize(); // テキストのサイズに合わせる

private:
	std::wstring text_;
	TextProperty text_prop_;
	Color text_color_;

	std::string color_key_;
	std::string font_key_;
};