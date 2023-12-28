#include "Ray.h"
#include "MeshCollider.h"
#include "SphereCollider.h"

Ray::Ray(Vec3 origin, Vec3 direction, float distance)
{
	this->origin = origin;
	this->direction = direction.normalized();
	this->distance = distance;
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
