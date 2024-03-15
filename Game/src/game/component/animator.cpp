#include "game/component/animator.h"
#include "game/bone.h"
#include "math/vec.h"
#include "math/quaternion.h"
#include "math/easing.h"
#include "game/entity.h"
#include "game/component/mesh_renderer.h"
#include "game/component/gui/control.h"
#include "game/component/gui/element/element.h"

Animator::Animator()
{
	animation_map_.clear();
	current_animation_ = nullptr;
}

Animator::Animator(const std::shared_ptr<Animation>& animation) : Animator()
{
	RegisterAnimation(animation);
}

Animator::Animator(const std::vector<std::shared_ptr<Animation>>& animations) : Animator()
{
	RegisterAnimations(animations);
}

bool Animator::Init()
{
	mesh_renderer_ = GetEntity()->GetComponent<MeshRenderer>();
	control_ = GetEntity()->GetComponent<Control>();

	return true;
}

void Animator::AfterUpdate(const float delta_time)
{
	// 再生中でなければ終了
	if (!current_animation_)
		return;
	
	// アニメーションブレンドの比率を計算
	if (blend_time_ > 0)
	{
		blend_ratio_ += 1.0f / blend_time_ * delta_time;
		if (blend_ratio_ > 1) blend_ratio_ = 1;
	}
	else
	{
		blend_ratio_ = 1;
	}

	float ticks_per_second = current_animation_->TicksPerSecond();
	ticks_per_second != 0 ? ticks_per_second : 25.0f;

	float time_in_ticks = current_time_ * ticks_per_second;
	float anim_time = time_in_ticks;

	bool end_flag = false;

	if (loop_) // ループ再生の場合
	{
		anim_time = (float)fmod(anim_time, current_animation_->Duration());
	}
	else
	{
		if (anim_time >= current_animation_->Duration())
		{
			if (animation_queue_.empty())
			{
				// 再生キューが空なら終了
				end_flag = true;
			}
			else
			{
				// 再生キューが空でなければ次のアニメーションを再生する
				Play(animation_queue_.front());
				animation_queue_.pop();

				anim_time = 0;
			}
		}
	}
	
	for (const auto& channel : current_animation_->Channels())
	{
		switch (channel.type)
		{
		case Animation::TYPE_BONE:
			AnimateBone(channel.bone, anim_time);
			break;
		case Animation::TYPE_GUI:
			AnimateGUI(channel.gui, anim_time);
			break;
		}
	}
	
	current_time_ += 60.0f / 100.0f * speed_ * delta_time;

	if (end_flag)
	{
		current_animation_ = nullptr;
	}
}

void Animator::RegisterAnimation(const std::shared_ptr<Animation>& animation)
{
	animation_map_[animation->Name()] = animation;
}

void Animator::RegisterAnimations(const std::vector<std::shared_ptr<Animation>>& animations)
{
	for (int i = 0; i < animations.size(); i++)
	{
		RegisterAnimation(animations[i]);
	}
}

void Animator::Play(std::string name, float speed, bool loop, float blend_speed)
{
	if (!animation_map_.contains(name))
		return;

	auto animation = animation_map_[name];

	// 現在の状態を保持（アニメーションの遷移のため）
	if (mesh_renderer_)
		mesh_renderer_->GetBones()->SaveBuffer();

	current_animation_ = animation_map_[name];
	current_time_ = 0;
	speed_ = speed;
	loop_ = loop;
	blend_time_ = blend_speed;
	blend_ratio_ = 0;
}

void Animator::Playing(std::string name, float speed, bool loop, float blend_time)
{
	if (!animation_map_.contains(name))
		return;

	auto animation = animation_map_[name];

	// 同じアニメーションを再生中なら、プロパティだけ更新する
	if (animation == current_animation_)
	{
		speed_ = speed;
		loop_ = loop;
		blend_time_ = blend_time;
	}
	else
	{
		Play(name, speed, loop, blend_time);
	}
}

void Animator::Push(std::string name, float speed, bool loop, float blend_speed)
{
	if (animation_map_.find(name) == animation_map_.end())
	{
		return;
	}

	AnimationArgs anim{};
	anim.name = name;
	anim.speed = speed;
	anim.loop = loop;
	anim.blend_time = blend_speed;

	animation_queue_.push(anim);
}

void Animator::Stop()
{
	current_animation_ = nullptr;
}

void Animator::SetSpeed(float speed)
{
	speed_ = speed;
}

void Animator::Play(AnimationArgs anim)
{
	Play(anim.name, anim.speed, anim.loop, anim.blend_time);
}

void Animator::AnimateBone(const Animation::Channel::BoneChannel& channel, const float current_time)
{
	if (!mesh_renderer_)
		return;

	auto bone = mesh_renderer_->FindBone(channel.name);
	
	if (!bone)
		return;

	if (!channel.position_keys.empty())
	{
		auto position = CalcCurrentVec3(channel.position_keys, current_time);
		position = Vec3::Lerp(bone->PositionBuffer(), position, blend_ratio_);

		bone->SetPosition(position);
	}

	if (!channel.rotation_keys.empty())
	{
		auto rotation = CalcCurrentQuat(channel.rotation_keys, current_time);
		rotation = Quaternion::Slerp(bone->RotationBuffer(), rotation, blend_ratio_);

		bone->SetRotation(rotation);
	}

	if (!channel.scale_keys.empty())
	{
		auto scale = CalcCurrentVec3(channel.scale_keys, current_time);
		scale = Vec3::Lerp(bone->ScaleBuffer(), scale, blend_ratio_);

		bone->SetScale(scale);
	}
}

