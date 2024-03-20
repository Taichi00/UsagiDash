#include "result_menu.h"
#include "game/component/all_components.h"
#include "app/component/game_manager.h"
#include "app/scene/title_scene.h"
#include "game/game.h"
#include "game/component/audio/audio_source.h"
#include "app/entity/custom_button.h"
#include "game/component/animator.h"
#include "game/component/timeline_player.h"
#include "app/component/result_manager.h"

ResultMenu::ResultMenu(const ResultManager::CrownScores& time_scores) : Entity("result_menu")
{
	auto game = Game::Get();

	auto title_picture = new Entity("title_label");
	{
		title_picture->AddComponent<Animator>();
		auto picture = title_picture->AddComponent<Picture>(new Picture(
			game->LoadResource<Bitmap>(L"assets/image/stage_clear_logo.png")
		));
		picture->SetAnchorPoint(Vec2(0.75f, 0.5f));
		picture->SetPivot(Vec2(0.5f, 0.5f));
		picture->SetPosition(Vec2(-50, -180));
		picture->SetScale(Vec2(0.66f, 0.66f));
		//picture->SetColor(Color(1, 1, 1, 0));
		picture->Transform();

		AddChild(title_picture);
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

		button->SetAnchorPoint(Vec2(0, 0.5f));
		button->SetPivot(Vec2(0, 0.5f));
		button->SetSize(Vec2(200, 35));
		button->SetPosition(Vec2(100, 250));
		button->SetRotation(0);
		button->Transform();

		AddChild(exit_button);
	}

	Entity* time_crown;
	Entity* coin_crown;
	Entity* death_crown;

	auto background_panel = new Entity("background_panel");
	{
		PanelProperty panel_prop = {};
		panel_prop.color = Color(1, 1, 1, 0.7f);
		panel_prop.radius = 32;

		background_panel->AddComponent<Animator>();
		auto panel = background_panel->AddComponent<Panel>(panel_prop);

		panel->SetAnchorPoint(Vec2(0.75f, 0.75f));
		panel->SetPivot(Vec2(0.5f, 0.5f));
		panel->SetSize(Vec2(350, 200));
		panel->SetRotation(2);
		panel->SetColor(Color(1, 1, 1, 0));
		panel->Transform();

		AddChild(background_panel);

		auto result_label = new Entity("result_label");
		{
			TextProperty prop = {};
			prop.color = Color(1, 1, 1);
			prop.font = L"Koruri";
			prop.font_size = 20;
			prop.font_weight = TextProperty::WEIGHT_EXTRA_BOLD;
			prop.horizontal_alignment = TextProperty::HORIZONTAL_ALIGNMENT_CENTER;
			prop.vertical_alignment = TextProperty::VERTICAL_ALIGNMENT_CENTER;
				
			PanelProperty pprop = {};
			pprop.color = Color(0.8f, 0.5f, 0.1f);
			pprop.radius = 25;
			pprop.padding = { 60, 7, 60, 7 };

			auto label = result_label->AddComponent<Label>(new Label("リザルト", prop, pprop, true));

			label->SetAnchorPoint(Vec2(0.5f, 0));
			label->SetPivot(Vec2(0.5f, 0.5f));
			label->SetPosition(Vec2(0, 35));
			label->Transform();

			background_panel->AddChild(result_label);
		}

		{
			TextProperty prop = {};
			prop.color = Color(0.35f, 0.3f, 0.3f);
			prop.font = L"Koruri";
			prop.font_size = 20;
			prop.font_weight = TextProperty::WEIGHT_BOLD;
			prop.horizontal_alignment = TextProperty::HORIZONTAL_ALIGNMENT_LEFT;
			prop.vertical_alignment = TextProperty::VERTICAL_ALIGNMENT_CENTER;

			auto time_label = new Entity("time_label");
			{
				auto label = time_label->AddComponent<Label>("タイム", prop);

				label->SetAnchorPoint(Vec2(0, 0));
				label->SetPivot(Vec2(0, 0.5f));
				label->SetPosition(Vec2(30, 90));
				label->Transform();

				background_panel->AddChild(time_label);
			}

			auto coin_label = new Entity("coin_label");
			{
				auto label = coin_label->AddComponent<Label>("あつめたコイン", prop);

				label->SetAnchorPoint(Vec2(0, 0));
				label->SetPivot(Vec2(0, 0.5f));
				label->SetPosition(Vec2(30, 120));
				label->Transform();

				background_panel->AddChild(coin_label);
			}

			auto death_label = new Entity("death_label");
			{
				auto label = death_label->AddComponent<Label>("落ちた回数", prop);

				label->SetAnchorPoint(Vec2(0, 0));
				label->SetPivot(Vec2(0, 0.5f));
				label->SetPosition(Vec2(30, 150));
				label->Transform();

				background_panel->AddChild(death_label);
			}
		}

		{
			TextProperty prop = {};
			prop.color = Color(0.35f, 0.3f, 0.3f);
			prop.font = L"Koruri";
			prop.font_size = 20;
			prop.font_weight = TextProperty::WEIGHT_EXTRA_BOLD;
			prop.horizontal_alignment = TextProperty::HORIZONTAL_ALIGNMENT_LEFT;
			prop.vertical_alignment = TextProperty::VERTICAL_ALIGNMENT_CENTER;

			auto time_score = new Entity("time_score");
			{
				auto label = time_score->AddComponent<Label>("00:00:00", prop);

				label->SetAnchorPoint(Vec2(0.6f, 0));
				label->SetPivot(Vec2(0, 0.5f));
				label->SetPosition(Vec2(0, 90));
				label->Transform();

				background_panel->AddChild(time_score);
			}

			auto coin_score = new Entity("coin_score");
			{
				auto label = coin_score->AddComponent<Label>("0/0", prop);

				label->SetAnchorPoint(Vec2(0.6f, 0));
				label->SetPivot(Vec2(0, 0.5f));
				label->SetPosition(Vec2(0, 120));
				label->Transform();

				background_panel->AddChild(coin_score);
			}

			auto death_score = new Entity("death_score");
			{
				auto label = death_score->AddComponent<Label>("0", prop);

				label->SetAnchorPoint(Vec2(0.6f, 0));
				label->SetPivot(Vec2(0, 0.5f));
				label->SetPosition(Vec2(0, 150));
				label->Transform();

				background_panel->AddChild(death_score);
			}
		}

		{
			time_crown = new Entity("time_crown");
			{
				auto picture = time_crown->AddComponent<Picture>();
				time_crown->AddComponent<Animator>();

				picture->SetAnchorPoint(Vec2(1, 0));
				picture->SetPivot(Vec2(0.5f, 0.5f));
				picture->SetPosition(Vec2(-40, 90));
				picture->SetScale(Vec2(0.4f, 0.4f));
				picture->SetColor(Color(1, 1, 1, 0));
				picture->Transform();

				background_panel->AddChild(time_crown);
			}

			coin_crown = new Entity("coin_crown");
			{
				auto picture = coin_crown->AddComponent<Picture>();
				coin_crown->AddComponent<Animator>();

				picture->SetAnchorPoint(Vec2(1, 0));
				picture->SetPivot(Vec2(0.5f, 0.5f));
				picture->SetPosition(Vec2(-40, 120));
				picture->SetScale(Vec2(0.4f, 0.4f));
				picture->SetColor(Color(1, 1, 1, 0));
				picture->Transform();

				background_panel->AddChild(coin_crown);
			}

			death_crown = new Entity("death_crown");
			{
				auto picture = death_crown->AddComponent<Picture>();
				death_crown->AddComponent<Animator>();

				picture->SetAnchorPoint(Vec2(1, 0));
				picture->SetPivot(Vec2(0.5f, 0.5f));
				picture->SetPosition(Vec2(-40, 150));
				picture->SetScale(Vec2(0.4f, 0.4f));
				picture->SetColor(Color(1, 1, 1, 0));
				picture->Transform();

				background_panel->AddChild(death_crown);
			}
		}
	}

	// タイムライン
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
					{ 0, Easing::LINEAR, Vec2(-50, -180) },
					{ 10, Easing::OUT_CUBIC, Vec2(0, -180) },
				};
				channel.gui.scale_keys =
				{
					{ 0, Easing::LINEAR, Vec2(1.2f, 0.8f) * 0.66f },
					{ 3, Easing::OUT_CUBIC, Vec2(0.8f, 1.2f) * 0.66f },
					{ 6, Easing::OUT_CUBIC, Vec2(1.0f, 1.0f) * 0.66f },
				};
				animation->AddChannel(channel);
			}
			animation->SetDuration(35);
			animation->SetTicksPerSecond(60);
		}

		{
			Timeline::AnimationTrack anim_track;
			anim_track.animation_keys =
			{
				{ 0, Easing::LINEAR, animation, 1, false }
			};
			track.animation.push_back(anim_track);
		}

		timeline->AddTrack(track);
	}

	// リザルト
	{
		Timeline::Track track = {};
		track.type = Timeline::TYPE_ANIMATION;
		track.target = background_panel;

		auto animation = std::make_shared<Animation>();
		{
			{
				Animation::Channel channel = {};
				channel.type = Animation::TYPE_GUI;
				channel.gui.name = "control";
				channel.gui.color_keys =
				{
					{ 0, Easing::LINEAR, Color(1, 1, 1, 0) },
					{ 5, Easing::OUT_CUBIC, Color(1, 1, 1, 1) }
				};
				channel.gui.scale_keys =
				{
					{ 0, Easing::LINEAR, Vec2(0.8f, 0.8f) },
					{ 10, Easing::OUT_BACK, Vec2(1, 1) },
				};
				animation->AddChannel(channel);
			}
			animation->SetDuration(60);
			animation->SetTicksPerSecond(60);
		}

		{
			Timeline::AnimationTrack anim_track;
			anim_track.animation_keys =
			{
				{ 30, Easing::LINEAR, animation, 1, false }
			};
			track.animation.push_back(anim_track);
		}

		timeline->AddTrack(track);
	}

	// 王冠
	{
		auto animation = std::make_shared<Animation>();
		{
			{
				Animation::Channel channel = {};
				channel.type = Animation::TYPE_GUI;
				channel.gui.name = "control";
				channel.gui.color_keys =
				{
					{ 0, Easing::LINEAR, Color(1, 1, 1, 0) },
					{ 5, Easing::OUT_CUBIC, Color(1, 1, 1, 1) }
				};
				channel.gui.scale_keys =
				{
					{ 0, Easing::LINEAR, Vec2(0.5f, 0.5f) },
					{ 10, Easing::OUT_BACK, Vec2(0.4f, 0.4f) },
				};
				animation->AddChannel(channel);
			}
			animation->SetDuration(60);
			animation->SetTicksPerSecond(60);
		}

		{
			Timeline::Track track = {};
			track.type = Timeline::TYPE_ANIMATION;
			track.target = time_crown;
			{
				Timeline::AnimationTrack anim_track;
				anim_track.animation_keys =
				{
					{ 60, Easing::LINEAR, animation, 1, false }
				};
				track.animation.push_back(anim_track);
			}
			timeline->AddTrack(track);
		}
		{
			Timeline::Track track = {};
			track.type = Timeline::TYPE_ANIMATION;
			track.target = coin_crown;
			{
				Timeline::AnimationTrack anim_track;
				anim_track.animation_keys =
				{
					{ 70, Easing::LINEAR, animation, 1, false }
				};
				track.animation.push_back(anim_track);
			}
			timeline->AddTrack(track);
		}
		{
			Timeline::Track track = {};
			track.type = Timeline::TYPE_ANIMATION;
			track.target = death_crown;
			{
				Timeline::AnimationTrack anim_track;
				anim_track.animation_keys =
				{
					{ 80, Easing::LINEAR, animation, 1, false }
				};
				track.animation.push_back(anim_track);
			}
			timeline->AddTrack(track);
		}
	}

	// サウンド
	{
		auto crown_audio = game->LoadResource<Audio>(L"assets/se/MI_SFX 28.wav");

		Timeline::Track track = {};
		track.type = Timeline::TYPE_AUDIO;
		track.target = this;
		track.audio.audio_keys =
		{
			{ 60, Easing::LINEAR, crown_audio, false, 0.5f },
			{ 70, Easing::LINEAR, crown_audio, false, 0.5f },
			{ 80, Easing::LINEAR, crown_audio, false, 0.5f },
		};
		timeline->AddTrack(track);
	}

	timeline->SetDuration(300);

	AddComponent(new AudioSource());
	AddComponent(new TimelinePlayer(timeline));
	AddComponent(new ResultManager(time_scores));
}
