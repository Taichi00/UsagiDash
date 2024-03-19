#define NOMINMAX

#include "star_controller.h"
#include "game/component/transform.h"
#include "game/component/collider/collider.h"
#include "game/entity.h"
#include "game/game.h"
#include "math/easing.h"
#include "game/component/particle_emitter.h"
#include "game/component/audio/audio_source.h"
#include "app/component/pause_manager.h"
#include "app/component/player_controller_2.h"
#include "game/component/rigidbody.h"
#include "game/component/timeline_player.h"
#include "app/component/game_manager.h"
#include "game/physics.h"
#include "game/component/camera.h"

StarController::StarController(ParticleEmitter* confetti,
	AudioSource* audio_twinkle,
	AudioSource* audio_pick
)
{
	confetti_ = confetti;
	audio_twinkle_ = audio_twinkle;
	audio_pick_ = audio_pick;
}

StarController::~StarController()
{
}

bool StarController::Init()
{
	timeline_player_ = GetEntity()->GetComponent<TimelinePlayer>();

	// キラキラ音を再生
	audio_twinkle_->Play(true);

	return true;
}

void StarController::Update(const float delta_time)
{
	// 回転
	transform->rotation = Quaternion::FromEuler(angle_, angle_, 0);

	angle_ = std::fmod(angle_ + angle_speed_ * delta_time, 2.0f * (float)PI);
}

void StarController::OnCollisionEnter(Collider* collider)
{
	if (collider->GetEntity()->tag == "player")
	{
		audio_twinkle_->Stop();
		audio_pick_->Play();
		confetti_->Emit();

		// ステージクリア演出
		GameManager::Get()->StageClear(GetEntity());
	}
}
