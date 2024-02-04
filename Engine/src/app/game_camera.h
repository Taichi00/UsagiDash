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

	bool Init();
	void BeforeCameraUpdate();

private:
	void Move();

private:
	Entity* target_;

	Camera* camera_;

	Vec3 angle_velocity_;
	Vec3 angle_;
	float distance_ = 20;
	float current_distance_;
};