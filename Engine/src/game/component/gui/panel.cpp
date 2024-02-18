#include "panel.h"
#include "game/game.h"
#include "engine/engine.h"
#include "engine/engine2d.h"

Panel::Panel()
{
	SetOffset({ 0, 0, 0, 0 });
	SetAnchor({ 0, 0, 0, 0 });
	color_ = Color::White();
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

bool Panel::Init()
{
	Control::Init();

	return true;
}

void Panel::Draw2D()
{
	auto engine2d = Game::Get()->GetEngine()->GetEngine2D();

	auto ratio = engine2d->RenderTargetSize().y / 720.0;

	auto rect = GetRect() * ratio;
	auto world_matrix = WorldMatrix();

	engine2d->SetTransform(world_matrix);
	engine2d->DrawFillRectangle(rect, color_);
}
