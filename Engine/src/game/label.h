#pragma once

#include "game/component/component.h"
#include "math/color.h"
#include <string>

class Label : public Component
{
public:
	Label();
	Label(const std::string& text);
	Label(const std::string& text, const std::string& font);
	Label(const std::string& text, const float size);
	Label(const std::string& text, const std::string& font, const float size);
	Label(const std::string& text, const std::string& font, const float size, const Color& color);

	~Label();

	bool Init();
	void Draw2D();

private:
	std::string text_;
	std::string font_;
	float size_;
	Color color_;

	float width_;
	float height_;

	std::string color_key_;
	std::string font_key_;
};