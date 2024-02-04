#include "game/component/collider/ray.h"
#include "game/component/collider/mesh_collider.h"
#include "game/component/collider/sphere_collider.h"

Ray::Ray(Vec3 origin, Vec3 direction, float distance)
{
	this->origin = origin;
	this->direction = direction.Normalized();
	this->distance = distance;

	aabb_ = AABB{};
	aabb_.max = Vec3::Max(this->origin, this->origin + this->direction * this->distance);
	aabb_.min = Vec3::Min(this->origin, this->origin + this->direction * this->distance);
	
	hits_.clear();
	nearest_hit_ = { nullptr, Vec3::Zero(), 0 };
}

Ray::~Ray()
{
}

bool Ray::Intersects(SphereCollider* sphere)
{
	return sphere->Intersects(this);
}

bool Ray::Intersects(CapsuleCollider* sphere)
{
	return false;
}

bool Ray::Intersects(FloorCollider* floor)
{
	return false;
}

bool Ray::Intersects(MeshCollider* collider)
{
	return collider->Intersects(this);
}

AABB Ray::GetAABB()
{
	return aabb_;
}

void Ray::AddHit(Collider::HitInfo hit)
{
	hits_.push_back(hit);

	if (hit.depth > nearest_hit_.depth)
	{
		nearest_hit_ = hit;
	}
}

const std::vector<Collider::HitInfo>& Ray::GetHits()
{
	return hits_;
}

const Collider::HitInfo& Ray::GetNearestHit()
{
	return nearest_hit_;
}
