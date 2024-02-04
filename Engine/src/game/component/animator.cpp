#include "game/component/animator.h"
#include "game/animation.h"
#include "game/bone.h"
#include "math/vec.h"
#include "math/quaternion.h"
#include "math/easing.h"
#include "game/entity.h"
#include "game/component/mesh_renderer.h"

Animator::Animator()
{
	animations_.clear();
	animation_map_.clear();
	current_animation_ = nullptr;
	current_time_ = 0;
	speed_ = 1.0f;
}

bool Animator::Init()
{
	mesh_renderer_ = GetEntity()->GetComponent<MeshRenderer>();

	RegisterAnimations(mesh_renderer_->GetModel().animations);

	return true;
}

void Animator::Update()
{
	if (!current_animation_)
		return;
	
	transition_ratio_ += 0.2f;
	if (transition_ratio_ > 1) transition_ratio_ = 1;

	float ticksPerSecond = current_animation_->GetTicksPerSecond();
	ticksPerSecond != 0 ? ticksPerSecond : 25.0f;
	float timeInTicks = current_time_ * ticksPerSecond;
	float animTime = timeInTicks;

	if (loop_)
	{
		animTime = fmod(animTime, current_animation_->GetDuration());
	}
	else
	{
		if (animTime >= current_animation_->GetDuration())
		{
			if (animation_queue_.empty())
			{
				current_animation_ = nullptr;
				return;
			}

			Play(animation_queue_.front());
			animation_queue_.pop();

			animTime = 0;
		}
	}
	
	for (auto channel : current_animation_->GetChannels())
	{
		std::string name = channel.name;
		auto bone = mesh_renderer_->FindBone(name);

		auto currentPos = CalcCurrentPosition(&channel.position_keys, animTime, bone);
		auto currentRot = CalcCurrentRotation(&channel.rotation_keys, animTime, bone);
		auto currentScale = CalcCurrentScale(&channel.scaling_keys, animTime, bone);

		bone->SetPosition(currentPos);
		bone->SetRotation(currentRot);
		bone->SetScale(currentScale);
	}
	
	current_time_ += 1.0 / 100.0 * speed_;
}

void Animator::RegisterAnimation(const std::shared_ptr<Animation>& animation)
{
	animations_.push_back(animation);
	animation_map_[animation->GetName()] = animation;
}

void Animator::RegisterAnimations(const std::vector<std::shared_ptr<Animation>>& animations)
{
	for (int i = 0; i < animations.size(); i++)
	{
		RegisterAnimation(animations[i]);
	}
}

void Animator::Play(std::string name, float speed, bool loop)
{
	if (animation_map_.find(name) == animation_map_.end())
	{
		current_animation_ = nullptr;
		return;
	}
	
	// 現在の状態を保持（アニメーションの遷移のため）
	mesh_renderer_->GetBones()->SaveBuffer();

	current_animation_ = animation_map_[name];
	current_time_ = 0;
	speed_ = speed;
	loop_ = loop;
	transition_ratio_ = 0;
}

void Animator::Push(std::string name, float speed, bool loop)
{
	if (animation_map_.find(name) == animation_map_.end())
	{
		return;
	}

	AnimationArgs anim{};
	anim.name = name;
	anim.speed = speed;
	anim.loop = loop;

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
	Play(anim.name, anim.speed, anim.loop);
}

Vec3 Animator::CalcCurrentPosition(std::vector<VectorKey>* keys, float currentTime, Bone* bone)
{
	int keyIndex[2] = { 0, 0 };
	float time[2] = { 0, 0 };

	for (int i = 0; i < keys->size(); i++)
	{
		auto t = (*keys)[i].time;
		if (currentTime < t)
		{
			keyIndex[1] = i;
			time[1] = t;
			break;
		}
		keyIndex[0] = i;
		time[0] = t;
	}

	Vec3 pos[2] = { (*keys)[keyIndex[0]].value, (*keys)[keyIndex[1]].value };

	float dur = time[1] - time[0];
	float rate = dur > 0 ? Easing::Linear((currentTime - time[0]) / dur) : 0;
	Vec3 currentPos = pos[0] * (1 - rate) + pos[1] * rate;
	Vec3 prevPos = bone->GetPositionBuffer();

	currentPos = currentPos * transition_ratio_ + prevPos * (1 - transition_ratio_);

	return currentPos;
}

Quaternion Animator::CalcCurrentRotation(std::vector<QuatKey>* keys, float currentTime, Bone* bone)
{
	int keyIndex[2] = { 0, 0 };
	float time[2] = { 0, 0 };

	for (int i = 0; i < keys->size(); i++)
	{
		auto t = (*keys)[i].time;
		if (currentTime < t)
		{
			keyIndex[1] = i;
			time[1] = t;
			break;
		}
		keyIndex[0] = i;
		time[0] = t;
	}

	Quaternion rot[2] = { (*keys)[keyIndex[0]].value, (*keys)[keyIndex[1]].value };

	float dur = time[1] - time[0];
	float rate = dur > 0 ? Easing::Linear((currentTime - time[0]) / dur) : 0;
	Quaternion currentRot = Quaternion::Slerp(rot[0], rot[1], rate);
	Quaternion prevRot = bone->GetRotationBuffer();

	currentRot = Quaternion::Slerp(prevRot, currentRot, transition_ratio_);
	
	return currentRot;
}

Vec3 Animator::CalcCurrentScale(std::vector<VectorKey>* keys, float currentTime, Bone* bone)
{
	int keyIndex[2] = { 0, 0 };
	float time[2] = { 0, 0 };

	for (int i = 0; i < keys->size(); i++)
	{
		auto t = (*keys)[i].time;
		if (currentTime < t)
		{
			keyIndex[1] = i;
			time[1] = t;
			break;
		}
		keyIndex[0] = i;
		time[0] = t;
	}

	Vec3 scale[2] = { (*keys)[keyIndex[0]].value, (*keys)[keyIndex[1]].value };

	float dur = time[1] - time[0];
	float rate = dur > 0 ? Easing::Linear((currentTime - time[0]) / dur) : 0;
	Vec3 currentScale = scale[0] * (1 - rate) + scale[1] * rate;
	Vec3 prevScale = bone->GetScaleBuffer();

	currentScale = currentScale * transition_ratio_ + prevScale * (1 - transition_ratio_);

	return currentScale;
}

