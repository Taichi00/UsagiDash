#pragma once

#include <string>
#include <unordered_map>

class LayerManager
{
public:
	LayerManager();
	~LayerManager() {}

	// レイヤーを追加する
	void AddLayer(const std::string& name);

	// レイヤーのインデックス番号を取得する
	int GetLayerIndex(const std::string& name) const;

	// コリジョンテーブルを設定する
	void SetCollisionTable(const std::string& name1, const std::string& name2, const bool flag);

	bool IsCollisionEnabled(const int layer1, const int layer2)
	{ 
		return collision_table_[layer1][layer2]; 
	}

private:
	// レイヤーの最大数
	static const int LAYER_MAX = 16;

	// コリジョンテーブル（衝突検出を行うかどうか）
	bool collision_table_[LAYER_MAX][LAYER_MAX] = {};

	// 名前とインデックス番号のマップ
	std::unordered_map<std::string, int> name_index_map_;

	int current_index_ = 0;
};