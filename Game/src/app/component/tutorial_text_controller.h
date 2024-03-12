#pragma once

#include "game/component/component.h"
#include <string>
#include <unordered_map>

class Collider;
class TutorialLabelController;
class PlayerController2;

class TutorialTextController : public Component
{
public:
	enum Task
	{
		TASK_MOVE,
		TASK_JUMP,
		TASK_CAMERA,
		TASK_DASHJUMP,
		TASK_WALLJAMP,
	};

	TutorialTextController(const std::string& text, const std::string& task_key);
	~TutorialTextController() {}

	bool Init() override;
	void Update(const float delta_time) override;
	void OnCollisionEnter(Collider* collider) override;

private:
	// タスクが達成されているかどうか確認する
	bool CheckTask();

private:
	std::string text_;
	Task task_;

	// このチュートリアルがアクティブかどうか
	bool is_active_ = false;

	TutorialLabelController* label_ = nullptr;
	PlayerController2* player_ = nullptr;
	Collider* collider_ = nullptr;

	static std::unordered_map<std::string, Task> key_task_map_;
};