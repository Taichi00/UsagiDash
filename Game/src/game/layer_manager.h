#pragma once

#include <string>
#include <unordered_map>

class LayerManager
{
public:
	LayerManager();
	~LayerManager() {}

	// ���C���[��ǉ�����
	void AddLayer(const std::string& name);

	// ���C���[�̃C���f�b�N�X�ԍ����擾����
	int GetLayerIndex(const std::string& name) const;

	// �R���W�����e�[�u����ݒ肷��
	void SetCollisionTable(const std::string& name1, const std::string& name2, const bool flag);

	bool IsCollisionEnabled(const int layer1, const int layer2)
	{ 
		return collision_table_[layer1][layer2]; 
	}

private:
	// ���C���[�̍ő吔
	static const int LAYER_MAX = 16;

	// �R���W�����e�[�u���i�Փˌ��o���s�����ǂ����j
	bool collision_table_[LAYER_MAX][LAYER_MAX] = {};

	// ���O�ƃC���f�b�N�X�ԍ��̃}�b�v
	std::unordered_map<std::string, int> name_index_map_;

	int current_index_ = 0;
};