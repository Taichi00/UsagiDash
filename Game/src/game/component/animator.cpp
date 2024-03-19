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
}

Animator::Animator(std::shared_ptr<Animation> animation) : Animator()
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
	for (int i = 0; i < ANIMATOR_LAYER_MAX; i++)
	{
		UpdateAnimation(i, delta_time);
	}
}

void Animator::RegisterAnimation(std::shared_ptr<Animation> animation)
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

void Animator::Play(std::shared_ptr<Animation> animation, float speed, bool loop, float blend_time, unsigned int layer)
{
	if (!animation || layer >= ANIMATOR_LAYER_MAX)
		return;

	// 現在の状態を保持（アニメーションの遷移のため）
	if (mesh_renderer_)
		mesh_renderer_->GetBones()->SaveBuffer();
	
	AnimationData data = {};
	data.animation = animation;
	data.speed = speed;
	data.loop = loop;
	data.blend_time = blend_time;
	data.start_position = transform->position;
	data.start_rotation = transform->rotation;
	data.start_scale = transform->scale;

	current_animations_[layer] = data;

	ProcessChannels(layer, 0);
}

void Animator::Play(const std::string& name, float speed, bool loop, float blend_time, unsigned int layer)
{
	if (!animation_map_.contains(name))
		return;

	auto& animation = animation_map_[name];
	
	Play(animation, speed, loop, blend_time, layer);
}

void Animator::Playing(std::shared_ptr<Animation> animation, float speed, bool loop, float blend_time, unsigned int layer)
{
	if (layer >= ANIMATOR_LAYER_MAX)
		return;

	if (animation == current_animations_[layer].animation)
	{
		// 同じアニメーションを再生中なら、プロパティだけ更新する
		current_animations_[layer].speed = speed;
		current_animations_[layer].loop = loop;
		current_animations_[layer].blend_time = blend_time;
	}
	else
	{
		Play(animation, speed, loop, blend_time, layer);
	}
}

void Animator::Playing(const std::string& name, float speed, bool loop, float blend_time, unsigned int layer)
{
	if (!animation_map_.contains(name))
		return;

	auto& animation = animation_map_[name];
	
	Playing(animation, speed, loop, blend_time, layer);
}

void Animator::Push(std::shared_ptr<Animation> animation, float speed, bool loop, float blend_time, unsigned int layer)
{
	if (!animation || layer >= ANIMATOR_LAYER_MAX)
		return;

	AnimationData data = {};
	data.animation = animation;
	data.speed = speed;
	data.loop = loop;
	data.blend_time = blend_time;

	animation_queues_[layer].push(data);
}

void Animator::Push(const std::string& name, float speed, bool loop, float blend_time, unsigned int layer)
{
	if (!animation_map_.contains(name))
		return;

	auto& animation = animation_map_[name];

	Push(animation, speed, loop, blend_time, layer);
}

void Animator::ClearQueue()
{
	for (auto& animation_queue : animation_queues_)
	{
		// 空のキューと swap することでキューを空にする
		std::queue<AnimationData>().swap(animation_queue);
	}
}

void Animator::Stop()
{
	for (int i = 0; i < ANIMATOR_LAYER_MAX; i++)
	{
		current_animations_[i] = {};
	}
}

std::shared_ptr<Animation> Animator::GetAnimation(const std::string& name) const
{
	if (!animation_map_.contains(name))
		return nullptr;

	return animation_map_.at(name);
}

void Animator::UpdateAnimation(unsigned int layer, const float delta_time)
{
	auto& data = current_animations_[layer];

	// 再生中でなければ終了
	if (!data.animation)
		return;

	// アニメーションブレンドの比率を計算
	if (data.blend_time > 0)
	{
		data.blend_ratio += 1.0f / data.blend_time * delta_time;
		if (data.blend_ratio > 1) data.blend_ratio = 1;
	}
	else
	{
		data.blend_ratio = 1;
	}

	float ticks_per_second = data.animation->TicksPerSecond();
	float anim_time = data.current_time * ticks_per_second;

	bool end_flag = false;

	if (data.loop) // ループ再生の場合
	{
		anim_time = std::fmod(anim_time, data.animation->Duration());
	}
	else
	{
		if (anim_time >= data.animation->Duration())
		{
			if (animation_queues_[layer].empty())
			{
				// 再生キューが空なら終了
				end_flag = true;
			}
			else
			{
				// 再生キューが空でなければ次のアニメーションを再生する
				auto& next = animation_queues_[layer].front();
				Play(next.animation, next.speed, next.loop, next.blend_time, layer);
				animation_queues_[layer].pop();

				anim_time = 0;
			}
		}
	}

	ProcessChannels(layer, anim_time);

	data.previous_time = data.current_time;
	data.current_time += 60.0f / 100.0f * data.speed * delta_time;

	if (end_flag)
	{
		data.animation = nullptr;
	}
}

