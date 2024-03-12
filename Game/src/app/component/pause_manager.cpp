#include "pause_manager.h"
#include "game/entity.h"
#include "game/scene.h"

PauseManager* PauseManager::instance_ = nullptr;

PauseManager::PauseManager()
{
}

PauseManager::~PauseManager()
{
}

bool PauseManager::Init()
{
	if (instance_ && instance_ != this)
	{
		GetEntity()->Destroy();
	}
	else
	{
		instance_ = this;
		GetScene()->DontDestroyOnLoad(GetEntity());
	}

	return true;
}

void PauseManager::BeforeDraw()
{
	if (is_paused_ != is_paused_prev_)
	{
		// エンティティに通知する
		if (is_paused_)
		{
			for (auto entity : entities_)
			{
				entity->DisableUpdate();
			}
		}
		else
		{
			for (auto entity : entities_)
			{
				entity->EnableUpdate();
			}
		}
	}

	is_paused_prev_ = is_paused_;
}

void PauseManager::Pause()
{
	is_paused_ = true;
}

void PauseManager::Resume()
{
	is_paused_ = false;
}

void PauseManager::TogglePause()
{
	is_paused_ ? Resume() : Pause();
}

void PauseManager::Add(Entity* entity)
{
	entities_.push_back(entity);
}

void PauseManager::Remove(Entity* entity)
{
	auto it = std::find(entities_.begin(), entities_.end(), entity);

	if (it != entities_.end())
	{
		entities_.erase(it);
	}
}

