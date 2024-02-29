#pragma once

#include "game/bone.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

class BoneList
{
public:
	BoneList() {}

	typedef std::vector<std::shared_ptr<Bone>>::iterator iterator;
	typedef std::vector<std::shared_ptr<Bone>>::const_iterator const_iterator;

	iterator begin() { return bones_.begin(); }
	const_iterator begin() const { return bones_.begin(); }
	iterator end() { return bones_.end(); }
	const_iterator end() const { return bones_.end(); }

	Bone* operator[](int i) { return bones_[i].get(); }

	// �{�[����ǉ�
	void Append(Bone* bone);

	// ���O����{�[��������
	Bone* Find(const std::string& name) const;

	// �{�[���̃C���f�b�N�X�ԍ����擾
	unsigned int Index(Bone* bone) const;

	// ���݂̏�Ԃ��o�b�t�@�ɕۑ�
	void SaveBuffer();

	// BoneList �𕡐�
	BoneList Clone();

	size_t Size() const { return bones_.size(); }
	Bone* RootBone() const { return root_bone_; }

private:
	std::vector<std::shared_ptr<Bone>> bones_;
	std::unordered_map<std::string, unsigned int> name_index_map_;
	Bone* root_bone_ = nullptr;
};