void Animator::AnimateGUI(const Animation::Channel::GUIChannel& channel, const float current_time)
{
	if (!control_)
		return;

	if (channel.name == "control")
	{
		if (!channel.position_keys.empty())
		{
			auto position = CalcCurrentVec2(channel.position_keys, current_time);
			control_->SetPosition(position);
		}

		if (!channel.rotation_keys.empty())
		{
			auto rotation = CalcCurrentFloat(channel.rotation_keys, current_time);
			control_->SetRotation(rotation);
		}

		if (!channel.color_keys.empty())
		{
			auto color = CalcCurrentColor(channel.color_keys, current_time);
			control_->SetColor(color);
		}

		if (!channel.scale_keys.empty())
		{
			auto scale = CalcCurrentVec2(channel.scale_keys, current_time);
			control_->SetScale(scale);
		}

		control_->Transform();
	}
	else
	{
		auto element = control_->GetElement(channel.name);

		if (!channel.position_keys.empty())
		{
			auto position = CalcCurrentVec2(channel.position_keys, current_time);
			element->SetPosition(position);
		}

		if (!channel.rotation_keys.empty())
		{
			auto rotation = CalcCurrentFloat(channel.rotation_keys, current_time);
			element->SetRotation(rotation);
		}

		if (!channel.color_keys.empty())
		{
			auto color = CalcCurrentColor(channel.color_keys, current_time);
			element->SetColor(color);
		}

		if (!channel.scale_keys.empty())
		{
			auto scale = CalcCurrentVec2(channel.scale_keys, current_time);
			element->SetScale(scale);
		}
	}
}

Vec2 Animator::CalcCurrentVec2(const std::vector<Animation::Vec2Key>& keys, const float current_time)
{
	int index1 = 0, index2 = 0;

	GetCurrentKeys(std::vector<Animation::Key>(keys.begin(), keys.end()), current_time, index1, index2);

	auto& key1 = keys[index1];
	auto& key2 = keys[index2];
	float t = GetEasingTime(key1, key2, current_time);

	return Vec2::Lerp(key1.value, key2.value, t);
}

Vec3 Animator::CalcCurrentVec3(const std::vector<Animation::Vec3Key>& keys, const float current_time)
{
	int index1 = 0, index2 = 0;

	GetCurrentKeys(std::vector<Animation::Key>(keys.begin(), keys.end()), current_time, index1, index2);

	auto& key1 = keys[index1];
	auto& key2 = keys[index2];
	float t = GetEasingTime(key1, key2, current_time);

	return Vec3::Lerp(key1.value, key2.value, t);
}

Quaternion Animator::CalcCurrentQuat(const std::vector<Animation::QuatKey>& keys, const float current_time)
{
	int index1 = 0, index2 = 0;

	GetCurrentKeys(std::vector<Animation::Key>(keys.begin(), keys.end()), current_time, index1, index2);

	auto& key1 = keys[index1];
	auto& key2 = keys[index2];
	float t = GetEasingTime(key1, key2, current_time);

	return Quaternion::Slerp(key1.value, key2.value, t);
}

Color Animator::CalcCurrentColor(const std::vector<Animation::ColorKey>& keys, const float current_time)
{
	int index1 = 0, index2 = 0;

	GetCurrentKeys(std::vector<Animation::Key>(keys.begin(), keys.end()), current_time, index1, index2);

	auto& key1 = keys[index1];
	auto& key2 = keys[index2];
	float t = GetEasingTime(key1, key2, current_time);

	return Color::Lerp(key1.value, key2.value, t);
}

float Animator::CalcCurrentFloat(const std::vector<Animation::FloatKey>& keys, const float current_time)
{
	int index1 = 0, index2 = 0;

	GetCurrentKeys(std::vector<Animation::Key>(keys.begin(), keys.end()), current_time, index1, index2);

	auto& key1 = keys[index1];
	auto& key2 = keys[index2];
	float t = GetEasingTime(key1, key2, current_time);

	return key1.value * (1 - t) + key2.value * t;
}

void Animator::GetCurrentKeys(const std::vector<Animation::Key>& keys, const float current_time, int& index1, int& index2)
{
	index1 = 0;
	index2 = (int)keys.size() - 1;

	for (auto i = 0u; i < keys.size(); i++)
	{
		auto& key = keys[i];
		if (current_time < key.time)
		{
			index2 = i;
			break;
		}
		index1 = i;
	}
}

float Animator::GetEasingTime(const Animation::Key& key1, const Animation::Key& key2, const float current_time)
{
	float duration = key2.time - key1.time;
	return duration > 0 ? Easing::GetFunction(key2.easing)((current_time - key1.time) / duration) : 0;
}
