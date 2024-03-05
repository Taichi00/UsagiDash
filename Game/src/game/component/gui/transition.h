#pragma once

#include "game/component/gui/control.h"
#include "math/color.h"
#include "math/vec.h"
#include "math/easing.h"
#include <game/resource/bitmap.h>
#include <memory>

class Bitmap;

class Transition : public Control
{
public:
	Transition(const Color& color, const Vec2& direction);
	~Transition();

	bool Init() override;
	void Update(const float delta_time) override;
	void Draw2D() override;

	// �t�F�[�h�C��
	void FadeIn(const float speed, const Easing::Type easing_type = Easing::LINEAR);

	// �t�F�[�h�A�E�g
	void FadeOut(const float speed, const Easing::Type easing_type = Easing::LINEAR);

	bool IsFadingIn() const { return is_fading_in_; }
	bool IsFadingOut() const { return is_fading_out_; }

private:
	Color color_;
	Vec2 direction_; // �t�F�[�h�̕���

	float time_;
	float speed_;
	Easing::Type easing_type_;

	bool is_fading_ = false; // �t�F�[�h��
	bool is_fading_in_ = false; // �t�F�[�h�C����
	bool is_fading_out_ = false; // �t�F�[�h�A�E�g��
};