#pragma once

#include "game/component/gui/control.h"
#include "math/vec.h"
#include "math/color.h"
#include "math/rect.h"

class Panel : public Control
{
public:
	Panel();
	Panel(const Rect2& offset, const Rect2& anchor, const Color& color);
	Panel(const Vec2& position, const Vec2& size, const Vec2& pivot, const Vec2& anchor_pos, const Color& color);
	Panel(const Vec2& position, const Vec2& size, const Vec2& pivot, const Vec2& anchor_pos, const float radius, const Color& color);

	~Panel() = default;

	bool Init() override;
	void Draw2D() override;

private:
	Color color_;
	float radius_;

	std::string color_key_;
};