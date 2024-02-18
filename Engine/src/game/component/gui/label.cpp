#include "game/component/gui/label.h"
#include "math/color.h"
#include "engine/engine.h"
#include "engine/engine2d.h"
#include "game/string_methods.h"
#include "game/component/transform.h"
#include "game/game.h"

Label::Label()
{
	text_ = L"";
	font_name_ = L"MS ƒSƒVƒbƒN";
	font_size_ = 24;
	font_color_ = Color::White();
	font_weight_ = FontWeight::NORMAL;
}

Label::Label(const std::string& text) : Label()
{
	text_ = StringMethods::GetWideString(text);
	FitSize();
}

Label::Label(const std::string& text, const std::string& font, const float size, const FontWeight& font_weight, const Color& color) : Label()
{
	text_ = StringMethods::GetWideString(text);
	font_name_ = StringMethods::GetWideString(font);
	font_size_ = size;
	font_color_ = color;
	font_weight_ = font_weight;
	FitSize();
}

Label::~Label()
{
}

bool Label::Init()
{
	Control::Init();

	return true;
}

void Label::Draw2D()
{
	auto engine2d = Game::Get()->GetEngine()->GetEngine2D();

	auto ratio = engine2d->RenderTargetSize().y / 720.0;

	auto rect = GetRect() * ratio;
	auto world_matrix = WorldMatrix();

	engine2d->SetTransform(world_matrix);
	engine2d->DrawText(text_, rect, font_name_, font_size_ * ratio, font_weight_, font_color_);
}

void Label::SetText(const std::string& text)
{
	text_ = StringMethods::GetWideString(text);
}

void Label::FitSize()
{
	auto engine2d = Game::Get()->GetEngine()->GetEngine2D();
	auto size = engine2d->GetTextSize(text_, font_name_, font_size_, font_weight_);
	SetAnchor({ 0, 0, 0, 0 });
	SetOffset({ 0, 0, -size.x, -size.y });
}
