#include "layer_manager.h"

LayerManager::LayerManager()
{
	// デフォルトレイヤーの追加
	AddLayer("default");
}

void LayerManager::AddLayer(const std::string& name)
{
	if (current_index_ >= LAYER_MAX)
		return;

	if (name_index_map_.contains(name))
		return;

	auto index = current_index_++;
	name_index_map_[name] = index;

	// コリジョンテーブルを初期化
	for (int i = 0; i <= index; i++)
	{
		collision_table_[index][i] = false;
		collision_table_[i][index] = false;
	}
}

int LayerManager::GetLayerIndex(const std::string& name) const
{
	// 存在しなければ default レイヤーとする
	if (!name_index_map_.contains(name))
		return 0;

	return name_index_map_.at(name);
}

void LayerManager::SetCollisionTable(const std::string& name1, const std::string& name2, const bool flag)
{
	if (!name_index_map_.contains(name1) || !name_index_map_.contains(name2))
		return;

	auto index1 = name_index_map_[name1];
	auto index2 = name_index_map_[name2];
	collision_table_[index1][index2] = flag;
	collision_table_[index2][index1] = flag;
}
