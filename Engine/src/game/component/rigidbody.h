#pragma once

#include "game/component/component.h"
#include "math/vec.h"

class Collider;

struct RigidbodyProperty
{
	Collider* collider;
	float mass;			// ����
	bool use_gravity;	// �d�͂̉e�����󂯂邩
	bool is_static;		// �s�����ǂ���
	float friction;		// ���C�͌W��
};

class Rigidbody : public Component
{
public:
	Rigidbody(RigidbodyProperty prop);
	~Rigidbody();

	bool Init();

	void Resolve();

public:
	Vec3 velocity, velocity_prev;
	float mass;
	bool use_gravity;
	bool is_static;
	float friction;

	Collider* collider;

	bool is_grounded;

	Rigidbody* floor_rigidbody;
	Vec3 floor_normal;
	Vec3 floor_velocity;
};