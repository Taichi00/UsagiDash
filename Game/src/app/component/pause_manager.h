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

	// 停止
	void Pause();
	// 再開
	void Resume();
	// 切り替え
	void TogglePause();

	// 追加
	void Add(Entity* entity);
	// 削除
	void Remove(Entity* entity);

	bool IsPaused() const { return is_paused_; }

private:
	static PauseManager* instance_;

	// ポーズ中に停止するエンティティのリスト
	std::vector<Entity*> entities_;

	// ポーズ中かどうか
	bool is_paused_ = false;
	bool is_paused_prev_ = false;
};