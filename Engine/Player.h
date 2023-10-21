#pragma once

#include "Component.h"
#include "Vec.h"

class Animator;

struct PlayerProperty
{
	float Speed;
	float Acceleration;
};

class Player : public Component
{
public:
	Player(PlayerProperty prop);
	~Player();

	bool Init();
	void Update();

private:
	void Move();
	void Animate();

private:
	Vec3 m_velocity;
	float m_speed;
	float m_acceleration;
	float m_angle;

	bool m_isRunning, _m_isRunning;

	Animator* m_pAnimator;
};