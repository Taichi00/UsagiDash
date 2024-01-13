#pragma once

#include "game/component/component.h"
#include "math/vec.h"

class Entity;
class Camera;

struct GameCameraProperty
{
	Entity* target;
};

class GameCamera : public Component
{
public:
	GameCamera(GameCameraProperty prop);
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