#include "Animator.h"
#include "Animation.h"
#include "Bone.h";
#include "Vec.h";
#include "Quaternion.h"
#include "Easing.h"
#include "Entity.h"
#include "MeshRenderer.h"

Animator::Animator()
{
	m_animations.clear();
	m_animationMap.clear();
	m_currentAnimation = nullptr;
	m_currentTime = 0;
	m_speed = 1.0f;
}

Animator::Animator(AnimatorProperty prop)
{
	Animator();

	RegisterAnimations(prop.Animations);
}

bool Animator::Init()
{
	m_pMeshRenderer = GetEntity()->GetComponent<MeshRenderer>();

	return true;
}

void Animator::Update()
{
	if (!m_currentAnimation)
		return;
	
	m_transitionRatio += 0.2;
	if (m_transitionRatio > 1) m_transitionRatio = 1;

	float ticksPerSecond = m_currentAnimation->GetTicksPerSecond();
	ticksPerSecond != 0 ? ticksPerSecond : 25.0f;
	float timeInTicks = m_currentTime * ticksPerSecond;
	float animTime = timeInTicks;

	if (m_loop)
	{
		animTime = fmod(animTime, m_currentAnimation->GetDuration());
	}
	else
	{
		if (animTime >= m_currentAnimation->GetDuration())
		{
			if (m_animationQueue.empty())
			{
				m_currentAnimation = nullptr;
				return;
			}

			Play(m_animationQueue.front());
			m_animationQueue.pop();

			animTime = 0;
		}
	}
	
	for (auto channel : m_currentAnimation->GetChannels())
	{
		std::string name = channel.Name;
		auto bone = m_pMeshRenderer->FindBone(name);

		auto currentPos = CalcCurrentPosition(&channel.PositionKeys, animTime, bone);
		auto currentRot = CalcCurrentRotation(&channel.RotationKeys, animTime, bone);
		auto currentScale = CalcCurrentScale(&channel.ScalingKeys, animTime, bone);

		bone->SetPosition(currentPos);
		bone->SetRotation(currentRot);
		bone->SetScale(currentScale);
	}
	
	m_currentTime += 1.0 / 100.0 * m_speed;
}

void Animator::RegisterAnimation(Animation* animation)
{
	m_animations.push_back(animation);
	m_animationMap[animation->GetName()] = animation;
}

void Animator::RegisterAnimations(std::vector<Animation*> animations)
{
	for (int i = 0; i < animations.size(); i++)
	{
		RegisterAnimation(animations[i]);
	}
}

void Animator::Play(std::string name, float speed, bool loop)
{
	if (m_animationMap.find(name) == m_animationMap.end())
	{
		m_currentAnimation = nullptr;
		return;
	}
	
	// 現在の状態を保持（アニメーションの遷移のため）
	m_pMeshRenderer->GetBones()->SaveBuffer();

	m_currentAnimation = m_animationMap[name];
	m_currentTime = 0;
	m_speed = speed;
	m_loop = loop;
	m_transitionRatio = 0;
}

void Animator::Push(std::string name, float speed, bool loop)
{
	if (m_animationMap.find(name) == m_animationMap.end())
	{
		return;
	}

	AnimationArgs anim{};
	anim.name = name;
	anim.speed = speed;
	anim.loop = loop;

	m_animationQueue.push(anim);
}

void Animator::Stop()
{
	m_currentAnimation = nullptr;
}

void Animator::SetSpeed(float speed)
{
	m_speed = speed;
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
		auto t = (*keys)[i].Time;
		if (currentTime < t)
		{
			keyIndex[1] = i;
			time[1] = t;
			break;
		}
		keyIndex[0] = i;
		time[0] = t;
	}

	Vec3 pos[2] = { (*keys)[keyIndex[0]].Value, (*keys)[keyIndex[1]].Value };

	float dur = time[1] - time[0];
	float rate = dur > 0 ? Easing::Linear((currentTime - time[0]) / dur) : 0;
	Vec3 currentPos = pos[0] * (1 - rate) + pos[1] * rate;
	Vec3 prevPos = bone->GetPositionBuffer();

	currentPos = currentPos * m_transitionRatio + prevPos * (1 - m_transitionRatio);

	return currentPos;
}

Quaternion Animator::CalcCurrentRotation(std::vector<QuatKey>* keys, float currentTime, Bone* bone)
{
	int keyIndex[2] = { 0, 0 };
	float time[2] = { 0, 0 };

	for (int i = 0; i < keys->size(); i++)
	{
		auto t = (*keys)[i].Time;
		if (currentTime < t)
		{
			keyIndex[1] = i;
			time[1] = t;
			break;
		}
		keyIndex[0] = i;
		time[0] = t;
	}

	Quaternion rot[2] = { (*keys)[keyIndex[0]].Value, (*keys)[keyIndex[1]].Value };

	float dur = time[1] - time[0];
	float rate = dur > 0 ? Easing::Linear((currentTime - time[0]) / dur) : 0;
	Quaternion currentRot = Quaternion::slerp(rot[0], rot[1], rate);
	Quaternion prevRot = bone->GetRotationBuffer();

	currentRot = Quaternion::slerp(prevRot, currentRot, m_transitionRatio);
	
	return currentRot;
}

Vec3 Animator::CalcCurrentScale(std::vector<VectorKey>* keys, float currentTime, Bone* bone)
{
	int keyIndex[2] = { 0, 0 };
	float time[2] = { 0, 0 };

	for (int i = 0; i < keys->size(); i++)
	{
		auto t = (*keys)[i].Time;
		if (currentTime < t)
		{
			keyIndex[1] = i;
			time[1] = t;
			break;
		}
		keyIndex[0] = i;
		time[0] = t;
	}

	Vec3 scale[2] = { (*keys)[keyIndex[0]].Value, (*keys)[keyIndex[1]].Value };

	float dur = time[1] - time[0];
	float rate = dur > 0 ? Easing::Linear((currentTime - time[0]) / dur) : 0;
	Vec3 currentScale = scale[0] * (1 - rate) + scale[1] * rate;
	Vec3 prevScale = bone->GetScaleBuffer();

	currentScale = currentScale * m_transitionRatio + prevScale * (1 - m_transitionRatio);

	return currentScale;
}

