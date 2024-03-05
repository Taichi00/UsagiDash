#include "title_scene.h"
#include "game/component/all_components.h"
#include "game/resource/all_resources.h"
#include "app/component/game_manager.h"
#include "game/input/input.h"
#include "app/scene/level1_scene.h"

bool TitleScene::Init()
{
    Scene::Init();

	auto camera = new Entity("camera");
	{
		camera->AddComponent(new Camera());
		camera->AddComponent(new AudioListener());
		CreateEntity(camera);
		SetMainCamera(camera);
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
			"Star Hopper",
			prop, {},
			true
		));
		label->SetPosition(Vec2(0, 0));
		label->SetPivot(Vec2(0.5f, 0.5f));
		label->SetAnchorPoint(Vec2(0.5f, 0.5f));
		label->Transform();

		CreateEntity(title_label);
	}

	auto cursor_area = new Entity("cursor_area");
	{
		auto area = cursor_area->AddComponent<CursorArea>(new CursorArea());
		area->SetOffset({ 0, 0, 0, 0 });
		area->SetAnchor({ 0, 0, 1, 1 });

		TextProperty text_prop = {};
		text_prop.font = L"Koruri";
		text_prop.font_size = 22;
		text_prop.color = Color(0.35f, 0.3f, 0.3f);
		text_prop.font_weight = TextProperty::WEIGHT_BOLD;
		text_prop.horizontal_alignment = TextProperty::HORIZONTAL_ALIGNMENT_CENTER;
		text_prop.vertical_alignment = TextProperty::VERTICAL_ALIGNMENT_CENTER;

		PanelProperty panel_prop = {};
		panel_prop.color = Color(1, 1, 1, 1);
		panel_prop.radius = 28;
		panel_prop.padding = { 24, 6, 24, 6 };

		auto start_button = new Entity("start_button");
		{
			auto button = start_button->AddComponent<Button>(new Button(
				"Start",
				text_prop,
				panel_prop,
				[]() {
					printf("Start\n");
				},
				false
			));
			button->SetAnchorPoint(Vec2(0.5f, 0.5f));
			button->SetPivot(Vec2(0.5f, 0.5f));
			button->SetSize(Vec2(200, 40));
			button->SetPosition(Vec2(0, 200));
			button->Transform();

			start_button->SetParent(cursor_area);
		}

		auto exit_button = new Entity("exit_button");
		{
			auto button = exit_button->AddComponent<Button>(new Button(
				"Exit",
				text_prop,
				panel_prop,
				[]() {
					printf("Exit\n");
				},
				false
			));
			button->SetAnchorPoint(Vec2(0.5f, 0.5f));
			button->SetPivot(Vec2(0.5f, 0.5f));
			button->SetSize(Vec2(200, 40));
			button->SetPosition(Vec2(0, 260));
			button->Transform();

			exit_button->SetParent(cursor_area);
		}

		CreateEntity(cursor_area);
	}

	auto transition = new Entity("transition");
	{
		transition->AddComponent(new Transition(
			Color(0.1f, 0.1f, 0.15f),
			Vec2(1, 1))
		);

		CreateEntity(transition);
	}

	auto game_manager = new Entity("game_manager");
	{
		game_manager->AddComponent(new GameManager(
			nullptr,
			nullptr,
			nullptr
		));
		//game_manager->AddComponent(new AudioSource(LoadResource<Audio>(L"assets/bgm/y014_m.wav"), 100.f));

		CreateEntity(game_manager);
	}

    return true;
}

void TitleScene::Update(const float delta_time)
{
	Scene::Update(delta_time);

	/*if (Input::GetButtonDown("ok"))
	{
		Game::Get()->LoadScene(new Level1Scene());
	}*/
}