void Animator::ProcessChannels(unsigned int layer, const float current_time)
{
	auto& data = current_animations_[layer];

	for (const auto& channel : data.animation->Channels())
	{
		switch (channel.type)
		{
		case Animation::TYPE_BONE:
			AnimateBone(channel.bone, current_time, data.blend_ratio);
			break;
		case Animation::TYPE_TRANSFORM:
			AnimateTransform(channel.transform, current_time, data.start_position, data.start_rotation, data.start_scale);
			break;
		case Animation::TYPE_GUI:
			AnimateGUI(channel.gui, current_time);
			break;
		}
	}
}

void Animator::AnimateBone(const Animation::BoneChannel& channel, const float current_time, const float blend_ratio)
{
	if (!mesh_renderer_)
		return;

	auto bone = mesh_renderer_->FindBone(channel.name);
	
	if (!bone)
		return;

	if (!channel.position_keys.empty())
	{
		auto position = Animation::GetCurrentVec3(channel.position_keys, current_time);
		position = Vec3::Lerp(bone->PositionBuffer(), position, blend_ratio);

		bone->SetPosition(position);
	}

	if (!channel.rotation_keys.empty())
	{
		auto rotation = Animation::GetCurrentQuat(channel.rotation_keys, current_time);
		rotation = Quaternion::Slerp(bone->RotationBuffer(), rotation, blend_ratio);

		bone->SetRotation(rotation);
	}

	if (!channel.scale_keys.empty())
	{
		auto scale = Animation::GetCurrentVec3(channel.scale_keys, current_time);
		scale = Vec3::Lerp(bone->ScaleBuffer(), scale, blend_ratio);

		bone->SetScale(scale);
	}
}

void Animator::AnimateTransform(
	const Animation::TransformChannel& channel,
	const float current_time,
	const Vec3& start_position,
	const Quaternion& start_rotation,
	const Vec3& start_scale
)
{
	if (!channel.position_keys.empty())
	{
		auto position = Animation::GetCurrentVec3(channel.position_keys, current_time);
		transform->position = start_position + position;
	}

	if (!channel.rotation_keys.empty())
	{
		auto rotation = Animation::GetCurrentQuat(channel.rotation_keys, current_time);
		transform->rotation = start_rotation * rotation;
	}

	if (!channel.scale_keys.empty())
	{
		auto scale = Animation::GetCurrentVec3(channel.scale_keys, current_time);
		transform->scale = start_scale * scale;
	}
}

void Animator::AnimateGUI(const Animation::GUIChannel& channel, const float current_time)
{
	if (!control_)
		return;

	if (channel.name == "control")
	{
		if (!channel.position_keys.empty())
		{
			auto position = Animation::GetCurrentVec2(channel.position_keys, current_time);
			control_->SetPosition(position);
		}

		if (!channel.rotation_keys.empty())
		{
			auto rotation = Animation::GetCurrentFloat(channel.rotation_keys, current_time);
			control_->SetRotation(rotation);
		}

		if (!channel.color_keys.empty())
		{
			auto color = Animation::GetCurrentColor(channel.color_keys, current_time);
			control_->SetColor(color);
		}

		if (!channel.scale_keys.empty())
		{
			auto scale = Animation::GetCurrentVec2(channel.scale_keys, current_time);
			control_->SetScale(scale);
		}

		control_->Transform();
	}
	else
	{
		auto element = control_->GetElement(channel.name);

		if (!channel.position_keys.empty())
		{
			auto position = Animation::GetCurrentVec2(channel.position_keys, current_time);
			element->SetPosition(position);
		}

		if (!channel.rotation_keys.empty())
		{
			auto rotation = Animation::GetCurrentFloat(channel.rotation_keys, current_time);
			element->SetRotation(rotation);
		}

		if (!channel.color_keys.empty())
		{
			auto color = Animation::GetCurrentColor(channel.color_keys, current_time);
			element->SetColor(color);
		}

		if (!channel.scale_keys.empty())
		{
			auto scale = Animation::GetCurrentVec2(channel.scale_keys, current_time);
			element->SetScale(scale);
		}
	}
}
