#include "panel_element.h"
#include "engine/engine2d.h"
#include "game/component/gui/control.h"

PanelElement::PanelElement() : Element()
{
}

PanelElement::PanelElement(const PanelProperty& prop, Control* control) : Element(control)
{
	prop_ = prop;
	color_ = prop_.color;
}

PanelElement::~PanelElement()
{
}

void PanelElement::Draw()
{
	Element::Draw();

	auto rect = control_->GetRect();
	rect = {
		rect.left - prop_.padding.left,
		rect.top - prop_.padding.top,
		rect.right + prop_.padding.right,
		rect.bottom + prop_.padding.bottom
	};

	engine_->DrawFillRectangle(rect, color_ * control_->GetColor(), prop_.radius);
}
