#pragma once

#include "game/component/component.h"
#include "math/vec.h"
#include "math/rect.h"
#include "math/matrix3x2.h"

class Engine2D;

class Control : public Component
{
public:
	Control();
	~Control();

	bool Init() override;
	void Update(const float delta_time) override;
	void Draw2D() override;

	void SetTransform(const Vec2& position, const Vec2& size, const Vec2& pivot, const Vec2& anchor_pos);

	void SetRotation(const float rotation) { rotation_ = rotation; }
	void SetScale(const Vec2& scale) { scale_ = scale; }
	void SetOffset(const Rect2& offset) { offset_ = offset; }
	void SetAnchor(const Rect2& anchor) { anchor_ = anchor; }

	float Rotation() const { return rotation_; }
	Vec2 Scale() const { return scale_; }
	Rect2 Offset() const { return offset_; }
	Rect2 Anchor() const { return anchor_; }
	Control* ParentControl() const { return parent_control_; }
	Rect2 GetRect() const { return rect_; };
	Matrix3x2 WorldMatrix() const { return world_matrix_; }

protected:
	void Layout();

	Vec2 ParentSize() const { return parent_size_; }

protected:
	Engine2D* engine_ = nullptr;

private:
	float rotation_ = 0.0;
	Vec2 scale_ = Vec2(1, 1);

	Rect2 offset_ = { 0, 0, 0, 0 };
	Rect2 anchor_ = { 0, 0, 0, 0 };
	Vec2 pivot_ = Vec2(0, 0);

	Control* parent_control_ = nullptr;

	Vec2 parent_size_;
	Vec2 parent_position_;

	Rect2 rect_;

	Matrix3x2 world_matrix_;
};