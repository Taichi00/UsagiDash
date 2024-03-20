#pragma once

#include "game/component/component.h"
#include "game/component/gui/element/element.h"
#include "math/vec.h"
#include "math/rect.h"
#include "math/matrix3x2.h"
#include "math/color.h"
#include <map>
#include <memory>
#include <vector>

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

	void SetPosition(const Vec2& position) { position_ = position; }
	void SetSize(const Vec2& size) { size_ = size; }
	void SetPivot(const Vec2& pivot) { pivot_ = pivot; }
	void SetAnchorPoint(const Vec2& point) { anchor_ = { point.x, point.y, point.x, point.y }; }
	void Transform();

	void SetRotation(const float rotation) { rotation_ = rotation; }
	void SetScale(const Vec2& scale) { scale_ = scale; }
	void SetOffset(const Rect2& offset) { offset_ = offset; }
	void SetAnchor(const Rect2& anchor) { anchor_ = anchor; }
	void SetColor(const Color& color) { color_ = color; }
	void SetZIndex(const int z_index) { z_index_ = z_index; }

	Vec2 Size() const { return Vec2(rect_.Width(), rect_.Height()); }
	Vec2 Pivot() const { return pivot_; }
	float Rotation() const { return rotation_; }
	Vec2 Scale() const { return scale_; }
	Rect2 Offset() const { return offset_; }
	Rect2 Anchor() const { return anchor_; }
	Control* ParentControl() const { return parent_control_; }
	Rect2 GetRect() const { return rect_; };
	Matrix3x2 WorldMatrix() const { return world_matrix_; }
	Color GetColor() const { return color_; }
	Color WorldColor() const { return world_color_; }
	int ZIndex() const { return z_index_; }

	Vec2 WorldPosition() const;

	// UI—v‘f‚ðŽæ“¾‚·‚é
	template <typename T>
	T* GetElement(const std::string& name)
	{
		return (T*)element_map_[name].get();
	}

	Element* GetElement(const std::string& name) { return element_map_[name].get(); }

protected:
	void Layout();

	Vec2 ParentSize() const { return parent_size_; }

	// UI—v‘f‚ð’Ç‰Á‚·‚é
	template <typename T>
	T* AddElement(const std::string& name, T* element)
	{
		element_map_[name] = std::unique_ptr<Element>(element);
		return element;
	}

protected:
	Engine2D* engine_ = nullptr;

private:
	Vec2 position_ = Vec2(0, 0);
	Vec2 size_ = Vec2(1, 1);
	float rotation_ = 0.0;
	Vec2 scale_ = Vec2(1, 1);
	Color color_ = Color::White();
	int z_index_ = 0;

	Rect2 offset_ = { 0, 0, 0, 0 };
	Rect2 anchor_ = { 0, 0, 0, 0 };
	Vec2 pivot_ = Vec2(0, 0);

	Control* parent_control_ = nullptr;

	Vec2 parent_size_;
	Vec2 parent_position_;
	Color parent_color_;

	Rect2 rect_;

	Matrix3x2 world_matrix_;

	Color world_color_;

	// UI—v‘f‚Ìƒ}ƒbƒv
	std::map<std::string, std::unique_ptr<Element>> element_map_;

	struct ElementInfo
	{
		Element* element = nullptr;
		int z_index = 0;

		bool operator<(const ElementInfo& info) const
		{
			return z_index < info.z_index;
		}
	};
};