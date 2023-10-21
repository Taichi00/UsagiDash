#pragma once

struct Model;

class SphereMesh
{
public:
	static Model Load(int sliceNum, int stackNum);
};