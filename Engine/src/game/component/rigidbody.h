#pragma once

#include "game/component/component.h"
#include "math/vec.h"
#include <memory>

class Collider;
class CollisionManager;

class Rigidbody : public Component
{
public:
	Rigidbody(float mass, bool use_gravity, bool is_static, float friction);
	~Rigidbody();

	bool Init();

	void Prepare(const float delta_time);
	void Resolve();

public:
	Vec3 position, position_prev;
	Vec3 velocity, velocity_prev;
	float mass; // ¿—Ê
	bool use_gravity; // d—Í‚Ì‰e‹¿‚ğó‚¯‚é‚©
	bool is_static; // •s“®‚©‚Ç‚¤‚©
	float friction; // –€C—ÍŒW”

	Collider* collider;

	bool is_grounded;
	bool is_touching_wall;

	Rigidbody* floor_rigidbody;
	Vec3 floor_normal;
	Vec3 floor_velocity;

	Vec3 wall_normal;

	CollisionManager* collision_manager_;
};