#include "collision_model.h"
#include "game/assimp_loader.h"

std::unique_ptr<CollisionModel> CollisionModel::Load(const std::wstring& key)
{
    return AssimpLoader::LoadCollision(key);
}

bool CollisionModel::Release()
{
    return true;
}
