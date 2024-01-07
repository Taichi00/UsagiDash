#pragma once

#include <memory>

class Model;

class SphereMesh
{
public:
	static std::unique_ptr<Model> Load(float radius, float r, float g, float b);
};