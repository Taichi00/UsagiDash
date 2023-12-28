#pragma once

#include "Component.h"
#include "Vec.h"

class Animator;
class Rigidbody;

struct FloatingPlayerProperty
{
	float Speed;
	float Acceleration;
};

class FloatingPlayer : public Component
{
public:
	FloatingPlayer(FloatingPlayerProperty prop);
	~FloatingPlayer();

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
	Rigidbody* m_pRigidbody;
};