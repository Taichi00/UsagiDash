#pragma once

#include "math/vec.h"
#include "math/quaternion.h"
#include "math/color.h"

class Particle
{
public:
	Particle();
	~Particle();

public:
	float time_to_live;	// ��������
	float time;			// �o�ߎ���

	Vec3 position;		// �ʒu
	Vec3 position_velocity;
	Vec3 position_acceleration;
	Vec3 position_start;
	Vec3 position_end;
	Vec3 position_middle;

	Vec3 rotation;		// �p�x
	Vec3 rotation_velocity;
	Vec3 rotation_acceleration;
	Vec3 rotation_start;
	Vec3 rotation_end;
	Vec3 rotation_middle;

	Vec3 scale;			// �g�嗦
	Vec3 scale_velocity;
	Vec3 scale_acceleration;
	Vec3 scale_start;
	Vec3 scale_end;
	Vec3 scale_middle;

	Color color; // �F
	Color color_velocity;
	Color color_acceleration;
	Color color_start;
	Color color_end;
	Color color_middle;
	Color final_color; // �ŏI�I�ɕ\������F

	Quaternion local_direction;	// �i�s����

	float width, height;
};