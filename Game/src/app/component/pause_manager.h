#pragma once

#include "game/component/component.h"
#include <vector>

class Entity;

class PauseManager : public Component
{
public:
	PauseManager();
	~PauseManager();

	static PauseManager* Get()
	{
		return instance_;
	}

	bool Init() override;
	void BeforeDraw() override;

	// ��~
	void Pause();
	// �ĊJ
	void Resume();
	// �؂�ւ�
	void TogglePause();

	// �ǉ�
	void Add(Entity* entity);
	// �폜
	void Remove(Entity* entity);

	bool IsPaused() const { return is_paused_; }

private:
	static PauseManager* instance_;

	// �|�[�Y���ɒ�~����G���e�B�e�B�̃��X�g
	std::vector<Entity*> entities_;

	// �|�[�Y�����ǂ���
	bool is_paused_ = false;
	bool is_paused_prev_ = false;
};