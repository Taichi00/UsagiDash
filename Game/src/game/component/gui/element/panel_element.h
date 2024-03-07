#pragma once

#include "element.h"
#include "game/component/gui/panel_property.h"
#include "math/vec.h"

class PanelElement : public Element
{
public:
	PanelElement();
	PanelElement(
		const PanelProperty& prop,
		Control* control
	);
	~PanelElement();

	void Draw() override;

private:
	PanelProperty prop_;
};