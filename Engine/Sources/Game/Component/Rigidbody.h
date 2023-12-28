#pragma once

#include "Component.h"
#include "Vec.h"

class Collider;

struct RigidbodyProperty
{
	Collider* Collider;
	float Mass;			// ¿—Ê
	bool UseGravity;	// d—Í‚Ì‰e‹¿‚ğó‚¯‚é‚©
	bool IsStatic;		// •s“®‚©‚Ç‚¤‚©
	float Friction;		// –€C—ÍŒW”
};

class Rigidbody : public Component
{
public:
	Rigidbody(RigidbodyProperty prop);
	~Rigidbody();

	bool Init();

	void Resolve();

public:
	Vec3 velocity, prevVelocity;
	float mass;
	bool useGravity;
	bool isStatic;
	float friction;

	Collider* collider;

	bool isGrounded;
	Vec3 floorNormal;
	Vec3 floorVelocity;
};