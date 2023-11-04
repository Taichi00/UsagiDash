#pragma once

struct Model;

class CapsuleMesh
{
public:
	static Model Load(float radius, float height, float r, float g, float b);
};