#pragma once

#include "math/color.h"
#include <string>

struct TextProperty
{
	enum FontWeight
	{
		WEIGHT_LIGHT = 300,
		WEIGHT_NORMAL = 400,
		WEIGHT_REGULAR = 400,
		WEIGHT_SEMI_BOLD = 600,
		WEIGHT_BOLD = 700,
		WEIGHT_EXTRA_BOLD = 800
	};

	enum HorizontalAlignment
	{
		HORIZONTAL_ALIGNMENT_LEFT,
		HORIZONTAL_ALIGNMENT_RIGHT,
		HORIZONTAL_ALIGNMENT_CENTER,
		HORIZONTAL_ALIGNMENT_FILL
	};

	enum VerticalAlignment
	{
		VERTICAL_ALIGNMENT_TOP,
		VERTICAL_ALIGNMENT_BOTTOM,
		VERTICAL_ALIGNMENT_CENTER,
		VERTICAL_ALIGNMENT_FILL
	};

	std::wstring font = L"MS Gothic";
	float font_size = 24.f;
	Color color = Color::White();
	FontWeight font_weight = WEIGHT_NORMAL;
	HorizontalAlignment horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
	VerticalAlignment vertical_alignment = VERTICAL_ALIGNMENT_TOP;
};