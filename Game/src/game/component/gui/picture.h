#pragma once

#include "game/component/gui/control.h"
#include "math/vec.h"
#include <string>
#include <memory>

class Bitmap;

class Picture : public Control
{
private:
	Picture();

public:
	Picture(const std::shared_ptr<Bitmap> bitmap);

	bool Init() override;
	void Draw2D() override;

private:
	std::shared_ptr<Bitmap> bitmap_;
};