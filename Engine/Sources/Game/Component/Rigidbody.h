#pragma once

#include "Component.h"
#include "Vec.h"

class Collider;

struct RigidbodyProperty
{
	Collider* Collider;
	float Mass;			// ����
	bool UseGravity;	// �d�͂̉e�����󂯂邩
	bool IsStatic;		// �s�����ǂ���
	float Friction;		// ���C�͌W��
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