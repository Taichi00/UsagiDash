#include "MeshCollider.h"
#include "SphereCollider.h"
#include "Vec.h"

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

	auto center = sphere->GetPosition() + sphere->offset;
	auto radius = sphere->radius;

	for (int i = 0; i < model.Meshes.size(); i++)
	{
		auto mesh = &(model.Meshes[i]);
		
		for (int j = 0; j < mesh->Indices.size(); j += 3)
		{
			int j0 = j, j1 = j + 1, j2 = j + 2;

			int idx0 = mesh->Indices[j0];
			int idx1 = mesh->Indices[j1];
			int idx2 = mesh->Indices[j2];

			auto p0 = Vec3(mesh->Vertices[idx0].Position);
			auto p1 = Vec3(mesh->Vertices[idx1].Position);
			auto p2 = Vec3(mesh->Vertices[idx2].Position);
			p0 = Vec3::Scale(p0, m_scale) + m_position + offset;
			p1 = Vec3::Scale(p1, m_scale) + m_position + offset;
			p2 = Vec3::Scale(p2, m_scale) + m_position + offset;

			auto n0 = Vec3(mesh->Vertices[idx0].Normal);
			auto n1 = Vec3(mesh->Vertices[idx1].Normal);
			auto n2 = Vec3(mesh->Vertices[idx2].Normal);

			Vec3 N = (n0 + n1 + n2).normalized();	// 平面の法線
			float dist = Vec3::dot(center - p0, N);	// 球と平面の距離

			if (dist < 0)	// 裏面なら
				continue;

			if (dist < -radius || dist > radius)
				continue;
			
			auto point0 = center - N * dist;	// 球の中心を平面に投影
			
			// point0がポリゴン内部にあるか判定
			auto c0 = Vec3::cross(point0 - p0, p1 - p0);
			auto c1 = Vec3::cross(point0 - p1, p2 - p1);
			auto c2 = Vec3::cross(point0 - p2, p0 - p2);
			bool inside = Vec3::dot(c0, N) <= 0 && Vec3::dot(c1, N) <= 0 && Vec3::dot(c2, N) <= 0;
			
			// 各エッジと交差しているか判定
			float radiussq = radius * radius;

			// Edge 1
			auto point1 = ClosestPointOnLineSegment(p0, p1, center);
			auto v1 = center - point1;
			float distsq1 = Vec3::dot(v1, v1);
			bool intersects = distsq1 < radiussq;

			// Edge 2
			auto point2 = ClosestPointOnLineSegment(p1, p2, center);
			auto v2 = center - point2;
			float distsq2 = Vec3::dot(v2, v2);
			intersects |= distsq2 < radiussq;

			// Edge 3
			auto point3 = ClosestPointOnLineSegment(p2, p0, center);
			auto v3 = center - point3;
			float distsq3 = Vec3::dot(v3, v3);
			intersects |= distsq3 < radiussq;

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
					auto d = center - point1;
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
					}
				}

				float len = intersectionVec.length();
				Vec3 normal = intersectionVec / len;
				float distance = radius - len;

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

Vec3 MeshCollider::ClosestPointOnLineSegment(Vec3 a, Vec3 b, Vec3 point)
{
	Vec3 ab = b - a;
	float t = Vec3::dot(point - a, ab) / Vec3::dot(ab, ab);
	return a + ab * (std::min)((std::max)(t, 0.0f), 1.0f);
}
