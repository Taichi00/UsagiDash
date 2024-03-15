#include "metal_ball_controller.h"
#include "game/component/audio/audio_source.h"
#include "game/component/collider/collider.h"
#include "game/entity.h"

MetalBallController::MetalBallController(AudioSource* audio_rolling)
{
	audio_rolling_ = audio_rolling;
}

MetalBallController::~MetalBallController()
{
}

void MetalBallController::OnCollisionEnter(Collider* collider)
{
	if (collider->GetEntity()->tag == "map")
	{
		audio_rolling_->Playing(4, true);
	}
}
