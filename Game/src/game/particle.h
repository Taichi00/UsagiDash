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
	float time_to_live;	// 生存時間
	float time;			// 経過時間

	Vec3 position;		// 位置
	Vec3 position_velocity;
	Vec3 position_acceleration;
	Vec3 position_start;
	Vec3 position_end;
	Vec3 position_middle;

	Vec3 rotation;		// 角度
	Vec3 rotation_velocity;
	Vec3 rotation_acceleration;
	Vec3 rotation_start;
	Vec3 rotation_end;
	Vec3 rotation_middle;

	Vec3 scale;			// 拡大率
	Vec3 scale_velocity;
	Vec3 scale_acceleration;
	Vec3 scale_start;
	Vec3 scale_end;
	Vec3 scale_middle;

	Color color; // 色
	Color color_velocity;
	Color color_acceleration;
	Color color_start;
	Color color_end;
	Color color_middle;
	Color final_color; // 最終的に表示する色

	Quaternion local_direction;	// 進行方向

	float width, height;
};