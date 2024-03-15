#include "polygon_collider.h"
#include "game/game.h"
#include "game/collision_manager.h"
#include "game/component/collider/sphere_collider.h"
#include "game/component/collider/capsule_collider.h"
#include "game/component/collider/ray.h"
#include "game/component/collider/mesh_collider.h"
#include <utility>

PolygonCollider::PolygonCollider(const CollisionPolygon& polygon, MeshCollider* mesh_collider)
{
	vertices_[0] = polygon.vertices[0].position;
	vertices_[1] = polygon.vertices[1].position;
	vertices_[2] = polygon.vertices[2].position;
	normal_ = polygon.normal;
	default_aabb_ = polygon.aabb;

	mesh_collider_ = mesh_collider;
	rigidbody_ = mesh_collider->GetRigidbody();

	type_ = POLYGON;
}

PolygonCollider::~PolygonCollider()
{
}

bool PolygonCollider::Init()
{
	collision_manager_ = Game::Get()->GetCollisionManager();
	collision_manager_->Add(this);

	return true;
}

bool PolygonCollider::Intersects(SphereCollider* sphere)
{
	bool res = false;

	auto center = sphere->GetPosition() + sphere->offset;
	auto radius = sphere->radius;

	Vec3 normal;
	float distance;

	// 衝突
	if (SphereIntersectsPolygon(center, radius, normal, distance))
	{
		mesh_collider_->AddHit({ sphere, -normal, distance });
		sphere->AddHit({ this, normal, distance });

		res = true;
	}

	return res;
}

bool PolygonCollider::Intersects(CapsuleCollider* capsule)
{
	bool res = false;

	float radius = capsule->radius;
	float height = capsule->height;

	// TODO: 回転の対応
	Vec3 base = capsule->GetPosition() + capsule->offset - Vec3(0, 1, 0) * (height / 2 + radius);
	Vec3 tip = capsule->GetPosition() + capsule->offset + Vec3(0, 1, 0) * (height / 2 + radius);
	Vec3 capsuleNormal = (tip - base).Normalized();
	Vec3 lineEndOffset = capsuleNormal * radius;
	Vec3 A = base + lineEndOffset;
	Vec3 B = tip - lineEndOffset;

	auto p0 = Vec3::Scale(vertices_[0], scale) + position_ + offset;
	auto p1 = Vec3::Scale(vertices_[1], scale) + position_ + offset;
	auto p2 = Vec3::Scale(vertices_[2], scale) + position_ + offset;
	
	Vec3 N = normal_;	// 平面の法線

	Vec3 referencePoint;	// カプセルに最も近いポリゴン上の点

	// 平面の法線とカプセルが平行である場合、平面上の1つの点をreferencePointとする
	if (Vec3::Dot(N, capsuleNormal) == 0)
	{
		referencePoint = p0;
	}
	else
	{
		// カプセルのトレース線と平面の交点を見つける
		float t = Vec3::Dot(N, (p0 - base)) / Vec3::Dot(N, capsuleNormal);
		Vec3 linePlaneIntersection = base + capsuleNormal * t;

		// ポリゴンに最も近い点を見つける
		auto c0 = Vec3::Cross(linePlaneIntersection - p0, p1 - p0);
		auto c1 = Vec3::Cross(linePlaneIntersection - p1, p2 - p1);
		auto c2 = Vec3::Cross(linePlaneIntersection - p2, p0 - p2);
		bool inside = Vec3::Dot(c0, N) <= 0 && Vec3::Dot(c1, N) <= 0 && Vec3::Dot(c2, N) <= 0;

		if (inside)
		{
			referencePoint = linePlaneIntersection;
		}
		else
		{
			// Edge 1
			auto point1 = ClosestPointOnLineSegment(p0, p1, linePlaneIntersection);
			auto v1 = linePlaneIntersection - point1;
			float distsq = Vec3::Dot(v1, v1);
			float bestDist = distsq;
			referencePoint = point1;

			// Edge 2
			auto point2 = ClosestPointOnLineSegment(p1, p2, linePlaneIntersection);
			auto v2 = linePlaneIntersection - point2;
			distsq = Vec3::Dot(v2, v2);
			if (distsq < bestDist)
			{
				referencePoint = point2;
				bestDist = distsq;
			}

			// Edge 3
			auto point3 = ClosestPointOnLineSegment(p2, p0, linePlaneIntersection);
			auto v3 = linePlaneIntersection - point3;
			distsq = Vec3::Dot(v3, v3);
			if (distsq < bestDist)
			{
				referencePoint = point3;
				bestDist = distsq;
			}
		}
	}

	// トレース先の球を決定
	Vec3 center = ClosestPointOnLineSegment(A, B, referencePoint);

	Vec3 normal;
	float distance;

	// 衝突
	if (SphereIntersectsPolygon(center, radius, normal, distance))
	{
		mesh_collider_->AddHit({ capsule, -normal, distance });
		capsule->AddHit({ this, normal, distance });
		
		res = true;
	}

	return res;
}

