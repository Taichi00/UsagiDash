#include "game/component/all_components.h"
#include "app/component/game_manager.h"
#include "app/component/map_loader.h"
#include "app/component/player_controller.h"
#include "app/component/player_controller_2.h"
#include "app/entity/player.h"
#include "app/component/camera_controller.h"
#include "app/component/pause_behavior.h"
#include "app/component/pause_manager.h"
#include "app/entity/tutorial_label.h"
#include "game/entity.h"
#include "game/game.h"
#include "game/input/input.h"
#include "game/resource/bitmap.h"
#include "game/resource/collision_model.h"
#include "game/resource/model.h"
#include "game/scene.h"
#include "level1_scene.h"
#include "app/scene/title_scene.h"
#include "math/color.h"
#include "math/vec.h"
#include "math/easing.h"
#include <game/animation.h>
#include <memory>
#include <random>
#include <string>
#include <vector>

Entity* player, *enemy, *testSphere, *movingObj;
float angle = 0;
Vec3 direction = Vec3(1, 0, 0);

bool Level1Scene::Init()
{
	Scene::Init();

	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<float> frand(-1, 1);

	SetSkybox(L"assets/skybox/default");

	auto pause_manager = new Entity("pause_manager");
	{
		pause_manager->AddComponent<PauseManager>();
		CreateEntity(pause_manager);
	}

	player = new Player("player");
	{
		CreateEntity(player);
	}

	auto camera = new Entity("camera");
	{
		camera->AddComponent(new Camera());
		camera->AddComponent(new AudioListener());
		camera->AddComponent(new CameraController(player));
		//camera->AddComponent(new PauseBehavior());
		CreateEntity(camera);
		SetMainCamera(camera);
	}

	{
		std::vector<Entity*> objects;
		std::shared_ptr sphereModel = SphereMesh::Load(1, 0.72f, 0, 0);
		std::shared_ptr capsuleModel = CapsuleMesh::Load(2, 2, 0.72f, 0, 0);

		for (int i = 0; i < 0; i++)
		{
			std::shared_ptr<Model> model;
			auto object = new Entity("object " + std::to_string(i + 1), "object", "object");
			/*
			if (i % 2 == 1)
			{
				object->AddComponent(new CapsuleCollider({ 2, 2 }));
				model = capsuleModel;
			}
			else
			{*/
			object->AddComponent(new SphereCollider(1));
			model = sphereModel;
			//}

			object->AddComponent(new MeshRenderer(model));
			object->AddComponent(new Rigidbody(0.5f, true, false, 0.1f));
			object->AddComponent(new PauseBehavior());
			CreateEntity(object);
			objects.push_back(object);

			auto pos = Vec3(frand(mt) * 20, frand(mt) * 10 + 10, frand(mt) * 20);
			//object->transform->position = Vec3(10 * (i + 1), 10, 0);
			object->transform->position = pos;
		}
	}

	movingObj = new Entity("rift", "object", "object");
	{
		movingObj->AddComponent(new MeshRenderer(LoadResource<Model>(L"assets/model/object/rift.obj")));
		movingObj->AddComponent(new MeshCollider(LoadResource<CollisionModel>(L"assets/model/object/rift.obj")));
		movingObj->AddComponent(new Rigidbody(5, false, true, 0.1f));
		movingObj->AddComponent(new PauseBehavior());

		movingObj->transform->position = Vec3(0, -10, 25);
		movingObj->transform->scale = Vec3(10, 10, 10);
		movingObj->GetComponent<Collider>()->scale = movingObj->transform->scale;
		CreateEntity(movingObj);
	}

	enemy = new Entity("enemy", "enemy", "object");
	{
		enemy->AddComponent(new MeshRenderer(LoadResource<Model>(L"assets/model/character/Enemy.gltf")));
		enemy->AddComponent(new SphereCollider(1.5f));
		enemy->AddComponent(new Rigidbody(0.5, true, false, 0.1f));
		enemy->AddComponent(new Animator());
		enemy->AddComponent(new PauseBehavior());

		enemy->GetComponent<Collider>()->offset = Vec3(0, 1.5f, 0);
		enemy->transform->position = Vec3(-5, 0, -5);
		enemy->transform->scale = Vec3(2, 2, 2);
		CreateEntity(enemy);
		enemy->GetComponent<Animator>()->Play("Walk", 2.0f);
	}

	auto map = new Entity("map", "map", "map");
	{
		map->AddComponent(new MeshRenderer(LoadResource<Model>(L"assets/map/level1.obj")));
		map->AddComponent(new MeshCollider(LoadResource<CollisionModel>(L"assets/map/level1.obj")));
		map->AddComponent(new Rigidbody(1, false, true, 0.1f));

		map->transform->scale = Vec3(0.1f, 0.1f, 0.1f);
		map->transform->position = Vec3(0, -10, 0);
		map->GetComponent<Collider>()->scale = map->transform->scale;
		map->GetComponent<MeshRenderer>()->SetOutlineWidth(0);
		CreateEntity(map);
	}

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

		auto coin_label = new Entity("coin_label");
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

			coin_label->SetParent(coin_gui);
		}

		auto coin_icon = new Entity("coin_icon");
		{
			auto picture = coin_icon->AddComponent<Picture>(new Picture(LoadResource<Bitmap>(L"assets/image/coin_icon.png")));
			picture->SetTransform(Vec2(-46, 0), Vec2(128, 128), Vec2(0.5f, 0.5f), Vec2(0.5, 0.5f));
			picture->SetScale(Vec2(0.32f, 0.32f));

			coin_icon->SetParent(coin_gui);
		}

		CreateEntity(coin_gui);
	}

	auto guide_label = new Entity("menu_label");
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

		auto label = guide_label->AddComponent<Label>(new Label(
			"<bitmap input_menu> ƒƒjƒ…[",
			prop,
			panel_prop,
			true
		));

		label->SetAnchorPoint(Vec2(1, 1));
		label->SetPivot(Vec2(1, 1));
		label->SetPosition(Vec2(-48, -30));
		label->SetRotation(0);
		label->Transform();

		CreateEntity(guide_label);
	}

	auto tutorial_label = new TutorialLabel();
	{
		CreateEntity(tutorial_label);
	}

	auto game_manager = new Entity("game_manager");
	{
		game_manager->AddComponent(new GameManager(
			GameManager::SCENE_GAME,
			player->GetComponent<PlayerController2>(),
			camera->GetComponent<CameraController>(),
			coin_gui->Child("coin_label")->GetComponent<Label>()
		));
		//game_manager->AddComponent(new AudioSource(LoadResource<Audio>(L"assets/bgm/y014_m.wav"), 100.f));

		CreateEntity(game_manager);
	}
	
	auto map_loader = new Entity("map_loader");
	{
		map_loader->AddComponent(new MapLoader(L"assets/Map/level1.map"));

		map_loader->transform->scale = Vec3(0.1f, 0.1f, 0.1f);
		map_loader->transform->position = Vec3(0, -10, 0);
		CreateEntity(map_loader);
	}

	return true;
}

void Level1Scene::Update(const float delta_time)
{
	Scene::Update(delta_time);

	auto rigidbody = movingObj->GetComponent<Rigidbody>();
	
	//rigidbody->velocity.x = sin(angle) * 20 - rigidbody->transform->position.x;
	//rigidbody->velocity.y = cos(angle) * 20 - rigidbody->transform->position.y;
	if (rigidbody->transform->position.y < -13)
	{
		rigidbody->velocity.y *= -1;
		rigidbody->transform->position.y = -13;
	}
	if (rigidbody->transform->position.y > 7)
	{
		rigidbody->velocity.y *= -1;
		rigidbody->transform->position.y = 7;
	}
	if (rigidbody->velocity.y >= 0)
	{
		rigidbody->velocity.y += 61.2f * delta_time;
	}
	else
	{
		rigidbody->velocity.y = -6.0f;
	}
	//rigidbody->velocity.x += (0 - movingObj->transform->position.x) * 1.8 * delta_time;
	//rigidbody->velocity.y += (-10 - movingObj->transform->position.y) * 0.03;
	//rigidbody->velocity.z += (25 - movingObj->transform->position.z) * 1.8 * delta_time;
}
