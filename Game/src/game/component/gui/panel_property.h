#pragma once

#include "math/color.h"
#include "math/rect.h"

struct PanelProperty
{
	float radius = 0;
	Rect2 padding = { 0, 0, 0, 0 };
	Color color = { 1, 1, 1, 0 };
	bool horizontal_rounded = false;
	bool vertical_rounded = false;
};
