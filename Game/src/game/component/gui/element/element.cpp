#include "element.h"
#include "game/component/gui/control.h"
#include "game/game.h"
#include "engine/engine.h"
#include "engine/engine2d.h"

Element::Element()
{
	engine_ = Game::Get()->GetEngine()->GetEngine2D();
}

Element::Element(Control* control) : Element()
{
	control_ = control;
}

Element::~Element()
{
}

void Element::Init()
{
}

void Element::Update()
{
	auto render_scale = engine_->RenderTargetScale();

	auto pivot_pos = control_->Size() * control_->Pivot();

	world_matrix_ =
		Matrix3x2::Translation(-pivot_pos) * 
		Matrix3x2::Scale(scale_) *
		Matrix3x2::Rotation(rotation_) *
		Matrix3x2::Translation(position_ + pivot_pos) *
		control_->WorldMatrix() *
		Matrix3x2::Scale(Vec2(render_scale, render_scale));
}

void Element::Draw()
{
	engine_->SetTransform(world_matrix_);
}
