#include "button.h"
#include "util/string_methods.h"
#include "engine/engine2d.h"

Button::Button(
	const std::string& text, 
	const TextProperty& text_prop, 
	const PanelProperty& panel_prop, 
	const std::function<void()>& function,
	const bool fit) : ButtonBase()
{
	text_.string = StringMethods::GetWideString(text);
	text_.prop = text_prop;
	panel_prop_ = panel_prop;
	function_ = function;
	fit_ = fit;
}

Button::~Button()
{
}

bool Button::Init()
{
	Control::Init();

	engine_->RegisterTextFormat(text_.prop.font);

	return false;
}

void Button::Update(const float delta_time)
{
	if (fit_)
		FitSize();

	Control::Update(delta_time);
}

void Button::Draw2D()
{
	auto ratio = engine_->RenderTargetScale();

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
	engine_->DrawText(text_, rect, text_.prop.color * GetColor());
}

void Button::OnPressed()
{
	function_();
}

void Button::FitSize()
{
	auto size = engine_->GetTextSize(text_);
	SetSize(size);
	Transform();
}

