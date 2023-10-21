#pragma once

#include "Component.h"
#include "Vec.h"
#include "Quaternion.h"

class Transform : public Component
{
public:
	bool Init();

public:
	Vec3 position;
	Vec3 scale;
	Quaternion rotation;
};