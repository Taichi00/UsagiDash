#include "app/component/camera_controller.h"
#include "app/component/game_manager.h"
#include "app/component/map_loader.h"
#include "app/component/pause_behavior.h"
#include "app/component/pause_manager.h"
#include "app/entity/pause_menu.h"
#include "app/entity/player.h"
#include "app/entity/tutorial_label.h"
#include "app/entity/stage_clear_event.h"
#include "app/component/bgm_manager.h"
#include "game/collision_manager.h"
#include "game/component/all_components.h"
#include "game/entity.h"
#include "game/game.h"
#include "game/input/input.h"
#include "game/resource/animation.h"
#include "game/resource/bitmap.h"
#include "game/resource/collision_model.h"
#include "game/resource/model.h"
#include "game/scene.h"
#include "level1_scene.h"
#include "math/color.h"
#include "math/easing.h"
#include "math/quaternion.h"
#include "math/vec.h"
#include <memory>
#include <string>
#include <vector>

bool Level1Scene::Init()
{
	Scene::Init();

	SetSkybox(L"assets/skybox/default");

	// ポーズマネージャー
	auto pause_manager = new Entity("pause_manager");
	{
		pause_manager->AddComponent<PauseManager>();
		CreateEntity(pause_manager);
	}

	// プレイヤー
	auto player = new Player("player");
	{
		CreateEntity(player);
	}

	// カメラ
	auto camera = new Entity("camera");
	{
		camera->AddComponent(new Camera());
		camera->AddComponent(new AudioListener());
		camera->AddComponent(new Animator());
		camera->AddComponent(new CameraController(player));
		CreateEntity(camera);
		SetMainCamera(camera);
	}

	// マップモデル
	auto map = new Entity("map", "map", "map");
	{
		auto collision_model = LoadResource<CollisionModel>(L"assets/map/level1.obj");

		map->AddComponent(new MeshRenderer(LoadResource<Model>(L"assets/map/level1.obj")));
		map->AddComponent(new MeshCollider(collision_model));
		map->AddComponent(new Rigidbody(1, false, true, 0.1f));

		map->transform->scale = Vec3(0.1f, 0.1f, 0.1f);
		map->transform->position = Vec3(0, -10, 0);
		map->GetComponent<Collider>()->scale = map->transform->scale;
		map->GetComponent<MeshRenderer>()->SetOutlineWidth(0);
		CreateEntity(map);
	}

	auto coin_label = new Entity("coin_label");

	auto gui = new Entity("gui");
	{
		// コイン GUI
		auto coin_gui = new Entity("coin_gui");
		{
			PanelProperty panel_prop = {};
			panel_prop.color = Color(0, 0, 0, 0.3f);
			panel_prop.radius = 20;

			auto panel = coin_gui->AddComponent<Panel>(new Panel(panel_prop));
			panel->SetAnchorPoint(Vec2(1, 0));
			panel->SetPivot(Vec2(0.5f, 0.5f));
			panel->SetSize(Vec2(128, 40));
			panel->SetPosition(Vec2(-120, 80));
			panel->SetRotation(4);
			panel->Transform();

			// コイン枚数ラベル
			{
				TextProperty coin_label_prop{};
				coin_label_prop.font = L"Koruri";
				coin_label_prop.font_size = 28;
				coin_label_prop.color = Color(1, 1, 1);
				coin_label_prop.font_weight = TextProperty::WEIGHT_EXTRA_BOLD;
				coin_label_prop.vertical_alignment = TextProperty::VERTICAL_ALIGNMENT_CENTER;

				auto animation = std::make_shared<Animation>("get");
				Animation::Channel channel = {};
				channel.type = Animation::TYPE_GUI;
				channel.gui.name = "label";
				channel.gui.position_keys = {
					{ 0, Easing::LINEAR, Vec2(0, 0) },
					{ 0.2f, Easing::OUT_CUBIC, Vec2(0, -3) },
					{ 0.6f, Easing::IN_CUBIC, Vec2(0, 0) },
				};
				channel.gui.color_keys = {
					{ 0, Easing::LINEAR, Color(1, 0.75f, 0) },
					{ 1, Easing::IN_CUBIC, Color(1, 1, 1) }
				};
				animation->AddChannel(channel);
				animation->SetDuration(1);
				animation->SetTicksPerSecond(3);

				auto label = coin_label->AddComponent<Label>(new Label("0000", coin_label_prop));
				coin_label->AddComponent(new Animator(animation));

				label->SetAnchorPoint(Vec2(0.5f, 0.5f));
				label->SetPivot(Vec2(0.5f, 0.5f));
				label->SetPosition(Vec2(10, -2));
				label->Transform();

				coin_gui->AddChild(coin_label);
			}

			// コインアイコン
			auto coin_icon = new Entity("coin_icon");
			{
				auto picture = coin_icon->AddComponent<Picture>(new Picture(LoadResource<Bitmap>(L"assets/image/coin_icon.png")));
				picture->SetTransform(Vec2(-46, 0), Vec2(128, 128), Vec2(0.5f, 0.5f), Vec2(0.5, 0.5f));
				picture->SetScale(Vec2(0.32f, 0.32f));

				coin_gui->AddChild(coin_icon);
			}

			gui->AddChild(coin_gui);
		}

		// メニューラベル
		auto menu_label = new Entity("menu_label");
		{
			TextProperty prop = {};
			prop.font = L"Koruri";
			prop.font_size = 16;
			prop.font_weight = TextProperty::WEIGHT_BOLD;
			prop.horizontal_alignment = TextProperty::HORIZONTAL_ALIGNMENT_CENTER;
			prop.vertical_alignment = TextProperty::VERTICAL_ALIGNMENT_BOTTOM;

			PanelProperty panel_prop = {};
			panel_prop.color = Color(0, 0, 0, 0.3f);
			panel_prop.padding = { 16, 4, 18, 6 };
			panel_prop.radius = 16;

			auto label = menu_label->AddComponent<Label>(new Label(
				"<bitmap input_menu> メニュー",
				prop,
				panel_prop,
				true
			));

			label->SetAnchorPoint(Vec2(1, 1));
			label->SetPivot(Vec2(1, 1));
			label->SetPosition(Vec2(-48, -30));
			label->SetRotation(0);
			label->Transform();

			gui->AddChild(menu_label);
		}

		// チュートリアル用ラベル
		auto tutorial_label = new TutorialLabel();
		{
			gui->AddChild(tutorial_label);
		}

		CreateEntity(gui);
	}

	auto bgm_manager = new Entity("bgm_manager");
	{
		bgm_manager->AddComponent<AudioSource>();
		auto manager = bgm_manager->AddComponent<BGMManager>();

		CreateEntity(bgm_manager);

		manager->Play(LoadResource<Audio>(L"assets/bgm/MusMus-CT-NV-24.wav"), 0.5f);
	}

	// ポーズメニュー
	auto pause_menu = new PauseMenu();
	{
		CreateEntity(pause_menu);

		// 無効にしておく
		pause_menu->SetActive(false);
	}

	// ゲームマネージャー
	auto game_manager = new Entity("game_manager");
	{
		game_manager->AddComponent(new GameManager(
			GameManager::SCENE_GAME,
			player,
			camera,
			coin_label->GetComponent<Label>(),
			{ 90, 150 }
		));

		CreateEntity(game_manager);
	}
	
	// マップローダー
	auto map_loader = new Entity("map_loader");
	{
		map_loader->AddComponent(new MapLoader(L"assets/Map/level1.map"));

		map_loader->transform->scale = Vec3(0.1f, 0.1f, 0.1f);
		map_loader->transform->position = Vec3(0, -10, 0);
		CreateEntity(map_loader);
	}

	// ステージクリア演出
	auto stage_clear_event = new StageClearEvent(this);
	{
		CreateEntity(stage_clear_event);
	}

	return true;
}

void Level1Scene::Update(const float delta_time)
{
	Scene::Update(delta_time);
}
