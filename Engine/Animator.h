#pragma once

#include <vector>
#include <string>
#include "BoneList.h"
#include "Vec.h"
#include "Quaternion.h"
#include "Component.h"

class Animation;
struct VectorKey;
struct QuatKey;
class MeshRenderer;

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
	void Play(std::string name, float speed = 1.0f);
	void Stop();

private:
	Vec3 CalcCurrentPosition(std::vector<VectorKey>* keys, float currentTime);
	Quaternion CalcCurrentRotation(std::vector<QuatKey>* keys, float currentTime);
	Vec3 CalcCurrentScale(std::vector<VectorKey>* keys, float currentTime);

private:
	std::vector<Animation*> m_animations;
	std::map<std::string, Animation*> m_animationMap;

	Animation* m_currentAnimation;
	float m_currentTime;
	float m_speed;

	MeshRenderer* m_pMeshRenderer;
};