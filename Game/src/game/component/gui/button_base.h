#pragma once

#include "game/component/gui/control.h"
#include <functional>

class ButtonBase : public Control
{
public:
	ButtonBase();
	virtual ~ButtonBase();

	// 押された瞬間
	virtual void OnPressed();
	// カーソルが合わせられた瞬間
	virtual void OnHovered();
	// カーソルが外れた瞬間
	virtual void OnUnhovered();

protected:
	// ボタンが押されたときに実行する関数
	std::function<void()> function_;
};