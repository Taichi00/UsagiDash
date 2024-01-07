#include "Model.h"
#include "AssimpLoader.h"

Model::Model()
{
}

Model::~Model()
{
}

std::unique_ptr<Model> Model::Load(const std::string& key)
{
	return AssimpLoader::Load(key);
}

bool Model::Release()
{
	return true;
}
