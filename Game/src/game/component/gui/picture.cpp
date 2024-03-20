#include "picture.h"
#include "game/game.h"
#include "engine/engine2d.h"
#include "game/resource/bitmap.h"

Picture::Picture()
{
	bitmap_ = nullptr;
}

Picture::Picture(const std::shared_ptr<Bitmap> bitmap)
{
	bitmap_ = bitmap;

	SetSize(bitmap->Size());
	Transform();
}

void Picture::Load(const std::shared_ptr<Bitmap> bitmap)
{
	if (!bitmap)
		return;
	
	bitmap_ = bitmap;

	SetSize(bitmap->Size());
	Transform();
}

bool Picture::Init()
{
	Control::Init();

	return true;
}

void Picture::Draw2D()
{
	if (!bitmap_)
		return;

	auto scale = engine_->RenderTargetScale();

	auto world_matrix = WorldMatrix();

	engine_->SetTransform(world_matrix * Matrix3x2::Scale(Vec2(1, 1) * scale));
	engine_->DrawBitmap(bitmap_.get(), WorldColor());
}