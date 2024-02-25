#pragma once

#include "game/resource/resource.h"
#include "engine/comptr.h"
#include "math/vec.h"
#include <d2d1_3.h>
#include <string>
#include <memory>

class Bitmap : public Resource
{
public:
	Bitmap() {};
	~Bitmap() {};

	static std::unique_ptr<Bitmap> Load(const std::wstring& path);

	void SetData(ComPtr<ID2D1Bitmap> data) { data_ = data; }
	void SetSize(const Vec2& size) { size_ = size; }

	ComPtr<ID2D1Bitmap> Data() const { return data_; }
	Vec2 Size() const { return size_; }

private:
	ComPtr<ID2D1Bitmap> data_;
	Vec2 size_;
};