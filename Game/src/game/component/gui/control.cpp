#include "control.h"
#include "game/entity.h"
#include "game/game.h"
#include "engine/engine.h"
#include "engine/engine2d.h"
#include "game/component/gui/element/element.h"
#include "game/gui_manager.h"
#include <algorithm>

Control::Control()
{
	engine_ = Game::Get()->GetEngine()->GetEngine2D();

	Game::Get()->GetGUIManager()->Add(this);

	parent_size_ = Vec2::Zero();
	parent_position_ = Vec2::Zero();
	rect_ = { 0, 0, 0, 0 };
	world_matrix_ = Matrix3x2::Identity();
}

Control::~Control()
{
	Game::Get()->GetGUIManager()->Remove(this);
}

bool Control::Init()
{
	parent_control_ = GetEntity()->Parent()->GetComponent<Control>();

	for (auto& element : element_map_)
	{
		element.second->Init();
	}

	Update(0);

	return true;
}

void Control::Update(const float delta_time)
{
	Layout();

	for (auto& element : element_map_)
	{
		element.second->Update();
	}
}

void Control::Draw2D()
{
	std::vector<ElementInfo> element_z_list;
	for (auto& element : element_map_)
	{
		element_z_list.push_back({ element.second.get(), element.second->ZIndex() });
	}

	// z index でソートする
	std::sort(element_z_list.begin(), element_z_list.end());

	// 描画
	for (auto& info : element_z_list)
	{
		info.element->Draw();
	}
}

void Control::SetTransform(const Vec2& position, const Vec2& size, const Vec2& pivot, const Vec2& anchor_pos)
{
	position_ = position;
	size_ = size;
	pivot_ = pivot;
	anchor_ = { anchor_pos.x, anchor_pos.y, anchor_pos.x, anchor_pos.y };

	Transform();
}

void Control::Transform()
{
	// position, size, pivotからoffsetを逆算する
	offset_.left = position_.x - pivot_.x * size_.x;
	offset_.top = position_.y - pivot_.y * size_.y;
	offset_.right = -position_.x - (1 - pivot_.x) * size_.x;
	offset_.bottom = -position_.y - (1 - pivot_.y) * size_.y;
}

Vec2 Control::WorldPosition() const
{
	return Vec2(world_matrix_.m31, world_matrix_.m32);
}

void Control::Layout()
{
	auto ratio = engine_->RenderTargetSize().y / 720.0;
	Matrix3x2 parent_matrix = Matrix3x2::Identity();

	// 親Controlのサイズ、位置を取得
	if (parent_control_)
	{
		auto rect = parent_control_->rect_;
		parent_size_ = Vec2(rect.Width(), rect.Height());
		parent_position_ = Vec2(rect.left, rect.top);
		parent_matrix = parent_control_->world_matrix_;
		parent_color_ = parent_control_->color_;
	}
	else
	{
		// 親がnullptrの場合はレンダーターゲットのサイズ を取得
		parent_size_ = engine_->RenderTargetSize() / engine_->RenderTargetScale();
		parent_position_ = Vec2::Zero();
		parent_color_ = Color(1, 1, 1, 1);
	}

	// サイズの算出
	Vec2 size;
	size.x = (anchor_.right - anchor_.left) * parent_size_.x - offset_.right - offset_.left;
	size.y = (anchor_.bottom - anchor_.top) * parent_size_.y - offset_.bottom - offset_.top;
	
	// 座標の算出
	Vec2 pos;
	pos.x = anchor_.left * parent_size_.x + offset_.left + pivot_.x * size.x;
	pos.y = anchor_.top * parent_size_.y + offset_.top + pivot_.y * size.y;
	/*printf("%s\n", pos.GetString().c_str());*/
	rect_ = { 0, 0, size.x, size.y };

	// ワールド変換行列の生成
	Matrix3x2 mat = Matrix3x2::Identity();
	mat = mat * Matrix3x2::Translation(-Vec2(pivot_.x * size.x, pivot_.y * size.y));
	mat = mat * Matrix3x2::Scale(scale_);
	mat = mat * Matrix3x2::Rotation(rotation_);
	mat = mat * Matrix3x2::Translation(pos);

	world_matrix_ = mat * parent_matrix;

	world_color_ = parent_color_ * color_;
}
