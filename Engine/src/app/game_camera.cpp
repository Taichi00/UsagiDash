#include "app/game_camera.h"
#include "game/input.h"
#include "game/entity.h"
#include "game/component/camera.h"
#include "game/physics.h"
#include "app/player.h"

GameCamera::GameCamera(GameCameraProperty prop)
{
	target_ = prop.target;
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

void GameCamera::BeforeCameraUpdate()
{
	Move();
}

void GameCamera::Move()
{
	float rate = 0.05;

	if (Input::GetKey(DIK_A))
	{
		angle_velocity_.y += -0.002;
	}
	else if (Input::GetKey(DIK_D))
	{
		angle_velocity_.y += 0.002;
	}
	else
	{
		angle_velocity_.y *= 0.92;
	}

	if (Input::GetKey(DIK_W))
	{
		angle_velocity_.x += -0.002;
	}
	else if (Input::GetKey(DIK_S))
	{
		angle_velocity_.x += 0.002;
	}
	else
	{
		angle_velocity_.x *= 0.92;
	}

	angle_velocity_.y = min(max(angle_velocity_.y, -0.04), 0.04);
	angle_velocity_.x = min(max(angle_velocity_.x, -0.04), 0.04);
	angle_ += angle_velocity_;

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

	auto origin = currPos + v * rate;
	auto direction = Quaternion::FromEuler(angle_.x, angle_.y, angle_.z) * Vec3(0, 0, 1);
	auto distance = distance_ * max(0.5, min(1.5, 0.75 + angle_.x * 0.8));

	// めり込み防止
	RaycastHit hit;
	if (Physics::Raycast(origin, direction, distance, hit))
	{
		distance = max(hit.distance - 0.1, 0);
	}

	// ズームインは素早く、ズームアウトはゆっくり
	auto drate = 0.15;
	if (distance - current_distance_ > 0) drate = 0.02;

	distance = current_distance_ + (distance - current_distance_) * drate;
	
	transform->position = Vec3(origin + direction * distance);
	camera_->SetFocusPosition(origin);

	current_distance_ = distance;
}
