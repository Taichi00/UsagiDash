#include "game/resource/model.h"
#include "game/assimp_loader.h"

Model::Model()
{
}

Model::~Model()
{
}

std::unique_ptr<Model> Model::Load(const std::wstring& key)
{
	auto model = AssimpLoader::Load(key);

	return std::move(model);
}

bool Model::Release()
{
	return true;
}
