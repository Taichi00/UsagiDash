#include "app/game_camera.h"
#include "game/input/input.h"
#include "game/entity.h"
#include "game/component/camera.h"
#include "game/physics.h"
#include "app/player.h"
#include <dinput.h>

GameCamera::GameCamera(Entity* target)
{
	angle_speed_ = 2.4f;
	angle_acceleration_ = 0.12f;
	distance_ = 20.f;

	target_ = target;
	angle_ = Vec3(0.5, 0, 0);
	current_distance_ = distance_;
}

GameCamera::~GameCamera()
{
}

bool GameCamera::Init()
{
	camera_ = GetEntity()->GetComponent<Camera>();
	return true;
}

void GameCamera::BeforeCameraUpdate(const float delta_time)
{
	Move(delta_time);
}

void GameCamera::Move(const float delta_time)
{
	auto input_type = Input::CurrentInputType();

	Vec2 input_dir;
	input_dir.x = Input::GetAxis("camera_horizontal");
	input_dir.y = Input::GetAxis("camera_vertical");
	float input_length = input_dir.Length();

	Vec2 angle_speed = Vec2(1, 1) * angle_speed_;
	float angle_acceleration = angle_acceleration_;
	float angle_friction = 0.92f;

	if (input_type == Input::InputType::GAMEPAD)
	{
		auto speed = angle_speed * 1.2f;
		if (std::abs(angle_velocity_.y) < speed.y * std::abs(input_dir.x) * 1.1f)
		{
			angle_speed.y = speed.y * std::abs(input_dir.x);
		}
		else
		{
			angle_speed.y = std::abs(angle_velocity_.y);
		}

		if (std::abs(angle_velocity_.x) < speed.x * std::abs(input_dir.y) * 1.1f)
		{
			angle_speed.x = speed.x * std::abs(input_dir.y);
		}
		else
		{
			angle_speed.x = std::abs(angle_velocity_.x);
		}

		angle_acceleration = angle_acceleration_ * 1.5f;
		angle_friction = 0.9f;
	}

	angle_velocity_.y +=  input_dir.x * angle_acceleration;
	angle_velocity_.x += -input_dir.y * angle_acceleration;

	if (input_dir.y == 0.f)
		angle_velocity_.x *= angle_friction;
	if (input_dir.x == 0.f)
		angle_velocity_.y *= angle_friction;

	angle_velocity_.y = std::min(std::max(angle_velocity_.y, -angle_speed.y), angle_speed.y);
	angle_velocity_.x = std::min(std::max(angle_velocity_.x, -angle_speed.x), angle_speed.x);
	
	angle_ += angle_velocity_ * delta_time;
	
	if (angle_.x < -1.5)
	{
		angle_.x = -1.5;
		angle_velocity_.x = 0;
	}
	else if (angle_.x > 1.5)
	{
		angle_.x = 1.5;
		angle_velocity_.x = 0;
	}

	Vec3 targetPos = target_->transform->position + Vec3(0, 3, 0);
	Vec3 currPos = camera_->GetFocusPosition();
	Vec3 v = targetPos - currPos;

	float rate = 0.05f;
	auto origin = currPos + v * rate;
	auto direction = Quaternion::FromEuler(angle_.x, angle_.y, angle_.z) * Vec3(0, 0, 1);
	auto distance = distance_ * std::max(0.5f, std::min(1.5f, 0.75f + angle_.x * 0.8f));

	// めり込み防止
	RaycastHit hit;
	if (Physics::Raycast(origin, direction, distance, hit, {"map"}))
	{
		distance = std::max(hit.distance - 0.1f, 0.f);
	}

	// ズームインは素早く、ズームアウトはゆっくり
	auto drate = 0.15f;
	if (distance - current_distance_ > 0) drate = 0.02f;

	distance = current_distance_ + (distance - current_distance_) * pow(drate, delta_time * 60);
	
	transform->position = Vec3(origin + direction * distance);
	camera_->SetFocusPosition(origin);

	current_distance_ = distance;
}
