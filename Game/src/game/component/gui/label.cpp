#include "game/component/gui/label.h"
#include "math/color.h"
#include "engine/engine.h"
#include "engine/engine2d.h"
#include "game/string_methods.h"
#include "game/component/transform.h"
#include "game/game.h"

Label::Label() : Control()
{
	text_ = L"";
	text_prop_ = TextProperty{};
	text_color_ = Color::White();
}

Label::Label(const std::string& text) : Label()
{
	text_ = StringMethods::GetWideString(text);
}

Label::Label(const std::string& text, const TextProperty& prop, const Color& color)
{
	text_ = StringMethods::GetWideString(text);
	text_prop_ = prop;
	text_color_ = color;
}

Label::~Label()
{
}

bool Label::Init()
{
	Control::Init();

	engine_->RegisterTextFormat(text_prop_.font);
	engine_->RegisterSolidColorBrush(text_color_);

	return true;
}

void Label::Draw2D()
{
	auto ratio = engine_->RenderTargetSize().y / 720.0f;

	auto rect = GetRect();
	auto world_matrix = WorldMatrix();

	engine_->SetTransform(world_matrix * Matrix3x2::Scale(Vec2(1, 1) * ratio));
	engine_->DrawText(
		text_, 
		rect, 
		text_prop_.font, text_prop_.font_size, text_prop_.font_weight, 
		text_prop_.horizontal_alignment, text_prop_.vertical_alignment,
		text_color_);
}

void Label::SetText(const std::string& text)
{
	text_ = StringMethods::GetWideString(text);
}

void Label::FitSize()
{
	auto size = engine_->GetTextSize(text_, text_prop_.font, text_prop_.font_size, text_prop_.font_weight);
	SetAnchor({ 0, 0, 0, 0 });
	SetOffset({ 0, 0, -size.x, -size.y });
}
