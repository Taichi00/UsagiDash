#pragma once

#include "game/component/gui/control.h"

class ButtonBase : public Control
{
public:
	ButtonBase();
	virtual ~ButtonBase();

	virtual void OnPressed() {};
};