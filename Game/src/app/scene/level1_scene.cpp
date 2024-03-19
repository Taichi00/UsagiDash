#include "app/component/camera_controller.h"
#include "app/component/game_manager.h"
#include "app/component/map_loader.h"
#include "app/component/metal_ball_emitter_controller.h"
#include "app/component/pause_behavior.h"
#include "app/component/pause_manager.h"
#include "app/component/player_controller.h"
#include "app/component/player_controller_2.h"
#include "app/entity/metal_ball_emitter.h"
#include "app/entity/pause_menu.h"
#include "app/entity/player.h"
#include "app/entity/tutorial_label.h"
#include "app/scene/title_scene.h"
#include "game/collision_manager.h"
#include "game/component/all_components.h"
#include "game/component/timeline_player.h"
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
#include <random>
#include <string>
#include <vector>

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

	auto player = new Player("player");
	{
		CreateEntity(player);
	}

	auto camera = new Entity("camera");
	{
		camera->AddComponent(new Camera());
		camera->AddComponent(new AudioListener());
		camera->AddComponent(new Animator());
		camera->AddComponent(new CameraController(player));
		//camera->AddComponent(new PauseBehavior());
		CreateEntity(camera);
		SetMainCamera(camera);
	}

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

	auto pause_menu = new PauseMenu();
	{
		CreateEntity(pause_menu);
		pause_menu->SetActive(false);
	}

	auto game_manager = new Entity("game_manager");
	{
		game_manager->AddComponent(new GameManager(
			GameManager::SCENE_GAME,
			player,
			camera,
			coin_gui->Child("coin_label")->GetComponent<Label>()
		));

		CreateEntity(game_manager);
	}
	
	auto map_loader = new Entity("map_loader");
	{
		map_loader->AddComponent(new MapLoader(L"assets/Map/level1.map"));

		map_loader->transform->scale = Vec3(0.1f, 0.1f, 0.1f);
		map_loader->transform->position = Vec3(0, -10, 0);
		CreateEntity(map_loader);
	}

	auto clear_event = new Entity("clear_event");
	{
		auto timeline = std::make_shared<Timeline>();

		{
			auto animator = player->GetComponent<Animator>();

			Timeline::Track track = {};
			track.type = Timeline::TYPE_ANIMATION;
			track.target = player;
			{
				Timeline::AnimationTrack animation;
				animation.animation_keys =
				{
					{ 0, Easing::LINEAR, animator->GetAnimation("Jump"), 2, false },
					{ 20, Easing::LINEAR, animator->GetAnimation("Jump_Idle"), 2, true },
					{ 40, Easing::LINEAR, animator->GetAnimation("Jump_Land"), 2, false },
					{ 60, Easing::LINEAR, animator->GetAnimation("Wave"), 2, true },
				};

				track.animation.push_back(animation);
			}
			timeline->AddTrack(track);
		}

		{
			auto star = FindEntity("star");

			Timeline::Track track = {};
			track.type = Timeline::TYPE_ANIMATION;
			track.target = star;
			{
				auto animation_up = std::make_shared<Animation>();
				{
					Animation::Channel channel = {};
					channel.type = Animation::TYPE_TRANSFORM;
					channel.transform.position_keys =
					{
						{ 0, Easing::LINEAR, Vec3(0, 0, 0) },
						{ 60, Easing::OUT_CUBIC, Vec3(0, 5, 0) }
					};
					animation_up->AddChannel(channel);
				}
				animation_up->SetDuration(60);
				animation_up->SetTicksPerSecond(60);

				auto animation_rotate = std::make_shared<Animation>();
				{
					Animation::Channel channel = {};
					channel.type = Animation::TYPE_TRANSFORM;
					channel.transform.rotation_keys =
					{
						{ 0, Easing::LINEAR, Quaternion::FromEuler(0, 0, 0) },
						{ 20, Easing::IN_QUAD, Quaternion::FromEuler(0, 3.14f, 0) },
						{ 40, Easing::OUT_QUAD, Quaternion::FromEuler(0, 6.28f, 0) },
					};
					animation_rotate->AddChannel(channel);
				}
				animation_rotate->SetDuration(40);
				animation_rotate->SetTicksPerSecond(60);

				{
					Timeline::AnimationTrack anim_track;
					anim_track.animation_keys =
					{
						{ 0, Easing::LINEAR, animation_up, 1, false }
					};
					track.animation.push_back(anim_track);
				}
				/*{
					Timeline::AnimationTrack anim_track;
					anim_track.animation_keys =
					{
						{ 0, Easing::LINEAR, animation_rotate, 2, true }
					};
					track.animation.push_back(anim_track);
				}*/
			}
			timeline->AddTrack(track);
		}

		{
			Timeline::Track track = {};
			track.type = Timeline::TYPE_ANIMATION;
			track.target = camera;
			{
				auto animation = std::make_shared<Animation>();
				{
					Animation::Channel channel = {};
					channel.type = Animation::TYPE_TRANSFORM;
					channel.transform.position_keys =
					{
						{ 0, Easing::LINEAR, Vec3(0, 5, 10) },
						{ 120, Easing::OUT_QUAD, Vec3(0, 5, 20) }
					};
					animation->AddChannel(channel);
				}
				animation->SetDuration(120);
				animation->SetTicksPerSecond(60);

				Timeline::AnimationTrack anim_track;
				anim_track.animation_keys =
				{
					{ 0, Easing::LINEAR, animation, 1, false }
				};
				track.animation.push_back(anim_track);
			}
			timeline->AddTrack(track);
		}

		{
			Timeline::Track track = {};
			track.type = Timeline::TYPE_AUDIO;
			track.target = clear_event;
			track.audio.audio_keys =
			{
				{ 60, Easing::LINEAR, LoadResource<Audio>(L"assets/se/confirmation_002.wav") },
				{ 90, Easing::LINEAR, LoadResource<Audio>(L"assets/se/cute-level-up-3-189853.wav") }
			};
			timeline->AddTrack(track);
		}

		timeline->SetDuration(120);

		clear_event->AddComponent(new AudioSource());
		clear_event->AddComponent(new TimelinePlayer(timeline));

		CreateEntity(clear_event);
	}

	return true;
}

void Level1Scene::Update(const float delta_time)
{
	Scene::Update(delta_time);
}
