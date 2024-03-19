#pragma once

#include <vector>

class ButtonBase;

class GUIManager
{
public:
	GUIManager();
	~GUIManager();

	void Update();

	// ボタンを追加
	void AddButton(ButtonBase* button);

	// ボタンを削除
	void RemoveButton(ButtonBase* button);

	// ボタンを選択する
	void PickButton(ButtonBase* button);

private:
	void UpdateTarget();
	void CheckButtonPress();

private:
	// ボタンのリスト
	std::vector<ButtonBase*> buttons_;

	// 現在選択されているボタン
	ButtonBase* current_picked_button_ = nullptr;
	ButtonBase* prev_picked_button_ = nullptr;
};