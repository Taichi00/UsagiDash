#include "BoneList.h"
#include "Bone.h"

BoneList::BoneList()
{
    m_bones.clear();
    m_boneMap.clear();
}

void BoneList::Append(Bone* bone)
{
    int index = m_bones.size();

    m_bones.push_back(bone);
    m_boneMap[bone->GetName()] = index;
}

Bone* BoneList::Find(std::string name)
{
    return m_bones[m_boneMap[name]];
}

int BoneList::Index(Bone* bone)
{
    return m_boneMap[bone->GetName()];
}

int BoneList::Size()
{
    return m_bones.size();
}

void BoneList::Clear()
{
    m_bones.clear();
    m_boneMap.clear();
}
