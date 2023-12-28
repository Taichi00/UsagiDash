#include "MeshCollider.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h"
#include "Ray.h"
#include "Vec.h"
#include <algorithm>

MeshCollider::MeshCollider(MeshColliderProperty prop)
{
	model = prop.Model;
}

MeshCollider::~MeshCollider()
{
}

bool MeshCollider::Init()
{
	Collider::Init();
	return true;
}

bool MeshCollider::Intersects(SphereCollider* sphere)
{
	bool res = false;

	std::vector<Vec3> collidedPoints;

	auto center = sphere->GetPosition() + sphere->offset;
	auto radius = sphere->radius;

	for (int i = 0; i < model.Meshes.size(); i++)
	{
		auto mesh = &(model.Meshes[i]);
		
		for (int j = 0; j < mesh->Faces.size(); j++)
		{
			auto face = &(mesh->Faces[j]);

			int idx0 = face->Indices[0];
			int idx1 = face->Indices[1];
			int idx2 = face->Indices[2];

			uint32_t indices[3] = { idx0, idx1, idx2 };
			Vec3 normal;
			float distance;

			// 衝突
			if (SphereIntersectsMesh(center, radius, *mesh, *face, normal, distance, collidedPoints))
			{
				hitColliders.push_back(sphere);
				hitNormals.push_back(-normal);
				hitDistances.push_back(distance);

				sphere->hitColliders.push_back(this);
				sphere->hitNormals.push_back(normal);
				sphere->hitDistances.push_back(distance);

				res = true;
			}

		}
	}

	return res;
}

bool MeshCollider::Intersects(CapsuleCollider* capsule)
{
	bool res = false;

	std::vector<Vec3> collidedPoints;

	float radius = capsule->radius;
	float height = capsule->height;

	// TODO: 回転の対応
	Vec3 base = capsule->GetPosition() + capsule->offset - Vec3(0, 1, 0) * (height / 2 + radius);
	Vec3 tip = capsule->GetPosition() + capsule->offset + Vec3(0, 1, 0) * (height / 2 + radius);
	Vec3 capsuleNormal = (tip - base).normalized();
	Vec3 lineEndOffset = capsuleNormal * radius;
	Vec3 A = base + lineEndOffset;
	Vec3 B = tip - lineEndOffset;
	
	for (int i = 0; i < model.Meshes.size(); i++)
	{
		auto mesh = &(model.Meshes[i]);

		for (int j = 0; j < mesh->Faces.size(); j++)
		{
			auto face = &(mesh->Faces[j]);

			int idx0 = face->Indices[0];
			int idx1 = face->Indices[1];
			int idx2 = face->Indices[2];

			auto p0 = Vec3(mesh->Vertices[idx0].Position);
			auto p1 = Vec3(mesh->Vertices[idx1].Position);
			auto p2 = Vec3(mesh->Vertices[idx2].Position);
			p0 = Vec3::Scale(p0, m_scale) + m_position + offset;
			p1 = Vec3::Scale(p1, m_scale) + m_position + offset;
			p2 = Vec3::Scale(p2, m_scale) + m_position + offset;

			Vec3 N = face->Normal;	// 平面の法線

			Vec3 referencePoint;	// カプセルに最も近いポリゴン上の点

			// 平面の法線とカプセルが平行である場合、平面上の1つの点をreferencePointとする
			if (Vec3::dot(N, capsuleNormal) == 0)
			{
				referencePoint = p0;
			}
			else
			{
				// カプセルのトレース線と平面の交点を見つける
				float t = Vec3::dot(N, (p0 - base)) / Vec3::dot(N, capsuleNormal);
				Vec3 linePlaneIntersection = base + capsuleNormal * t;

				// ポリゴンに最も近い点を見つける
				auto c0 = Vec3::cross(linePlaneIntersection - p0, p1 - p0);
				auto c1 = Vec3::cross(linePlaneIntersection - p1, p2 - p1);
				auto c2 = Vec3::cross(linePlaneIntersection - p2, p0 - p2);
				bool inside = Vec3::dot(c0, N) <= 0 && Vec3::dot(c1, N) <= 0 && Vec3::dot(c2, N) <= 0;

				if (inside)
				{
					referencePoint = linePlaneIntersection;
				}
				else
				{
					// Edge 1
					auto point1 = ClosestPointOnLineSegment(p0, p1, linePlaneIntersection);
					auto v1 = linePlaneIntersection - point1;
					float distsq = Vec3::dot(v1, v1);
					float bestDist = distsq;
					referencePoint = point1;

					// Edge 2
					auto point2 = ClosestPointOnLineSegment(p1, p2, linePlaneIntersection);
					auto v2 = linePlaneIntersection - point2;
					distsq = Vec3::dot(v2, v2);
					if (distsq < bestDist)
					{
						referencePoint = point2;
						bestDist = distsq;
					}

					// Edge 3
					auto point3 = ClosestPointOnLineSegment(p2, p0, linePlaneIntersection);
					auto v3 = linePlaneIntersection - point3;
					distsq = Vec3::dot(v3, v3);
					if (distsq < bestDist)
					{
						referencePoint = point3;
						bestDist = distsq;
					}
				}
			}

			// トレース先の球を決定
			Vec3 center = ClosestPointOnLineSegment(A, B, referencePoint);

			uint32_t indices[3] = { idx0, idx1, idx2 };
			Vec3 normal;
			float distance;

			// 衝突
			if (SphereIntersectsMesh(center, radius, *mesh, *face, normal, distance, collidedPoints))
			{
				distance -= 0.0;

				hitColliders.push_back(capsule);
				hitNormals.push_back(-normal);
				hitDistances.push_back(distance);

				capsule->hitColliders.push_back(this);
				capsule->hitNormals.push_back(normal);
				capsule->hitDistances.push_back(distance);

				res = true;
			}

		}
	}
	//if (collidedPoints.size() > 0)
	//{
	//	//printf("test");
	//}
	return res;
}

