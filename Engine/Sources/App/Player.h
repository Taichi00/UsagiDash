#pragma once

#include "Component.h"
#include "Vec.h"

class Animator;
class Rigidbody;
class ParticleEmitter;

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
	Vec3 m_moveDirection;
	float m_speed;
	float m_acceleration;
	float m_angle;

	bool m_isRunning, _m_isRunning;
	bool m_isGrounded, _m_isGrounded;
	int m_airCount = 0;
	int m_jumpFrame, _m_jumpFrame;
	int m_jumpFrameMax = 15;

	Animator* m_pAnimator;
	Rigidbody* m_pRigidbody;
};