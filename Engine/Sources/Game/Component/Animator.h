#pragma once

#include "Component.h"
#include "Quaternion.h"
#include "Vec.h"
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <vector>

class Animation;
struct VectorKey;
struct QuatKey;
class Bone;
class MeshRenderer;

struct AnimationArgs
{
	std::string name;
	float speed;
	bool loop;
};

class Animator : public Component
{
public:
	Animator();

	bool Init();
	void Update();

	void RegisterAnimation(const std::shared_ptr<Animation>& animation);
	void RegisterAnimations(const std::vector<std::shared_ptr<Animation>>& animations);
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
	std::vector<std::shared_ptr<Animation>> m_animations;
	std::map<std::string, std::shared_ptr<Animation>> m_animationMap;

	std::shared_ptr<Animation> m_currentAnimation;
	std::queue<AnimationArgs> m_animationQueue;
	float m_currentTime;
	float m_speed;
	bool m_loop;
	float m_transitionRatio;

	MeshRenderer* m_pMeshRenderer;
};