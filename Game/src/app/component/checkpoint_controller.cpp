#include "checkpoint_controller.h"
#include "game/component/collider/collider.h"
#include "game/entity.h"
#include "app/component/game_manager.h"
#include "game/component/particle_emitter.h"
#include "game/component/animator.h"
#include "game/component/audio/audio_source.h"

CheckpointController::CheckpointController()
{
}

bool CheckpointController::Init()
{
	game_manager_ = GameManager::Get();

	confetti_emitter_ = GetEntity()->Child("confetti_emitter")->GetComponent<ParticleEmitter>();
	smoke_emitter_ = GetEntity()->Child("smoke_emitter")->GetComponent<ParticleEmitter>();
	animator_ = GetEntity()->GetComponent<Animator>();
	audio_ = GetEntity()->GetComponent<AudioSource>();

	animator_->Play("Waving", 4);

	return true;
}

void CheckpointController::OnCollisionEnter(Collider* collider)
{
	if (game_manager_->CurrentCheckpoint() == GetEntity())
		return;

	if (collider->GetEntity()->tag == "player")
	{
		// プレイヤーのリスポーン位置を設定
		auto position = transform->position + Vec3(0, 5, 0);
		game_manager_->SetCurrentCheckpoint(GetEntity());

		confetti_emitter_->Emit();
		smoke_emitter_->Emit();

		animator_->Play("Jump", 2.5f, false, 0);
		animator_->Push("Waving", 4);

		audio_->SetPitch(0.9f);
		audio_->Play(0.6f);
	}
}
