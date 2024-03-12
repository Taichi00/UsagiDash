#include "label_element.h"
#include "engine/engine2d.h"
#include "game/component/gui/control.h"
#include "util/string_methods.h"

LabelElement::LabelElement() : Element()
{
}

LabelElement::LabelElement(const Text& text, Control* control) : Element(control)
{
	text_ = text;
	color_ = text_.prop.color;
}

LabelElement::~LabelElement()
{
}

void LabelElement::Init()
{
	Element::Init();

	engine_->RegisterTextFormat(text_.prop.font);

	text_.Parse();
}

void LabelElement::Draw()
{
	Element::Draw();

	engine_->DrawText(text_, control_->GetRect(), color_ * control_->GetColor());
}

void LabelElement::SetText(const std::string& text)
{
	text_.string = StringMethods::GetWideString(text);
	text_.Parse();
}

Vec2 LabelElement::GetTextSize()
{
	return engine_->GetTextSize(text_);
}
