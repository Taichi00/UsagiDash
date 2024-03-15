#include "metal_ball_emitter.h"
#include "app/component/metal_ball_emitter_controller.h"
#include "app/component/pause_behavior.h"

MetalBallEmitter::MetalBallEmitter(const float radius, const float width, const float spawn_rate) : Entity("metal_ball_emitter")
{
	AddComponent(new MetalBallEmitterController(radius, width, spawn_rate));
	AddComponent(new PauseBehavior());
}
