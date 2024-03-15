#include "metal_ball_emitter_controller.h"
#include "app/entity/metal_ball.h"
#include "game/scene.h"

MetalBallEmitterController::MetalBallEmitterController(const float radius, const float width, const float spawn_rate)
{
	radius_ = radius;
	width_ = width;
	spawn_rate_ = spawn_rate;

	// óêêîê∂ê¨
	std::random_device rd;
	rand_ = std::mt19937(rd());
}

MetalBallEmitterController::~MetalBallEmitterController()
{
}

bool MetalBallEmitterController::Init()
{
	return true;
}

void MetalBallEmitterController::Update(const float delta_time)
{
	if (spawn_rate_ == 0)
		return;

	std::uniform_real_distribution<float> frand(-1, 1);
	auto interval = 1.0f / spawn_rate_;

	if (spawn_timer_ >= interval)
	{
		spawn_timer_ -= interval;

		// ìSãÖÇê∂ê¨
		auto ball = GetScene()->CreateEntity(new MetalBall(radius_));

		auto position = transform->WorldPosition();
		position += transform->rotation * Vec3(width_ / 2.0f, 0, 0) * frand(rand_);

		ball->transform->position = position;
	}

	spawn_timer_ += 60.0f * delta_time;
}
