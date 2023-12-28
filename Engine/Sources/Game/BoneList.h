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

	iterator begin() { return m_bones.begin(); }
	const_iterator begin() const { return m_bones.begin(); }
	iterator end() { return m_bones.end(); }
	const_iterator end() const { return m_bones.end(); }

	Bone* operator[](int i) { return m_bones[i]; }

	void Append(Bone* bone);
	Bone* Find(const std::string& name);
	int Index(Bone* bone) const;
	int Size();
	void Clear();

	void SaveBuffer();

private:
	std::vector<Bone*> m_bones;
	std::map<std::string, int> m_boneMap;
};