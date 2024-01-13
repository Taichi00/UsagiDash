#include "game/bone_list.h"
#include "game/bone.h"

BoneList::BoneList()
{
    bones_.clear();
    bone_map_.clear();
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
    int index = bones_.size();

    bones_.push_back(bone);
    bone_map_[bone->GetName()] = index;
}

Bone* BoneList::Find(const std::string& name)
{
    return bones_[bone_map_[name]];
}

int BoneList::Index(Bone* bone) const
{
    return bone_map_.at(bone->GetName());
}

int BoneList::Size()
{
    return bones_.size();
}

void BoneList::Clear()
{
    bones_.clear();
    bone_map_.clear();
}

void BoneList::SaveBuffer()
{
    for (auto bone : bones_)
    {
        bone->SaveBuffer();
    }
}
