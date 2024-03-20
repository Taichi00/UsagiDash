#pragma once

#include <vector>

class Control;
class ButtonBase;

class GUIManager
{
public:
	GUIManager();
	~GUIManager();

	void Update();

	void Add(Control* control);
	void Remove(Control* control);

	// ボタンを追加
	void AddButton(ButtonBase* button);

	// ボタンを削除
	void RemoveButton(ButtonBase* button);

	// ボタンを選択する
	void PickButton(ButtonBase* button);

	// z index でソートしたリストを取得する
	std::vector<Control*> GetDrawList();

private:
	void UpdateTarget();
	void CheckButtonPress();

private:
	// control のリスト
	std::vector<Control*> controls_;
	// ボタンのリスト
	std::vector<ButtonBase*> buttons_;

	// 現在選択されているボタン
	ButtonBase* current_picked_button_ = nullptr;
	ButtonBase* prev_picked_button_ = nullptr;
};