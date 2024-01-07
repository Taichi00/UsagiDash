#pragma once

#include <memory>

class Model;

class CapsuleMesh
{
public:
	static std::unique_ptr<Model> Load(float radius, float height, float r, float g, float b);
};