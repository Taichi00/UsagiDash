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
}

bool Picture::Init()
{
	Control::Init();

	return true;
}

void Picture::Draw2D()
{
	auto ratio = engine_->RenderTargetSize().y / 720.0f;

	auto world_matrix = WorldMatrix();

	engine_->SetTransform(world_matrix * Matrix3x2::Scale(Vec2(1, 1) * ratio));
	engine_->DrawBitmap(bitmap_.get());
}