#include "panel.h"
#include "game/game.h"
#include "engine/engine.h"
#include "engine/engine2d.h"

Panel::Panel()
{
	SetOffset({ 0, 0, 0, 0 });
	SetAnchor({ 0, 0, 0, 0 });
	color_ = Color::White();
	radius_ = 0.f;
}

Panel::Panel(const Rect2& offset, const Rect2& anchor, const Color& color)
{
	SetOffset(offset);
	SetAnchor(anchor);
	color_ = color;
}

Panel::Panel(const Vec2& position, const Vec2& size, const Vec2& pivot, const Vec2& anchor_pos, const Color& color)
{
	SetTransform(position, size, pivot, anchor_pos);
	color_ = color;
}

Panel::Panel(const Vec2& position, const Vec2& size, const Vec2& pivot, const Vec2& anchor_pos, const float radius, const Color& color) : Panel(position, size, pivot, anchor_pos, color)
{
	radius_ = radius;
}

bool Panel::Init()
{
	Control::Init();

	engine_->RegisterSolidColorBrush(color_);

	return true;
}

void Panel::Draw2D()
{
	auto ratio = engine_->RenderTargetSize().y / 720.0f;

	auto rect = GetRect();
	auto world_matrix = WorldMatrix();

	engine_->SetTransform(world_matrix * Matrix3x2::Scale(Vec2(1, 1) * ratio));
	engine_->DrawFillRectangle(rect, color_ * GetColor(), radius_);
}
