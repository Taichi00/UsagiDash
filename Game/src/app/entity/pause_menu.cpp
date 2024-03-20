#include "pause_menu.h"
#include "game/component/all_components.h"
#include "app/component/game_manager.h"
#include "app/scene/title_scene.h"
#include "game/game.h"
#include "game/component/audio/audio_source.h"
#include "app/entity/custom_button.h"

PauseMenu::PauseMenu() : Entity("pause_menu")
{
	auto game = Game::Get();

	{
		PanelProperty prop = {};
		prop.color = Color(0, 0, 0, 0.1f);

		auto panel = AddComponent<Panel>(new Panel(prop));
		panel->SetAnchor({ 0, 0, 1, 1 });
		panel->SetOffset({ 0, 0, 0, 0 });
	}

	auto title_label = new Entity("title_label");
	{
		TextProperty prop = {};
		prop.font = L"Koruri";
		prop.font_size = 80;
		prop.font_weight = TextProperty::WEIGHT_EXTRA_BOLD;
		prop.horizontal_alignment = TextProperty::HORIZONTAL_ALIGNMENT_CENTER;
		prop.vertical_alignment = TextProperty::VERTICAL_ALIGNMENT_CENTER;

		auto label = title_label->AddComponent<Label>(new Label(
			"PAUSE",
			prop, {},
			true
		));
		label->SetAnchorPoint(Vec2(0.25f, 0.5f));
		label->SetPivot(Vec2(0.5f, 0.5f));
		label->SetPosition(Vec2(0, -100));
		label->SetRotation(0);
		label->Transform();

		AddChild(title_label);
	}

	auto resume_button = new CustomButton(
		"resume_button",
		"つづける",
		[]() {
			GameManager::Get()->Resume();
		}
	);
	{
		auto button = resume_button->GetComponent<Button>();

		button->SetAnchorPoint(Vec2(0.25f, 0.5f));
		button->SetPivot(Vec2(0.5f, 0.5f));
		button->SetSize(Vec2(200, 35));
		button->SetPosition(Vec2(0, 100));
		button->SetRotation(-3);
		button->Transform();

		AddChild(resume_button);
	}

	auto option_button = new CustomButton(
		"option_button",
		"フルスクリーン",
		[]() {
			Game::Get()->ToggleFullscreen();
		}
	);
	{
		auto button = option_button->GetComponent<Button>();

		button->SetAnchorPoint(Vec2(0.25f, 0.5f));
		button->SetPivot(Vec2(0.5f, 0.5f));
		button->SetSize(Vec2(200, 35));
		button->SetPosition(Vec2(0, 160));
		button->SetRotation(-3);
		button->Transform();

		AddChild(option_button);
	}

	auto exit_button = new CustomButton(
		"exit_button",
		"タイトルにもどる",
		[]() {
			GameManager::Get()->LoadTitle();
		}
	);
	{
		auto button = exit_button->GetComponent<Button>();

		button->SetAnchorPoint(Vec2(0.25f, 0.5f));
		button->SetPivot(Vec2(0.5f, 0.5f));
		button->SetSize(Vec2(200, 35));
		button->SetPosition(Vec2(0, 220));
		button->SetRotation(-3);
		button->Transform();

		AddChild(exit_button);
	}
}
