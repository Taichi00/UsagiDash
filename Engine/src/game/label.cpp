#include "game/label.h"
#include "math/color.h"
#include "engine/engine.h"
#include "engine/engine2d.h"
#include "game/string_methods.h"
#include "game/component/transform.h"
#include "game/game.h"

Label::Label()
{
	text_ = "";
	font_ = "MS ƒSƒVƒbƒN";
	size_ = 24;
	color_ = Color::White();

	width_ = 500;
	height_ = size_;
}

Label::Label(const std::string& text) : Label()
{
	this->text_ = text;
}

Label::Label(const std::string& text, const std::string& font) : Label()
{
	this->text_ = text;
	this->font_ = font;
}

Label::Label(const std::string& text, const float size) : Label()
{
	this->text_ = text;
	this->size_ = size;
}

Label::Label(const std::string& text, const std::string& font, const float size) : Label()
{
	this->text_ = text;
	this->font_ = font;
	this->size_ = size;
}

Label::Label(const std::string& text, const std::string& font, const float size, const Color& color) : Label()
{
	this->text_ = text;
	this->font_ = font;
	this->size_ = size;
	this->color_ = color;
}

Label::~Label()
{
}

bool Label::Init()
{
	auto engine2D = Game::Get()->GetEngine()->GetEngine2D();
	
	color_key_ = color_.GetString();
	font_key_ = font_ + std::to_string(size_);

	engine2D->RegisterSolidColorBrush(color_key_, { color_.r, color_.g, color_.b });
	engine2D->RegisterTextFormat(font_key_, StringMethods::GetWideString(font_), size_);

	return true;
}

void Label::Draw2D()
{
	auto engine2D = Game::Get()->GetEngine()->GetEngine2D();

	auto x = transform->position.x;
	auto y = transform->position.y + (height_ - size_) / 2;

	engine2D->DrawText(font_key_, color_key_, StringMethods::GetWideString(text_), { x, y, x + width_, y + height_ });
}
