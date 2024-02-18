#pragma once

#include "game/component/gui/control.h"
#include "math/color.h"
#include <string>
#include <memory>

class Label : public Control
{
public:
	enum FontWeight : unsigned
	{
		LIGHT = 300,
		NORMAL = 400,
		REGULAR = 400,
		SEMI_BOLD = 600,
		BOLD = 700,
		EXTRA_BOLD = 800
	};

	Label();
	Label(const std::string& text);
	Label(const std::string& text, const std::string& font, const float size, const FontWeight& font_weight, const Color& color);

	~Label();

	bool Init();
	void Draw2D();

	void SetText(const std::string& text);

private:
	void FitSize(); // テキストのサイズに合わせる

private:
	std::wstring text_;
	std::wstring font_name_;
	float font_size_;
	Color font_color_;
	FontWeight font_weight_;

	std::string color_key_;
	std::string font_key_;
};