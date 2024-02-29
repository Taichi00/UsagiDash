#include "game/bone_list.h"

//BoneList BoneList::Copy(const BoneList& boneList)
//{
//    BoneList newBoneList;
//
//    std::vector<int> parentIndices;
//    std::vector<std::vector<int>> childrenIndices;
//
//    for (auto bone : boneList)
//    {
//        auto parent = bone->Parent();
//        auto children = bone->Children();
//
//        if (parent)
//        {
//            parentIndices.push_back(boneList.Index(parent));
//        }
//        else
//        {
//            parentIndices.push_back(-1);
//        }
//
//        if (!children.empty())
//        {
//            std::vector<int> childIndices;
//            for (auto child : children)
//            {
//                childIndices.push_back(boneList.Index(child));
//            }
//            childrenIndices.push_back(childIndices);
//        }
//        else
//        {
//            childrenIndices.push_back(std::vector<int>());
//        }
//
//        newBoneList.Append(new Bone(*bone));
//    }
//
//    for (int i = 0; i < newBoneList.Size(); i++)
//    {
//        if (parentIndices[i] != -1)
//        {
//            newBoneList[i]->SetParent(newBoneList[parentIndices[i]]);
//        }
//        
//        for (auto index : childrenIndices[i])
//        {
//            newBoneList[i]->AddChild(newBoneList[index]);
//        }
//    }
//
//    return newBoneList;
//}

void BoneList::Append(Bone* bone)
{
    bones_.push_back(std::shared_ptr<Bone>(bone));
    name_index_map_[bone->Name()] = (unsigned int)(bones_.size() - 1);

    if (!bone->Parent())
        root_bone_ = bone;
}

Bone* BoneList::Find(const std::string& name) const
{
    Bone* bone = nullptr;

    if (name_index_map_.contains(name))
    {
        bone = bones_[name_index_map_.at(name)].get();
    }

    return bone;
}

unsigned int BoneList::Index(Bone* bone) const
{
    return name_index_map_.at(bone->Name());
}

void BoneList::SaveBuffer()
{
    for (auto& bone : bones_)
    {
        bone->SaveBuffer();
    }
}

BoneList BoneList::Clone()
{
    BoneList bonelist;

    for (auto& bone : bones_)
    {
        bonelist.Append(new Bone(*bone));
    }

    // 親ポインタを書き換える
    for (int i = 0; i < bonelist.Size(); i++)
    {
        auto bone = bonelist[i];
        auto parent = bones_[i]->Parent();
        if (parent)
        {
            auto new_parent = bonelist[name_index_map_.at(parent->Name())];
            bone->SetParent(new_parent);
        }
        else
        {
            bone->SetParent(nullptr);
        }
    }

    return bonelist;
}

//void BoneList::CreateNameMap(Bone* bone)
//{
//    bone_name_map_[bone->Name()] = bone;
//
//    for (auto child : bone->Children())
//    {
//        CreateNameMap(child);
//    }
//}

//Bone& BoneList::Cloning(const Bone* bone)
//{
//    Bone new_bone = bone;
//
//    for (auto child : bone.Children())
//    {
//        new_bone.AddChild(Cloning(*child));
//    }
//
//    return new_bone;
//}
