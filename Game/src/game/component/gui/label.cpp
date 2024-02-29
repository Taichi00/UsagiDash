#include "game/component/gui/label.h"
#include "math/color.h"
#include "engine/engine.h"
#include "engine/engine2d.h"
#include "game/string_methods.h"
#include "game/component/transform.h"
#include "game/game.h"

Label::Label() : Control()
{
	text_ = Text{};
	//text_prop_ = TextProperty{};
	panel_prop_ = PanelProperty{};
	text_color_ = Color::White();
}

Label::Label(const std::string& text) : Label()
{
	text_.string = StringMethods::GetWideString(text);
}

Label::Label(const std::string& text, const TextProperty& text_prop, const PanelProperty& panel_prop, const Color& color, const bool fit)
{
	text_.string = StringMethods::GetWideString(text);
	text_.prop = text_prop;
	panel_prop_ = panel_prop;
	text_color_ = color;
	fit_ = fit;
}

Label::~Label()
{
}

bool Label::Init()
{
	Control::Init();

	engine_->RegisterTextFormat(text_.prop.font);
	engine_->RegisterSolidColorBrush(text_color_);
	engine_->RegisterSolidColorBrush(panel_prop_.color);

	text_.Parse();

	return true;
}

void Label::Update(const float delta_time)
{
	if (fit_)
		FitSize();

	Control::Update(delta_time);
}

void Label::Draw2D()
{
	auto ratio = engine_->AspectRatio();

	auto padding = panel_prop_.padding;

	auto rect = GetRect();
	Rect2 panel_rect = { 
		rect.left - padding.left, 
		rect.top - padding.top, 
		rect.right + padding.right, 
		rect.bottom + padding.bottom
	};

	auto world_matrix = WorldMatrix();

	engine_->SetTransform(world_matrix * Matrix3x2::Scale(Vec2(1, 1) * ratio));
	engine_->DrawFillRectangle(panel_rect, panel_prop_.color * GetColor(), panel_prop_.radius);
	engine_->DrawText(text_, rect, text_color_ * GetColor());
}

void Label::SetText(const std::string& text)
{
	text_.string = StringMethods::GetWideString(text);
	text_.Parse();

	Update(0);
}

void Label::FitSize()
{
	auto size = engine_->GetTextSize(text_);
	SetSize(size);
	Transform();
}
