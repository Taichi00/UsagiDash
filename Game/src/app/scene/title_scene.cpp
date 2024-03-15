#include "title_scene.h"
#include "game/component/all_components.h"
#include "game/resource/all_resources.h"
#include "app/component/game_manager.h"
#include "game/input/input.h"
#include "app/scene/level1_scene.h"

bool TitleScene::Init()
{
    Scene::Init();

	SetSkybox(L"assets/skybox/default");

	auto game = Game::Get();

	auto camera = new Entity("camera");
	{
		camera->AddComponent(new Camera());
		camera->AddComponent(new AudioListener());
		CreateEntity(camera);
		SetMainCamera(camera);
	}

	auto background = new Entity("background");
	{
		PanelProperty prop = {};
		prop.color = Color(0.35f, 0.7f, 1);

		auto panel = background->AddComponent<Panel>(new Panel(prop));
		panel->SetAnchor({ 0, 0, 1, 1 });
		panel->SetOffset({ 0, 0, 0, 0 });

		CreateEntity(background);
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
			"タイトル",
			prop, {},
			true
		));
		label->SetAnchorPoint(Vec2(0.25f, 0.5f));
		label->SetPivot(Vec2(0.5f, 0.5f));
		label->SetPosition(Vec2(0, -100));
		label->SetRotation(0);
		label->Transform();

		CreateEntity(title_label);
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

		auto audio_press = game->LoadResource<Audio>(L"assets/se/abs-popup-3.wav");
		auto audio_hover = game->LoadResource<Audio>(L"assets/se/MI_SFX 25.wav");

		auto start_button = new Entity("start_button");
		{
			auto as_press = start_button->AddComponent<AudioSource>(audio_press);
			auto as_hover = start_button->AddComponent<AudioSource>(audio_hover);
			auto button = start_button->AddComponent<AnimatedButton>(new AnimatedButton(
				"はじめる",
				text_prop,
				panel_prop,
				[]() {
					GameManager::Get()->StartGame();
				},
				as_press,
				as_hover,
				false
			));
			start_button->AddComponent<Animator>(new Animator(animations));

			button->SetAnchorPoint(Vec2(0.25f, 0.5f));
			button->SetPivot(Vec2(0.5f, 0.5f));
			button->SetSize(Vec2(200, 35));
			button->SetPosition(Vec2(0, 100));
			button->SetRotation(-3);
			button->Transform();

			CreateEntity(start_button);
		}

		auto option_button = new Entity("option_button");
		{
			auto as_press = option_button->AddComponent<AudioSource>(audio_press);
			auto as_hover = option_button->AddComponent<AudioSource>(audio_hover);
			auto button = option_button->AddComponent<AnimatedButton>(new AnimatedButton(
				"フルスクリーン",
				text_prop,
				panel_prop,
				[]() {
					Game::Get()->ToggleFullscreen();
				},
				as_press,
				as_hover,
				false
			));
			option_button->AddComponent<Animator>(new Animator(animations));

			button->SetAnchorPoint(Vec2(0.25f, 0.5f));
			button->SetPivot(Vec2(0.5f, 0.5f));
			button->SetSize(Vec2(200, 35));
			button->SetPosition(Vec2(0, 160));
			button->SetRotation(-3);
			button->Transform();

			CreateEntity(option_button);
		}

		auto exit_button = new Entity("exit_button");
		{
			auto as_press = exit_button->AddComponent<AudioSource>(audio_press);
			auto as_hover = exit_button->AddComponent<AudioSource>(audio_hover);
			auto button = exit_button->AddComponent<AnimatedButton>(new AnimatedButton(
				"おわる",
				text_prop,
				panel_prop,
				[]() {
					GameManager::Get()->EndGame();
				},
				as_press,
				as_hover,
				false
			));
			exit_button->AddComponent<Animator>(new Animator(animations));

			button->SetAnchorPoint(Vec2(0.25f, 0.5f));
			button->SetPivot(Vec2(0.5f, 0.5f));
			button->SetSize(Vec2(200, 35));
			button->SetPosition(Vec2(0, 220));
			button->SetRotation(-3);
			button->Transform();

			CreateEntity(exit_button);
		}
	}

	auto guide_label = new Entity("guide_label");
	{
		TextProperty prop = {};
		prop.font = L"Koruri";
		prop.font_size = 16;
		prop.font_weight = TextProperty::WEIGHT_SEMI_BOLD;
		prop.horizontal_alignment = TextProperty::HORIZONTAL_ALIGNMENT_CENTER;
		prop.vertical_alignment = TextProperty::VERTICAL_ALIGNMENT_CENTER;

		auto label = guide_label->AddComponent<Label>(new Label(
			"<bitmap input_move> 選択  <bitmap input_ok> 決定",
			prop,
			{},
			true
		));

		label->SetAnchorPoint(Vec2(0.25f, 0.5f));
		label->SetPivot(Vec2(0.5f, 0.5f));
		label->SetPosition(Vec2(0, 260));
		label->SetRotation(-3);
		label->Transform();

		CreateEntity(guide_label);
	}

	auto game_manager = new Entity("game_manager");
	{
		game_manager->AddComponent(new GameManager(
			GameManager::SCENE_TITLE,
			nullptr,
			nullptr,
			nullptr
		));
		game_manager->AddComponent(new AudioSource(LoadResource<Audio>(L"assets/bgm/Sweet Treats.wav")));

		CreateEntity(game_manager);
	}

    return true;
}

void TitleScene::Update(const float delta_time)
{
	Scene::Update(delta_time);
}