bool MeshCollider::Intersects(Ray* ray)
{
	bool res = false;

	auto rayOrigin = ray->origin;
	auto rayDirection = ray->direction;
	auto rayDistance = ray->distance;

	for (int i = 0; i < model.Meshes.size(); i++)
	{
		auto mesh = &(model.Meshes[i]);

		for (int j = 0; j < mesh->Faces.size(); j++)
		{
			auto face = &(mesh->Faces[j]);

			int idx0 = face->Indices[0];
			int idx1 = face->Indices[1];
			int idx2 = face->Indices[2];

			auto p0 = Vec3(mesh->Vertices[idx0].Position);
			auto p1 = Vec3(mesh->Vertices[idx1].Position);
			auto p2 = Vec3(mesh->Vertices[idx2].Position);
			p0 = Vec3::Scale(p0, m_scale) + m_position + offset;
			p1 = Vec3::Scale(p1, m_scale) + m_position + offset;
			p2 = Vec3::Scale(p2, m_scale) + m_position + offset;

			Vec3 N = face->Normal;	// 平面の法線

			// Rayの点座標
			auto rp0 = rayOrigin;
			auto rp1 = rayOrigin + rayDirection * rayDistance;

			// Rayと平面の貫通
			if (Vec3::dot(rp0 - p0, N) * Vec3::dot(rp1 - p0, N) > 0)
				continue;

			// 貫通点の座標を確定
			auto d0 = abs(Vec3::dot(N, rp0 - p0));
			auto d1 = abs(Vec3::dot(N, rp1 - p0));
			auto dist = rayDistance * d0 / (d0 + d1);
			auto point = rayOrigin + rayDirection * dist;
			 
			// 貫通点がポリゴン内部に含まれているか
			auto in0 = Vec3::cross(point - p0, p1 - p0);
			auto in1 = Vec3::cross(point - p1, p2 - p1);
			auto in2 = Vec3::cross(point - p2, p0 - p2);
			bool inside = Vec3::dot(in0, N) <= 0 && Vec3::dot(in1, N) <= 0 && Vec3::dot(in2, N) <= 0;

			if (inside)
			{
				ray->hitColliders.push_back(this);
				ray->hitNormals.push_back(N);
				ray->hitDistances.push_back(dist);

				res = true;
			}
		}
	}

	return res;
}

