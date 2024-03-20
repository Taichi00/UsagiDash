#include "stage_clear_event.h"
#include "game/component/all_components.h"
#include "game/scene.h"

StageClearEvent::StageClearEvent(Scene* scene) : Entity("stage_clear_event")
{
	auto game = Game::Get();

	auto camera = scene->FindEntity("camera");
	auto player = scene->FindEntity("player");
	auto star = scene->FindEntity("star");

	auto timeline = std::make_shared<Timeline>();

	// プレイヤー
	{
		auto animator = player->GetComponent<Animator>();

		Timeline::Track track = {};
		track.type = Timeline::TYPE_ANIMATION;
		track.target = player;
		{
			{
				Timeline::AnimationTrack anim_track;
				anim_track.animation_keys =
				{
					{ 0, Easing::LINEAR, animator->GetAnimation("Wave"), 2, true },
					{ 120, Easing::LINEAR, animator->GetAnimation("Yes"), 2, false },
					{ 150, Easing::LINEAR, animator->GetAnimation("Yes"), 2, false },
					{ 180, Easing::LINEAR, animator->GetAnimation("Jump"), 2, false },
					{ 200, Easing::LINEAR, animator->GetAnimation("Jump_Idle"), 2, true },
					{ 220, Easing::LINEAR, animator->GetAnimation("Jump_Land"), 2, false },
					{ 240, Easing::LINEAR, animator->GetAnimation("Wave"), 2, true },
				};
				track.animation.push_back(anim_track);
			}
			{
				auto animation = std::make_shared<Animation>();
				{
					Animation::Channel channel = {};
					channel.type = Animation::TYPE_TRANSFORM;
					channel.transform.position_keys =
					{
						{ 0, Easing::LINEAR, Vec3(0, 0, 0) },
						{ 10, Easing::OUT_CUBIC, Vec3(0, 2, 1) },
						{ 25, Easing::IN_CUBIC, Vec3(0, 0, 2) },
					};
					channel.transform.rotation_keys =
					{
						{ 0, Easing::LINEAR, Quaternion::FromEuler(0, 0, 0) },
						{ 8, Easing::LINEAR, Quaternion::FromEuler(0, 3.14f, 0) },
						{ 20, Easing::OUT_QUAD, Quaternion::FromEuler(0, 6.28f, 0) },
					};
					animation->AddChannel(channel);
				}
				animation->SetDuration(30);
				animation->SetTicksPerSecond(60);

				Timeline::AnimationTrack anim_track;
				anim_track.animation_keys =
				{
					{ 180, Easing::LINEAR, animation, 1, false },
				};
				track.animation.push_back(anim_track);
			}
		}
		timeline->AddTrack(track);
	}

	// スター
	{
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
					{ 60, Easing::OUT_CUBIC, Vec3(0, 3, 0) }
				};
				animation_up->AddChannel(channel);
			}
			animation_up->SetDuration(60);
			animation_up->SetTicksPerSecond(60);

			{
				Timeline::AnimationTrack anim_track;
				anim_track.animation_keys =
				{
					{ 0, Easing::LINEAR, animation_up, 1, false }
				};
				track.animation.push_back(anim_track);
			}
		}
		timeline->AddTrack(track);
	}

	// カメラ
	{
		Timeline::Track track = {};
		track.type = Timeline::TYPE_CAMERA;
		track.target = camera;
		track.camera.camera_keys =
		{
			{ 0, Easing::LINEAR, star },
			{ 120, Easing::LINEAR, nullptr }
		};
		timeline->AddTrack(track);
	}

	{
		Timeline::Track track = {};
		track.type = Timeline::TYPE_ANIMATION;
		track.target = camera;
		{
			auto animation1 = std::make_shared<Animation>();
			{
				Animation::Channel channel = {};
				channel.type = Animation::TYPE_TRANSFORM;
				channel.transform.position_keys =
				{
					{ 0, Easing::LINEAR, Vec3(5, 3, 0) },
					{ 110, Easing::OUT_QUAD, Vec3(0, 3, 20) },
					{ 160, Easing::LINEAR, Vec3(0, 3, 20) },
					{ 190, Easing::OUT_CUBIC, Vec3(5, 3, 20) },
				};
				animation1->AddChannel(channel);
			}
			animation1->SetDuration(250);
			animation1->SetTicksPerSecond(60);

			auto animation2 = std::make_shared<Animation>();
			{
				Animation::Channel channel = {};
				channel.type = Animation::TYPE_TRANSFORM;
				channel.transform.rotation_keys =
				{
					{ 0, Easing::LINEAR, Quaternion::FromEuler(0, 0, 0) },
					{ 60, Easing::OUT_QUAD, Quaternion::FromEuler(0.25f, 0, 0) },
				};
				animation2->AddChannel(channel);
			}
			animation2->SetDuration(130);
			animation2->SetTicksPerSecond(60);

			{
				Timeline::AnimationTrack anim_track;
				anim_track.animation_keys =
				{
					{ 0, Easing::LINEAR, animation1, 1, false },
				};
				track.animation.push_back(anim_track);
			}
			{
				Timeline::AnimationTrack anim_track;
				anim_track.animation_keys =
				{
					{ 120, Easing::LINEAR, animation2, 1, false },
				};
				track.animation.push_back(anim_track);
			}
		}
		timeline->AddTrack(track);
	}

	// サウンド
	{
		Timeline::Track track = {};
		track.type = Timeline::TYPE_AUDIO;
		track.target = this;
		track.audio.audio_keys =
		{
			{ 0, Easing::LINEAR, game->LoadResource<Audio>(L"assets/bgm/MusMus-JGL-025.wav"), false, 0.5f },
		};
		timeline->AddTrack(track);
	}

	timeline->SetDuration(360);

	AddComponent(new AudioSource());
	AddComponent(new TimelinePlayer(timeline));
}
