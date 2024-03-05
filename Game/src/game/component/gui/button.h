#pragma once

#include "game/component/gui/button_base.h"
#include "game/component/gui/text.h"
#include "game/component/gui/panel_property.h"
#include <functional>
#include <string>

class Button : public ButtonBase
{
public:
	Button(
		const std::string& text, 
		const TextProperty& text_prop, 
		const PanelProperty& panel_prop,
		const std::function<void()>& function,
		const bool fit = true
	);
	~Button();

	bool Init() override;
	void Update(const float delta_time) override;
	void Draw2D() override;

	// ƒ{ƒ^ƒ“‚ğ‰Ÿ‚³‚ê‚½‚Æ‚«
	void OnPressed() override;

private:
	void FitSize();

private:
	Text text_;
	PanelProperty panel_prop_;

	// ‰Ÿ‚³‚ê‚½‚Æ‚«‚ÉÀs‚·‚éŠÖ”
	std::function<void()> function_;

	bool fit_;
};