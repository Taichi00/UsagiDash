#include "bitmap.h"
#include "game/game.h"
#include "engine/engine.h"
#include "engine/engine2d.h"
#include <iostream>

std::unique_ptr<Bitmap> Bitmap::Load(const std::wstring& path)
{
	auto bitmap = std::make_unique<Bitmap>();

	try
	{
		auto engine2d = Game::Get()->GetEngine()->GetEngine2D();
		engine2d->LoadBitmapFromFile(path, bitmap->data_.GetAddressOf());
	}
	catch (const char* e)
	{
		std::cout << "Failed load bitmap file: " << e << std::endl;
		return nullptr;
	}

	auto size = bitmap->data_->GetPixelSize();
	bitmap->size_ = Vec2(size.width, size.height);

	return std::move(bitmap);
}