bool MeshCollider::SphereIntersectsMesh(
	const Vec3& center, const float radius,
	const CollisionMesh& mesh, const CollisionFace& face,
	Vec3& normal, float& distance, std::vector<Vec3>& collidedPoints)
{
	int idx0 = face.Indices[0];
	int idx1 = face.Indices[1];
	int idx2 = face.Indices[2];

	Vec3 p[3];
	p[0] = Vec3(mesh.Vertices[idx0].Position);
	p[1] = Vec3(mesh.Vertices[idx1].Position);
	p[2] = Vec3(mesh.Vertices[idx2].Position);
	p[0] = Vec3::Scale(p[0], m_scale) + m_position + offset;
	p[1] = Vec3::Scale(p[1], m_scale) + m_position + offset;
	p[2] = Vec3::Scale(p[2], m_scale) + m_position + offset;

	Vec3 N = face.Normal;	// 平面の法線
	float dist = Vec3::dot(center - p[0], N);	// 球と平面の距離

	if (dist < 0)	// 裏面なら
		return false;

	if (dist < -radius || dist > radius)
		return false;

	auto point0 = center - N * dist;	// 球の中心を平面に投影

	// point0がポリゴン内部にあるか判定
	auto c0 = Vec3::cross(point0 - p[0], p[1] - p[0]);
	auto c1 = Vec3::cross(point0 - p[1], p[2] - p[1]);
	auto c2 = Vec3::cross(point0 - p[2], p[0] - p[2]);
	bool inside = Vec3::dot(c0, N) <= 0 && Vec3::dot(c1, N) <= 0 && Vec3::dot(c2, N) <= 0;

	// 各エッジと交差しているか判定
	float radiussq = radius * radius;

	std::vector<Vec3> points;
	bool intersects = false;
	for (const auto& edge : face.Edges)
	{
		auto point = ClosestPointOnLineSegment(p[edge.first], p[edge.second], center);

		// すでに衝突しているエッジは無視
		/*if (std::find(collidedPoints.begin(), collidedPoints.end(), point) != collidedPoints.end())
		{
			continue;
		}*/

		auto v = center - point;
		float distsq = Vec3::dot(v, v);
		intersects |= distsq < radiussq;

		points.push_back(point);
	}
	// Edge 1
	//auto point1 = ClosestPointOnLineSegment(p0, p1, center);
	//auto v1 = center - point1;
	//float distsq1 = Vec3::dot(v1, v1);
	//bool intersects = distsq1 < radiussq;

	//// Edge 2
	//auto point2 = ClosestPointOnLineSegment(p1, p2, center);
	//auto v2 = center - point2;
	//float distsq2 = Vec3::dot(v2, v2);
	//intersects |= distsq2 < radiussq;

	//// Edge 3
	//auto point3 = ClosestPointOnLineSegment(p2, p0, center);
	//auto v3 = center - point3;
	//float distsq3 = Vec3::dot(v3, v3);
	//intersects |= distsq3 < radiussq;

	// 衝突
	if (inside || intersects)
	{
		Vec3 bestPoint = point0;
		Vec3 intersectionVec;

		if (inside)
		{
			intersectionVec = center - point0;
		}
		else
		{
			float bestDistsq = radiussq;
			for (auto& point : points)
			{
				auto d = center - point;
				float distsq = Vec3::dot(d, d);
				if (distsq < bestDistsq)
				{
					bestDistsq = distsq;
					bestPoint = point;
					intersectionVec = d;
				}
			}

			//collidedPoints.push_back(bestPoint);

			/*auto d = center - point1;
			float bestDistsq = Vec3::dot(d, d);
			bestPoint = point1;
			intersectionVec = d;

			d = center - point2;
			float distsq = Vec3::dot(d, d);
			if (distsq < bestDistsq)
			{
				bestDistsq = distsq;
				bestPoint = point2;
				intersectionVec = d;
			}

			d = center - point3;
			distsq = Vec3::dot(d, d);
			if (distsq < bestDistsq)
			{
				bestDistsq = distsq;
				bestPoint = point3;
				intersectionVec = d;
			}*/
		}

		float len = intersectionVec.length();
		normal = intersectionVec.normalized();
		distance = radius - len;

		return true;
	}

	return false;
}

