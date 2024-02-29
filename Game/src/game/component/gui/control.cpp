#include "control.h"
#include "game/entity.h"
#include "game/game.h"
#include "engine/engine.h"
#include "engine/engine2d.h"

Control::Control()
{
	engine_ = Game::Get()->GetEngine()->GetEngine2D();

	parent_size_ = Vec2::Zero();
	parent_position_ = Vec2::Zero();
	rect_ = { 0, 0, 0, 0 };
	world_matrix_ = Matrix3x2::Identity();
}

Control::~Control()
{
}

bool Control::Init()
{
	parent_control_ = GetEntity()->GetParent()->GetComponent<Control>();

	return true;
}

void Control::Update(const float delta_time)
{
	Layout();
}

void Control::Draw2D()
{
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
	}
	else
	{
		// 親がnullptrの場合はレンダーターゲットのサイズ を取得
		parent_size_ = Vec2(1280, 720);//engine_->RenderTargetSize();
		parent_position_ = Vec2::Zero();
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
}
