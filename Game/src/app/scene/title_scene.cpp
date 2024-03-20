#include "title_scene.h"
#include "game/component/all_components.h"
#include "game/resource/all_resources.h"
#include "app/component/game_manager.h"
#include "game/input/input.h"
#include "app/scene/level1_scene.h"
#include "app/entity/custom_button.h"
#include "app/component/bgm_manager.h"

bool TitleScene::Init()
{
    Scene::Init();

	SetSkybox(L"assets/skybox/title");

	auto game = Game::Get();

	auto rabbit = new Entity("rabbit");
	{
		auto model = LoadResource<Model>(L"assets/model/character/Character2.glb");

		float pbr_color[4] = { 1, 1, 0, 1 };
		auto smoke_albedo = game->LoadResource<Texture2D>(L"assets/effect/smoke_albedo.png");
		auto smoke_normal = game->LoadResource<Texture2D>(L"assets/effect/smoke_normal.png");
		std::shared_ptr smoke_pbr = Texture2D::GetMono(pbr_color);

		auto run_smoke_prop = ParticleEmitterProperty{};
		run_smoke_prop.time_to_live = 50;
		run_smoke_prop.time_to_live_range = 10;
		run_smoke_prop.spawn_rate = 0.1f;
		run_smoke_prop.spawn_rate_range = 0;
		run_smoke_prop.position_type = PROP_TYPE_PVA;
		run_smoke_prop.position_pva.position = Vec3(0, 0, -1);
		run_smoke_prop.position_pva.position_range = Vec3(0.5f, 0, 0);
		run_smoke_prop.position_pva.velocity = Vec3(0, 0.5f, -100);
		run_smoke_prop.position_pva.velocity_range = Vec3(0, 0.5f, 0);
		run_smoke_prop.rotation_pva.rotation_range = Vec3(0, 0, 3.14f);
		run_smoke_prop.rotation_pva.velocity_range = Vec3(0, 0, 6);
		run_smoke_prop.scale_type = PROP_TYPE_EASING;
		run_smoke_prop.scale_easing.type = EASE_OUT_CUBIC;
		run_smoke_prop.scale_easing.keep_aspect = true;
		run_smoke_prop.scale_easing.middle_enabled = true;
		run_smoke_prop.scale_easing.start = Vec3(0.5f, 0.5f, 0.5f);
		run_smoke_prop.scale_easing.middle = Vec3(2.5f, 2.5f, 2.5f);
		run_smoke_prop.scale_easing.middle_range = Vec3(0.5f, 0.5f, 0.5f);
		run_smoke_prop.scale_easing.end = Vec3(0, 0, 0);
		run_smoke_prop.sprite.albedo_texture = smoke_albedo;
		run_smoke_prop.sprite.normal_texture = smoke_normal;
		run_smoke_prop.sprite.pbr_texture = smoke_pbr;

		auto mesh_renderer = rabbit->AddComponent<MeshRenderer>(new MeshRenderer(model));
		auto animator = rabbit->AddComponent<Animator>(new Animator(model->animations));
		auto particle_emitter = rabbit->AddComponent(new ParticleEmitter(run_smoke_prop));

		CreateEntity(rabbit);

		rabbit->transform->rotation = Quaternion::FromEuler(0, -0.4f, 0);
		rabbit->transform->scale = Vec3(5, 5, 5);

		animator->Play("Run", 2, true, 0);
		particle_emitter->Emit();
		mesh_renderer->SetOutlineWidth(0.004f);
	}

	auto camera = new Entity("camera");
	{
		auto cam = camera->AddComponent(new Camera());
		camera->AddComponent(new AudioListener());

		CreateEntity(camera);
		SetMainCamera(camera);

		cam->Focus(true);
		cam->SetFocusPosition(Vec3(-13, 10, 0));
		
		camera->transform->position = Vec3(0, 20, 40);
	}

	auto title_picture = new Entity("title_label");
	{
		title_picture->AddComponent<Animator>();
		auto picture = title_picture->AddComponent<Picture>(new Picture(
			game->LoadResource<Bitmap>(L"assets/image/title_logo.png")
		));
		picture->SetAnchorPoint(Vec2(0.3f, 0.5f));
		picture->SetPivot(Vec2(0.5f, 0.5f));
		picture->SetPosition(Vec2(0, -100));
		picture->SetScale(Vec2(0.66f, 0.66f));
		//picture->SetColor(Color(1, 1, 1, 0));
		picture->Transform();

		CreateEntity(title_picture);
	}

	{
		auto start_button = new CustomButton(
			"start_button", 
			"はじめる",
			[]() {
				GameManager::Get()->StartGame();
			}
		);
		{
			auto button = start_button->GetComponent<Button>();

			button->SetAnchorPoint(Vec2(0.25f, 0.5f));
			button->SetPivot(Vec2(0.5f, 0.5f));
			button->SetSize(Vec2(200, 35));
			button->SetPosition(Vec2(0, 100));
			button->SetRotation(-3);
			button->Transform();

			CreateEntity(start_button);
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

			CreateEntity(option_button);
		}

		auto exit_button = new CustomButton(
			"exit_button",
			"おわる",
			[]() {
				GameManager::Get()->EndGame();
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

	auto bgm_manager = new Entity("bgm_manager");
	{
		bgm_manager->AddComponent<AudioSource>();
		auto manager = bgm_manager->AddComponent<BGMManager>();

		CreateEntity(bgm_manager);

		manager->Play(LoadResource<Audio>(L"assets/bgm/PerituneMaterial_Radiant Sunshine_loop.wav"), 0.5f);
	}

	auto game_manager = new Entity("game_manager");
	{
		game_manager->AddComponent(new GameManager(
			GameManager::SCENE_TITLE,
			nullptr,
			nullptr,
			nullptr,
			{}
		));

		CreateEntity(game_manager);
	}

	// タイムライン
	auto timeline_player = new Entity("timeline_player");
	{
		auto timeline = std::make_shared<Timeline>();

		// タイトル
		{
			auto animator = title_picture->GetComponent<Animator>();

			Timeline::Track track = {};
			track.type = Timeline::TYPE_ANIMATION;
			track.target = title_picture;

			auto animation = std::make_shared<Animation>();
			{
				{
					Animation::Channel channel = {};
					channel.type = Animation::TYPE_GUI;
					channel.gui.name = "control";
					channel.gui.position_keys =
					{
						{ 0, Easing::LINEAR, Vec2(-50, -100) },
						{ 10, Easing::OUT_BACK, Vec2(0, -100) },
					};
					channel.gui.scale_keys =
					{
						{ 0, Easing::LINEAR, Vec2(1.2f, 0.8f) * 0.66f },
						{ 3, Easing::OUT_CUBIC, Vec2(0.8f, 1.2f) * 0.66f },
						{ 6, Easing::OUT_CUBIC, Vec2(1.0f, 1.0f) * 0.66f },
					};
					animation->AddChannel(channel);
				}
				animation->SetDuration(32);
				animation->SetTicksPerSecond(60);
			}

			{
				Timeline::AnimationTrack anim_track;
				anim_track.animation_keys =
				{
					{ 0, Easing::LINEAR, animation, 1, true }
				};
				track.animation.push_back(anim_track);
			}

			timeline->AddTrack(track);
		}

		auto player = timeline_player->AddComponent<TimelinePlayer>(timeline);

		CreateEntity(timeline_player);

		player->Play();
	}

    return true;
}

void TitleScene::Update(const float delta_time)
{
	Scene::Update(delta_time);
}
