#pragma once

#include "math/octree.h"
#include <vector>
#include <memory>
#include <string>
#include <utility>
#include <set>

class Collider;
class Rigidbody;
class Ray;
class Entity;

class CollisionManager
{
public:
	CollisionManager();

	void Update(const float delta_time);

	void Add(Collider* collider);
	void Add(Rigidbody* rigidbody);

	void Remove(Collider* collider);
	void Remove(Rigidbody* rigidbody);

	void Detect(Ray* ray, const std::vector<std::string>& mask_tags = {});

	void SetNonCollisionTags(const std::string& tag1, const std::string& tag2);

private:
	std::vector<Collider*> colliders_;
	std::vector<Rigidbody*> rigidbodies_;

	std::vector<std::pair<std::string, std::string>> non_collision_tags_;

	std::set<std::pair<Entity*, Collider*>> collided_entities_;

	std::unique_ptr<Octree<Collider>> octree_;
	std::vector<std::unique_ptr<OctreeObject<Collider>>> octree_objects_;
};