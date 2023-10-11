#include "BoneList.h"
#include "Bone.h"

BoneList::BoneList()
{
    m_bones.clear();
    m_boneMap.clear();
}

BoneList BoneList::Copy(const BoneList& boneList)
{
    BoneList newBoneList;

    std::vector<int> parentIndices;
    std::vector<std::vector<int>> childrenIndices;

    for (auto bone : boneList)
    {
        auto parent = bone->GetParent();
        auto children = bone->GetChildren();

        if (parent)
        {
            parentIndices.push_back(boneList.Index(parent));
        }
        else
        {
            parentIndices.push_back(-1);
        }

        if (!children.empty())
        {
            std::vector<int> childIndices;
            for (auto child : children)
            {
                childIndices.push_back(boneList.Index(child));
            }
            childrenIndices.push_back(childIndices);
        }
        else
        {
            childrenIndices.push_back(std::vector<int>());
        }

        newBoneList.Append(new Bone(*bone));
    }

    for (int i = 0; i < newBoneList.Size(); i++)
    {
        if (parentIndices[i] != -1)
        {
            newBoneList[i]->SetParent(newBoneList[parentIndices[i]]);
        }
        
        for (auto index : childrenIndices[i])
        {
            newBoneList[i]->AddChild(newBoneList[index]);
        }
    }

    return newBoneList;
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

int BoneList::Index(Bone* bone) const
{
    return m_boneMap.at(bone->GetName());
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
