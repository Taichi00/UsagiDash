#include "game/entity.h"
#include "game/scene.h"
#include "game/collision_manager.h"
#include "game/component/collider/mesh_collider.h"
#include "game/component/collider/sphere_collider.h"
#include "game/component/collider/capsule_collider.h"
#include "game/component/collider/ray.h"
#include "game/component/collider/polygon_collider.h"
#include "game/component/rigidbody.h"
#include "math/vec.h"
#include <algorithm>

MeshCollider::MeshCollider(std::shared_ptr<CollisionModel> model)
{
	this->model = model;
}

MeshCollider::~MeshCollider()
{
	
}

bool MeshCollider::Init()
{
	rigidbody_ = GetEntity()->GetComponent<Rigidbody>();

	collision_manager_ = Game::Get()->GetCollisionManager();

	// PolygonColliderを生成
	for (auto& mesh : model->meshes)
	{
		for (auto& polygon : mesh.polygons)
		{
			auto pc = std::make_shared<PolygonCollider>(polygon, this);
			pc->RegisterEntity(GetEntity());
			pc->Init();
			polygons_.push_back(pc);
		}
	}

	return true;
}

//bool MeshCollider::Intersects(SphereCollider* sphere)
//{
//	bool res = false;
//
//	std::vector<Vec3> collidedPoints;
//
//	auto center = sphere->GetPosition() + sphere->offset;
//	auto radius = sphere->radius;
//
//	for (int i = 0; i < model->meshes.size(); i++)
//	{
//		auto mesh = &(model->meshes[i]);
//		
//		for (int j = 0; j < mesh->faces.size(); j++)
//		{
//			auto face = &(mesh->faces[j]);
//
//			auto face_aabb = face->aabb;
//			face_aabb.Scale(scale);
//			face_aabb.Translate(position_ + offset);
//
//			// AABBが衝突していなければ無視
//			if (!face_aabb.Intersects(sphere->GetAABB()))
//			{
//				continue;
//			}
//
//			int idx0 = face->indices[0];
//			int idx1 = face->indices[1];
//			int idx2 = face->indices[2];
//
//			uint32_t indices[3] = { idx0, idx1, idx2 };
//			Vec3 normal;
//			float distance;
//
//			// 衝突
//			if (SphereIntersectsMesh(center, radius, *mesh, *face, normal, distance, collidedPoints))
//			{
//				AddHit({ sphere, -normal, distance });
//				sphere->AddHit({ this, normal, distance });
//
//				res = true;
//			}
//
//		}
//	}
//
//	return res;
//}
//
//bool MeshCollider::Intersects(CapsuleCollider* capsule)
//{
//	bool res = false;
//
//	std::vector<Vec3> collidedPoints;
//
//	float radius = capsule->radius;
//	float height = capsule->height;
//
//	// TODO: 回転の対応
//	Vec3 base = capsule->GetPosition() + capsule->offset - Vec3(0, 1, 0) * (height / 2 + radius);
//	Vec3 tip = capsule->GetPosition() + capsule->offset + Vec3(0, 1, 0) * (height / 2 + radius);
//	Vec3 capsuleNormal = (tip - base).Normalized();
//	Vec3 lineEndOffset = capsuleNormal * radius;
//	Vec3 A = base + lineEndOffset;
//	Vec3 B = tip - lineEndOffset;
//	
//	for (int i = 0; i < model->meshes.size(); i++)
//	{
//		auto mesh = &(model->meshes[i]);
//
//		for (int j = 0; j < mesh->faces.size(); j++)
//		{
//			auto face = &(mesh->faces[j]);
//
//			auto face_aabb = face->aabb;
//			face_aabb.Scale(scale);
//			face_aabb.Translate(position_ + offset);
//
//			// AABBが衝突していなければ無視
//			if (!face_aabb.Intersects(capsule->GetAABB()))
//			{
//				continue;
//			}
//
//			int idx0 = face->indices[0];
//			int idx1 = face->indices[1];
//			int idx2 = face->indices[2];
//
//			auto p0 = Vec3(mesh->vertices[idx0].position);
//			auto p1 = Vec3(mesh->vertices[idx1].position);
//			auto p2 = Vec3(mesh->vertices[idx2].position);
//			p0 = Vec3::Scale(p0, scale) + position_ + offset;
//			p1 = Vec3::Scale(p1, scale) + position_ + offset;
//			p2 = Vec3::Scale(p2, scale) + position_ + offset;
//
//			Vec3 N = face->normal;	// 平面の法線
//
//			Vec3 referencePoint;	// カプセルに最も近いポリゴン上の点
//
//			// 平面の法線とカプセルが平行である場合、平面上の1つの点をreferencePointとする
//			if (Vec3::Dot(N, capsuleNormal) == 0)
//			{
//				referencePoint = p0;
//			}
//			else
//			{
//				// カプセルのトレース線と平面の交点を見つける
//				float t = Vec3::Dot(N, (p0 - base)) / Vec3::Dot(N, capsuleNormal);
//				Vec3 linePlaneIntersection = base + capsuleNormal * t;
//
//				// ポリゴンに最も近い点を見つける
//				auto c0 = Vec3::Cross(linePlaneIntersection - p0, p1 - p0);
//				auto c1 = Vec3::Cross(linePlaneIntersection - p1, p2 - p1);
//				auto c2 = Vec3::Cross(linePlaneIntersection - p2, p0 - p2);
//				bool inside = Vec3::Dot(c0, N) <= 0 && Vec3::Dot(c1, N) <= 0 && Vec3::Dot(c2, N) <= 0;
//
//				if (inside)
//				{
//					referencePoint = linePlaneIntersection;
//				}
//				else
//				{
//					// Edge 1
//					auto point1 = ClosestPointOnLineSegment(p0, p1, linePlaneIntersection);
//					auto v1 = linePlaneIntersection - point1;
//					float distsq = Vec3::Dot(v1, v1);
//					float bestDist = distsq;
//					referencePoint = point1;
//
//					// Edge 2
//					auto point2 = ClosestPointOnLineSegment(p1, p2, linePlaneIntersection);
//					auto v2 = linePlaneIntersection - point2;
//					distsq = Vec3::Dot(v2, v2);
//					if (distsq < bestDist)
//					{
//						referencePoint = point2;
//						bestDist = distsq;
//					}
//
//					// Edge 3
//					auto point3 = ClosestPointOnLineSegment(p2, p0, linePlaneIntersection);
//					auto v3 = linePlaneIntersection - point3;
//					distsq = Vec3::Dot(v3, v3);
//					if (distsq < bestDist)
//					{
//						referencePoint = point3;
//						bestDist = distsq;
//					}
//				}
//			}
//
//			// トレース先の球を決定
//			Vec3 center = ClosestPointOnLineSegment(A, B, referencePoint);
//
//			uint32_t indices[3] = { idx0, idx1, idx2 };
//			Vec3 normal;
//			float distance;
//
//			// 衝突
//			if (SphereIntersectsMesh(center, radius, *mesh, *face, normal, distance, collidedPoints))
//			{
//				distance -= 0.0;
//
//				AddHit({ capsule, -normal, distance });
//				capsule->AddHit({ this, normal, distance });
//
//				res = true;
//			}
//
//		}
//	}
//	//if (collidedPoints.size() > 0)
//	//{
//	//	//printf("test");
//	//}
//	return res;
//}
//
//bool MeshCollider::Intersects(Ray* ray)
//{
//	bool res = false;
//
//	auto rayOrigin = ray->origin;
//	auto rayDirection = ray->direction;
//	auto rayDistance = ray->distance;
//
//	for (int i = 0; i < model->meshes.size(); i++)
//	{
//		auto mesh = &(model->meshes[i]);
//
//		for (int j = 0; j < mesh->faces.size(); j++)
//		{
//			auto face = &(mesh->faces[j]);
//
//			int idx0 = face->indices[0];
//			int idx1 = face->indices[1];
//			int idx2 = face->indices[2];
//
//			auto p0 = Vec3(mesh->vertices[idx0].position);
//			auto p1 = Vec3(mesh->vertices[idx1].position);
//			auto p2 = Vec3(mesh->vertices[idx2].position);
//			p0 = Vec3::Scale(p0, scale) + position_ + offset;
//			p1 = Vec3::Scale(p1, scale) + position_ + offset;
//			p2 = Vec3::Scale(p2, scale) + position_ + offset;
//
//			Vec3 N = face->normal;	// 平面の法線
//
//			// Rayの点座標
//			auto rp0 = rayOrigin;
//			auto rp1 = rayOrigin + rayDirection * rayDistance;
//
//			// Rayと平面の貫通
//			if (Vec3::Dot(rp0 - p0, N) * Vec3::Dot(rp1 - p0, N) > 0)
//				continue;
//
//			// 貫通点の座標を確定
//			auto d0 = abs(Vec3::Dot(N, rp0 - p0));
//			auto d1 = abs(Vec3::Dot(N, rp1 - p0));
//			auto dist = rayDistance * d0 / (d0 + d1);
//			auto point = rayOrigin + rayDirection * dist;
//			 
//			// 貫通点がポリゴン内部に含まれているか
//			auto in0 = Vec3::Cross(point - p0, p1 - p0);
//			auto in1 = Vec3::Cross(point - p1, p2 - p1);
//			auto in2 = Vec3::Cross(point - p2, p0 - p2);
//			bool inside = Vec3::Dot(in0, N) <= 0 && Vec3::Dot(in1, N) <= 0 && Vec3::Dot(in2, N) <= 0;
//
//			if (inside)
//			{
//				ray->AddHit({ this, N, rayDistance - dist });
//
//				res = true;
//			}
//		}
//	}
//
//	return res;
//}
//
//bool MeshCollider::SphereIntersectsMesh(
//	const Vec3& center, const float radius,
//	const CollisionMesh& mesh, const CollisionFace& face,
//	Vec3& normal, float& distance, std::vector<Vec3>& collidedPoints)
//{
//	int idx0 = face.indices[0];
//	int idx1 = face.indices[1];
//	int idx2 = face.indices[2];
//
//	Vec3 p[3];
//	p[0] = Vec3(mesh.vertices[idx0].position);
//	p[1] = Vec3(mesh.vertices[idx1].position);
//	p[2] = Vec3(mesh.vertices[idx2].position);
//	p[0] = Vec3::Scale(p[0], scale) + position_ + offset;
//	p[1] = Vec3::Scale(p[1], scale) + position_ + offset;
//	p[2] = Vec3::Scale(p[2], scale) + position_ + offset;
//
//	Vec3 N = face.normal;	// 平面の法線
//	float dist = Vec3::Dot(center - p[0], N);	// 球と平面の距離
//
//	if (dist < 0)	// 裏面なら
//		return false;
//
//	if (dist < -radius || dist > radius)
//		return false;
//
//	auto point0 = center - N * dist;	// 球の中心を平面に投影
//
//	// point0がポリゴン内部にあるか判定
//	auto c0 = Vec3::Cross(point0 - p[0], p[1] - p[0]);
//	auto c1 = Vec3::Cross(point0 - p[1], p[2] - p[1]);
//	auto c2 = Vec3::Cross(point0 - p[2], p[0] - p[2]);
//	bool inside = Vec3::Dot(c0, N) <= 0 && Vec3::Dot(c1, N) <= 0 && Vec3::Dot(c2, N) <= 0;
//
//	// 各エッジと交差しているか判定
//	float radiussq = radius * radius;
//
//	std::vector<Vec3> points;
//	bool intersects = false;
//	for (const auto& edge : face.edges)
//	{
//		auto point = ClosestPointOnLineSegment(p[edge.first], p[edge.second], center);
//
//		// すでに衝突しているエッジは無視
//		/*if (std::find(collidedPoints.begin(), collidedPoints.end(), point) != collidedPoints.end())
//		{
//			continue;
//		}*/
//
//		auto v = center - point;
//		float distsq = Vec3::Dot(v, v);
//		intersects |= distsq < radiussq;
//
//		points.push_back(point);
//	}
//	// Edge 1
//	//auto point1 = ClosestPointOnLineSegment(p0, p1, center);
//	//auto v1 = center - point1;
//	//float distsq1 = Vec3::dot(v1, v1);
//	//bool intersects = distsq1 < radiussq;
//
//	//// Edge 2
//	//auto point2 = ClosestPointOnLineSegment(p1, p2, center);
//	//auto v2 = center - point2;
//	//float distsq2 = Vec3::dot(v2, v2);
//	//intersects |= distsq2 < radiussq;
//
//	//// Edge 3
//	//auto point3 = ClosestPointOnLineSegment(p2, p0, center);
//	//auto v3 = center - point3;
//	//float distsq3 = Vec3::dot(v3, v3);
//	//intersects |= distsq3 < radiussq;
//
//	// 衝突
//	if (inside || intersects)
//	{
//		Vec3 bestPoint = point0;
//		Vec3 intersectionVec;
//
//		if (inside)
//		{
//			intersectionVec = center - point0;
//		}
//		else
//		{
//			float bestDistsq = radiussq;
//			for (auto& point : points)
//			{
//				auto d = center - point;
//				float distsq = Vec3::Dot(d, d);
//				if (distsq < bestDistsq)
//				{
//					bestDistsq = distsq;
//					bestPoint = point;
//					intersectionVec = d;
//				}
//			}
//
//			//collidedPoints.push_back(bestPoint);
//
//			/*auto d = center - point1;
//			float bestDistsq = Vec3::dot(d, d);
//			bestPoint = point1;
//			intersectionVec = d;
//
//			d = center - point2;
//			float distsq = Vec3::dot(d, d);
//			if (distsq < bestDistsq)
//			{
//				bestDistsq = distsq;
//				bestPoint = point2;
//				intersectionVec = d;
//			}
//
//			d = center - point3;
//			distsq = Vec3::dot(d, d);
//			if (distsq < bestDistsq)
//			{
//				bestDistsq = distsq;
//				bestPoint = point3;
//				intersectionVec = d;
//			}*/
//		}
//
//		float len = intersectionVec.Length();
//		normal = intersectionVec.Normalized();
//		distance = radius - len;
//
//		return true;
//	}
//
//	return false;
//}

void MeshCollider::PrepareAABB()
{
	aabb_ = model->aabb;
}

