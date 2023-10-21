#include "Transform.h"

bool Transform::Init()
{
	position = Vec3::Zero();
	scale = Vec3(1, 1, 1);
	rotation = Quaternion::identity();
	return true;
}
