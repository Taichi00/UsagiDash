#include "panel.h"
#include "game/game.h"
#include "engine/engine.h"
#include "engine/engine2d.h"

Panel::Panel(const PanelProperty& prop)
{
	AddElement<PanelElement>("panel", new PanelElement(prop, this));
}

bool Panel::Init()
{
	Control::Init();

	return true;
}

void Panel::Update(const float delta_time)
{
	Control::Update(delta_time);
}

void Panel::Draw2D()
{
	Control::Draw2D();
}
