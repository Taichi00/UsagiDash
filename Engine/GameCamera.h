#pragma once

#include "Component.h"
#include "Vec.h"

class Entity;
class Camera;

struct GameCameraProperty
{
	Entity* Target;
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
	Entity* m_pTarget;

	Camera* m_pCamera;

	Vec3 m_angleVelocity;
	Vec3 m_angle;
	float m_distance = 20;
	float m_currDistance;
};