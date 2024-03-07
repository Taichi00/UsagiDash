#include "button_base.h"
#include "game/game.h"
#include "game/gui_manager.h"

ButtonBase::ButtonBase()
{
	Game::Get()->GetGUIManager()->AddButton(this);
}

ButtonBase::~ButtonBase()
{
	Game::Get()->GetGUIManager()->RemoveButton(this);
}

void ButtonBase::OnPressed()
{
	function_();
}

void ButtonBase::OnHovered()
{
}

void ButtonBase::OnUnhovered()
{
}
