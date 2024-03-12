#include "pause_menu.h"
#include "game/component/all_components.h"
#include "app/component/game_manager.h"
#include "app/scene/title_scene.h"

PauseMenu::PauseMenu() : Entity("pause_menu")
{
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

	{
		TextProperty text_prop = {};
		text_prop.font = L"Koruri";
		text_prop.font_size = 22;
		text_prop.color = Color(0.35f, 0.3f, 0.3f);
		text_prop.font_weight = TextProperty::WEIGHT_BOLD;
		text_prop.horizontal_alignment = TextProperty::HORIZONTAL_ALIGNMENT_CENTER;
		text_prop.vertical_alignment = TextProperty::VERTICAL_ALIGNMENT_CENTER;

		PanelProperty panel_prop = {};
		panel_prop.color = Color(1, 1, 1, 1);
		panel_prop.radius = 24;
		panel_prop.padding = { 24, 6, 24, 6 };

		std::vector<std::shared_ptr<Animation>> animations;
		{
			auto animation = std::make_shared<Animation>("press");
			{
				Animation::Channel channel = {};
				channel.type = Animation::TYPE_GUI;
				channel.gui.name = "control";
				channel.gui.scale_keys = {
					{ 0, Easing::LINEAR, Vec2(0.8f, 0.8f) },
					{ 0.2f, Easing::OUT_BACK, Vec2(1.1f, 1.1f) }
				};
				animation->AddChannel(channel);
			}
			animation->SetDuration(1);
			animation->SetTicksPerSecond(1);

			animations.push_back(animation);
		}
		{
			auto animation = std::make_shared<Animation>("hover");
			{
				Animation::Channel channel = {};
				channel.type = Animation::TYPE_GUI;
				channel.gui.name = "panel";
				channel.gui.color_keys = {
					{ 0, Easing::LINEAR, Color(0.1f, 0.1f, 0.1f) }
				};
				animation->AddChannel(channel);
			}
			{
				Animation::Channel channel = {};
				channel.type = Animation::TYPE_GUI;
				channel.gui.name = "label";
				channel.gui.color_keys = {
					{ 0, Easing::LINEAR, Color(1, 1, 1) }
				};
				animation->AddChannel(channel);
			}
			{
				Animation::Channel channel = {};
				channel.type = Animation::TYPE_GUI;
				channel.gui.name = "control";
				channel.gui.scale_keys = {
					{ 0, Easing::LINEAR, Vec2(1, 1) },
					{ 0.1f, Easing::OUT_CUBIC, Vec2(1.1f, 1.1f) }
				};
				animation->AddChannel(channel);
			}
			animation->SetDuration(1);
			animation->SetTicksPerSecond(1);

			animations.push_back(animation);
		}
		{
			auto animation = std::make_shared<Animation>("unhover");
			{
				Animation::Channel channel = {};
				channel.type = Animation::TYPE_GUI;
				channel.gui.name = "panel";
				channel.gui.color_keys = {
					{ 0, Easing::LINEAR, Color(0.1f, 0.1f, 0.1f) },
					{ 0.1f, Easing::LINEAR, Color(1, 1, 1) }
				};
				animation->AddChannel(channel);
			}
			{
				Animation::Channel channel = {};
				channel.type = Animation::TYPE_GUI;
				channel.gui.name = "label";
				channel.gui.color_keys = {
					{ 0, Easing::LINEAR, Color(1, 1, 1) },
					{ 0.1f, Easing::LINEAR, Color(0.35f, 0.3f, 0.3f) }
				};
				animation->AddChannel(channel);
			}
			{
				Animation::Channel channel = {};
				channel.type = Animation::TYPE_GUI;
				channel.gui.name = "control";
				channel.gui.scale_keys = {
					{ 0, Easing::LINEAR, Vec2(1.1f, 1.1f) },
					{ 0.1f, Easing::OUT_CUBIC, Vec2(1, 1) }
				};
				animation->AddChannel(channel);
			}
			animation->SetDuration(1);
			animation->SetTicksPerSecond(1);

			animations.push_back(animation);
		}

		auto resume_button = new Entity("resume_button");
		{
			auto button = resume_button->AddComponent<AnimatedButton>(new AnimatedButton(
				"つづける",
				text_prop,
				panel_prop,
				[]() {
					GameManager::Get()->Resume();
				},
				false
			));
			resume_button->AddComponent<Animator>(new Animator(animations));

			button->SetAnchorPoint(Vec2(0.25f, 0.5f));
			button->SetPivot(Vec2(0.5f, 0.5f));
			button->SetSize(Vec2(200, 35));
			button->SetPosition(Vec2(0, 100));
			button->SetRotation(-3);
			button->Transform();

			AddChild(resume_button);
		}

		auto option_button = new Entity("option_button");
		{
			auto button = option_button->AddComponent<AnimatedButton>(new AnimatedButton(
				"フルスクリーン",
				text_prop,
				panel_prop,
				[]() {
					Game::Get()->ToggleFullscreen();
				},
				false
			));
			option_button->AddComponent<Animator>(new Animator(animations));

			button->SetAnchorPoint(Vec2(0.25f, 0.5f));
			button->SetPivot(Vec2(0.5f, 0.5f));
			button->SetSize(Vec2(200, 35));
			button->SetPosition(Vec2(0, 160));
			button->SetRotation(-3);
			button->Transform();

			AddChild(option_button);
		}

		auto exit_button = new Entity("exit_button");
		{
			auto button = exit_button->AddComponent<AnimatedButton>(new AnimatedButton(
				"タイトルにもどる",
				text_prop,
				panel_prop,
				[]() {
					GameManager::Get()->LoadTitle();
				},
				false
			));
			exit_button->AddComponent<Animator>(new Animator(animations));

			button->SetAnchorPoint(Vec2(0.25f, 0.5f));
			button->SetPivot(Vec2(0.5f, 0.5f));
			button->SetSize(Vec2(200, 35));
			button->SetPosition(Vec2(0, 220));
			button->SetRotation(-3);
			button->Transform();

			AddChild(exit_button);
		}
	}
}
