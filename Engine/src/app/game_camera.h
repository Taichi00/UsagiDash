#pragma once

#include "game/component/component.h"
#include "math/vec.h"

class Entity;
class Camera;

class GameCamera : public Component
{
public:
	GameCamera(Entity* target);
	~GameCamera();

	bool Init() override;
	void BeforeCameraUpdate(const float delta_time) override;

private:
	void Move(const float delta_time);

private:
	Entity* target_;

	Camera* camera_;

	Vec3 angle_velocity_;
	Vec3 angle_;
	float distance_ = 20;
	float current_distance_;
};