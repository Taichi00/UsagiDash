#pragma once

#include "Vec.h"
#include "Quaternion.h"

class Particle
{
public:
	Particle();
	~Particle();

public:
	float timeToLive;	// 生存時間
	float time;			// 経過時間

	Vec3 position;		// 位置
	Vec3 positionVelocity;
	Vec3 positionAcceleration;
	Vec3 positionStart;
	Vec3 positionEnd;
	Vec3 positionMiddle;

	Vec3 rotation;		// 角度
	Vec3 rotationVelocity;
	Vec3 rotationAcceleration;
	Vec3 rotationStart;
	Vec3 rotationEnd;
	Vec3 rotationMiddle;

	Vec3 scale;			// 拡大率
	Vec3 scaleVelocity;
	Vec3 scaleAcceleration;
	Vec3 scaleStart;
	Vec3 scaleEnd;
	Vec3 scaleMiddle;

	Quaternion localDirection;	// 進行方向

	float width, height;
};