#pragma once

#include <vector>
#include <map>
#include <string>

class Bone;

class BoneList
{
public:
	BoneList();

	static BoneList Copy(const BoneList& boneList);

	typedef std::vector<Bone*>::iterator iterator;
	typedef std::vector<Bone*>::const_iterator const_iterator;

	iterator begin() { return bones_.begin(); }
	const_iterator begin() const { return bones_.begin(); }
	iterator end() { return bones_.end(); }
	const_iterator end() const { return bones_.end(); }

	Bone* operator[](int i) { return bones_[i]; }

	void Append(Bone* bone);
	Bone* Find(const std::string& name);
	int Index(Bone* bone) const;
	int Size();
	void Clear();

	void SaveBuffer();

	const std::vector<Bone*>& RootBones();

private:
	std::vector<Bone*> bones_;
	std::map<std::string, int> bone_map_;
	std::vector<Bone*> root_bones_;
};