#pragma once

#include "game/resource/resource.h"
#include "math/vec.h"
#include "math/aabb.h"
#include <memory>
#include <string>
#include <vector>
#include <utility>

struct CollisionVertex
{
	Vec3 position;	// à íuç¿ïW
	Vec3 normal;	// ñ@ê¸
};

//struct CollisionFace
//{
//	uint32_t indices[3];
//	Vec3 normal;
//	std::vector<std::pair<uint32_t, uint32_t>> edges;
//	AABB aabb;
//};

struct CollisionPolygon
{
	CollisionVertex vertices[3];
	Vec3 normal;
	AABB aabb;
};

struct CollisionMesh
{
	//std::vector<CollisionVertex> vertices;
	//std::vector<CollisionFace> faces;
	std::vector<CollisionPolygon> polygons;
	AABB aabb;
};

class CollisionModel : public Resource
{
public:
	CollisionModel() = default;
	~CollisionModel() = default;

	static std::unique_ptr<CollisionModel> Load(const std::wstring& key);
	bool Release() override;

public:
	std::vector<CollisionMesh> meshes;
	AABB aabb;
};