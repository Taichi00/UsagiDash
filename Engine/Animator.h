#pragma once

#include <vector>
#include <string>
#include "BoneList.h"
#include "Vec.h"
#include "Quaternion.h"
#include "Component.h"
#include <queue>

class Animation;
struct VectorKey;
struct QuatKey;
class MeshRenderer;

struct AnimationArgs
{
	std::string name;
	float speed;
	bool loop;
};

struct AnimatorProperty
{
	std::vector<Animation*> Animations;
};

class Animator : public Component
{
public:
	Animator();
	Animator(AnimatorProperty prop);

	bool Init();
	void Update();

	void RegisterAnimation(Animation* animation);
	void RegisterAnimations(std::vector<Animation*> animations);
	void Play(std::string name, float speed = 1.0f, bool loop = true);
	void Push(std::string name, float speed = 1.0f, bool loop = true);
	void Stop();
	void SetSpeed(float speed);

private:
	void Play(AnimationArgs anim);

	Vec3 CalcCurrentPosition(std::vector<VectorKey>* keys, float currentTime, Bone* bone);
	Quaternion CalcCurrentRotation(std::vector<QuatKey>* keys, float currentTime, Bone* bone);
	Vec3 CalcCurrentScale(std::vector<VectorKey>* keys, float currentTime, Bone* bone);

private:
	std::vector<Animation*> m_animations;
	std::map<std::string, Animation*> m_animationMap;

	Animation* m_currentAnimation;
	std::queue<AnimationArgs> m_animationQueue;
	float m_currentTime;
	float m_speed;
	bool m_loop;
	float m_transitionRatio;

	MeshRenderer* m_pMeshRenderer;
};