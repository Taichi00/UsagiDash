#pragma once

#include "math/vec.h"
#include "math/color.h"
#include "math/matrix3x2.h"

class Control;
class Engine2D;

class Element
{
public:
	Element();
	Element(Control* control);
	virtual ~Element();

	virtual void Init();
	virtual void Update();
	virtual void Draw();

	Vec2 Position() const { return position_; }
	Vec2 Scale() const { return scale_; }
	float Rotation() const { return rotation_; }
	Color GetColor() const { return color_; }
	int ZIndex() const { return z_index_; }

	void SetPosition(const Vec2& position) { position_ = position; }
	void SetScale(const Vec2& scale) { scale_ = scale; }
	void SetRotation(float rotation) { rotation_ = rotation; }
	void SetColor(const Color& color) { color_ = color; }
	void SetZIndex(const int z_index) { z_index_ = z_index; }

protected:
	Control* control_ = nullptr;

	Vec2 position_;
	Vec2 scale_ = Vec2(1, 1);
	float rotation_ = 0;
	Color color_ = Color::White();
	int z_index_ = 0;

	Matrix3x2 world_matrix_ = Matrix3x2::Identity();

	Engine2D* engine_ = nullptr;
};