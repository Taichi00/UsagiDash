#pragma once

#include "game/component/component.h"
#include "math/quaternion.h"
#include "math/vec.h"
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
	std::vector<std::shared_ptr<Animation>> animations_;
	std::map<std::string, std::shared_ptr<Animation>> animation_map_;

	std::shared_ptr<Animation> current_animation_;
	std::queue<AnimationArgs> animation_queue_;
	float current_time_;
	float speed_;
	bool loop_;
	float transition_ratio_;

	MeshRenderer* mesh_renderer_;
};