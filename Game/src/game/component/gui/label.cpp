#include "game/component/gui/label.h"
#include "math/color.h"
#include "engine/engine.h"
#include "engine/engine2d.h"
#include "util/string_methods.h"
#include "game/component/transform.h"
#include "game/game.h"

Label::Label(const std::string& text, const TextProperty& text_prop)
{
	Text t = {};
	t.string = StringMethods::GetWideString(text);
	t.prop = text_prop;

	AddElement<LabelElement>("label", new LabelElement(t, this))->SetZIndex(1);
	fit_ = true;
}

Label::Label(const std::string& text, const TextProperty& text_prop, const PanelProperty& panel_prop, const bool fit)
{
	Text t = {};
	t.string = StringMethods::GetWideString(text);
	t.prop = text_prop;

	AddElement<PanelElement>("panel", new PanelElement(panel_prop, this))->SetZIndex(0);
	AddElement<LabelElement>("label", new LabelElement(t, this))->SetZIndex(1);
	fit_ = fit;
}

Label::~Label()
{
}

bool Label::Init()
{
	Control::Init();

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
	Control::Draw2D();
}

void Label::SetText(const std::string& text)
{
	GetElement<LabelElement>("label")->SetText(text);
	Update(0);
}

void Label::FitSize()
{
	auto size = GetElement<LabelElement>("label")->GetTextSize();
	SetSize(size);
	Transform();
}
