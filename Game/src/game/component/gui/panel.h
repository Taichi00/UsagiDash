#pragma once

#include "game/component/gui/control.h"
#include "game/component/gui/element/panel_element.h"

class Panel : public Control
{
public:
	Panel(const PanelProperty& prop);
	~Panel() = default;

	bool Init() override;
	void Update(const float delta_time) override;
	void Draw2D() override;

private:
};