bool PolygonCollider::Intersects(Ray* ray)
{
	bool res = false;

	auto rayOrigin = ray->origin;
	auto rayDirection = ray->direction;
	auto rayDistance = ray->distance;

	auto p0 = Vec3::Scale(vertices_[0], scale) + position_ + offset;
	auto p1 = Vec3::Scale(vertices_[1], scale) + position_ + offset;
	auto p2 = Vec3::Scale(vertices_[2], scale) + position_ + offset;

	Vec3 N = normal_;	// 平面の法線

	// Rayの点座標
	auto rp0 = rayOrigin;
	auto rp1 = rayOrigin + rayDirection * rayDistance;

	// Rayと平面の貫通
	if (Vec3::Dot(rp0 - p0, N) * Vec3::Dot(rp1 - p0, N) > 0)
	{
		return false;
	}

	// 貫通点の座標を確定
	auto d0 = abs(Vec3::Dot(N, rp0 - p0));
	auto d1 = abs(Vec3::Dot(N, rp1 - p0));
	auto dist = rayDistance * d0 / (d0 + d1);
	auto point = rayOrigin + rayDirection * dist;

	// 貫通点がポリゴン内部に含まれているか
	auto in0 = Vec3::Cross(point - p0, p1 - p0);
	auto in1 = Vec3::Cross(point - p1, p2 - p1);
	auto in2 = Vec3::Cross(point - p2, p0 - p2);
	bool inside = Vec3::Dot(in0, N) <= 0 && Vec3::Dot(in1, N) <= 0 && Vec3::Dot(in2, N) <= 0;

	if (inside)
	{
		ray->AddHit({ this, N, rayDistance - dist });

		res = true;
	}

	return res;
}

bool PolygonCollider::SphereIntersectsPolygon(
	const Vec3& center, const float radius,
	Vec3& normal, float& distance)
{
	Vec3 p[3];
	p[0] = Vec3::Scale(vertices_[0], scale) + position_ + offset;
	p[1] = Vec3::Scale(vertices_[1], scale) + position_ + offset;
	p[2] = Vec3::Scale(vertices_[2], scale) + position_ + offset;

	Vec3 N = normal_;	// 平面の法線
	float dist = Vec3::Dot(center - p[0], N);	// 球と平面の距離
	
	//if (dist < 0)	// 裏面なら
	//	return false;

	if (dist < -radius || dist > radius)
		return false;
	
	auto point0 = center - N * dist;	// 球の中心を平面に投影

	// point0がポリゴン内部にあるか判定
	auto c0 = Vec3::Cross(point0 - p[0], p[1] - p[0]);
	auto c1 = Vec3::Cross(point0 - p[1], p[2] - p[1]);
	auto c2 = Vec3::Cross(point0 - p[2], p[0] - p[2]);
	bool inside = Vec3::Dot(c0, N) <= 0 && Vec3::Dot(c1, N) <= 0 && Vec3::Dot(c2, N) <= 0;

	// 各エッジと交差しているか判定
	float radiussq = radius * radius;

	// Edge 1
	auto point1 = ClosestPointOnLineSegment(p[0], p[1], center);
	auto v1 = center - point1;
	float distsq1 = Vec3::Dot(v1, v1);
	bool intersects = distsq1 < radiussq;

	// Edge 2
	auto point2 = ClosestPointOnLineSegment(p[1], p[2], center);
	auto v2 = center - point2;
	float distsq2 = Vec3::Dot(v2, v2);
	intersects |= distsq2 < radiussq;

	// Edge 3
	auto point3 = ClosestPointOnLineSegment(p[2], p[0], center);
	auto v3 = center - point3;
	float distsq3 = Vec3::Dot(v3, v3);
	intersects |= distsq3 < radiussq;

	// 衝突
	if (inside || intersects)
	{
		Vec3 bestPoint;
		Vec3 intersectionVec;

		if (inside)
		{
			bestPoint = point0;
			intersectionVec = center - point0;
		}
		else
		{
			auto d = center - point1;
			float bestDistsq = Vec3::Dot(d, d);
			bestPoint = point1;
			intersectionVec = d;

			d = center - point2;
			float distsq = Vec3::Dot(d, d);
			if (distsq < bestDistsq)
			{
				bestDistsq = distsq;
				bestPoint = point2;
				intersectionVec = d;
			}

			d = center - point3;
			distsq = Vec3::Dot(d, d);
			if (distsq < bestDistsq)
			{
				bestDistsq = distsq;
				bestPoint = point3;
				intersectionVec = d;
			}
		}

		float len = intersectionVec.Length();
		normal = intersectionVec.Normalized();
		distance = radius - len;
		
		return true;
	}

	return false;
}

void PolygonCollider::PrepareAABB()
{
	aabb_ = default_aabb_;

	offset = mesh_collider_->offset;
	scale = mesh_collider_->scale;
}
