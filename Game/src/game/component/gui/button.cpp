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
	Text t = {};
	t.string = StringMethods::GetWideString(text);
	t.prop = text_prop;

	AddElement<PanelElement>("panel", new PanelElement(panel_prop, this))->SetZIndex(0);
	AddElement<LabelElement>("label", new LabelElement(t, this))->SetZIndex(1);

	function_ = function;
	fit_ = fit;
}

Button::~Button()
{
}

bool Button::Init()
{
	Control::Init();

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
	Control::Draw2D();
}

void Button::OnPressed()
{
	function_();
}

void Button::OnHovered()
{
	//panel_color_ = panel_prop_.color.Inverse();
	//text_color_ = text_.prop.color.Inverse();
}

void Button::OnUnhovered()
{
	//panel_color_ = panel_prop_.color;
	//text_color_ = text_.prop.color;
}

void Button::FitSize()
{
	auto size = GetElement<LabelElement>("label")->GetTextSize();
	SetSize(size);
	Transform();
}

