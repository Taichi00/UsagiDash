#pragma once

#include "Vec.h"
#include "Quaternion.h"

class Particle
{
public:
	Particle();
	~Particle();

public:
	float timeToLive;	// ��������
	float time;			// �o�ߎ���

	Vec3 position;		// �ʒu
	Vec3 positionVelocity;
	Vec3 positionAcceleration;
	Vec3 positionStart;
	Vec3 positionEnd;
	Vec3 positionMiddle;

	Vec3 rotation;		// �p�x
	Vec3 rotationVelocity;
	Vec3 rotationAcceleration;
	Vec3 rotationStart;
	Vec3 rotationEnd;
	Vec3 rotationMiddle;

	Vec3 scale;			// �g�嗦
	Vec3 scaleVelocity;
	Vec3 scaleAcceleration;
	Vec3 scaleStart;
	Vec3 scaleEnd;
	Vec3 scaleMiddle;

	Quaternion localDirection;	// �i�s����

	float width, height;
};