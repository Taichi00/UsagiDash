#include "game/physics/physics.h"
#include "game/game.h"
#include "game/scene.h"
#include "game/collision_manager.h"
#include "game/component/collider/ray.h"

bool Physics::Raycast(
    const Vec3& origin, 
    const Vec3& direction, 
    const float maxDistance, 
    RaycastHit& hit, 
    const std::vector<std::string>& mask_layers
)
{
    auto collision_manager = Game::Get()->GetCollisionManager();
    auto ray = RayShape(origin, direction, maxDistance);

    ray.aabb.max = Vec3::Max(origin, origin + direction * maxDistance);
    ray.aabb.min = Vec3::Min(origin, origin + direction * maxDistance);

    Physics::Hit nearest_hit = {};
    Collider* nearest_collider = nullptr;

    // 衝突検出
    if (!collision_manager->Raycast(&ray, mask_layers, nearest_hit, nearest_collider))
        return false;

    hit.collider = nearest_collider;
    hit.distance = maxDistance - nearest_hit.depth;
    hit.point = origin + direction * hit.distance;
    hit.normal = -nearest_hit.normal;
    
    return true;
}

bool Physics::DetectCollision(CollisionShape* shape1, CollisionShape* shape2, Hit& hit)
{
    bool result = false;
    bool inverse = false;

    switch (shape1->type)
    {
    case CollisionShape::SPHERE:
        switch (shape2->type)
        {
        case CollisionShape::SPHERE:
            result = DetectSphereSphereCollision(
                static_cast<SphereShape*>(shape1), 
                static_cast<SphereShape*>(shape2),
                hit
            );
            break;

        case CollisionShape::CAPSULE:
            result = DetectCapsuleSphereCollision(
                static_cast<CapsuleShape*>(shape2),
                static_cast<SphereShape*>(shape1),
                hit
            );
            inverse = true;
            break;

        case CollisionShape::POLYGON:
            result = DetectPolygonSphereCollision(
                static_cast<PolygonShape*>(shape2),
                static_cast<SphereShape*>(shape1),
                hit
            );
            inverse = true;
            break;

        case CollisionShape::RAY:
            result = DetectSphereRayCollision(
                static_cast<SphereShape*>(shape1),
                static_cast<RayShape*>(shape2),
                hit
            );
            break;
        }
        break;

    case CollisionShape::CAPSULE:
        switch (shape2->type)
        {
        case CollisionShape::SPHERE:
            result = DetectCapsuleSphereCollision(
                static_cast<CapsuleShape*>(shape1),
                static_cast<SphereShape*>(shape2),
                hit
            );
            break;

        case CollisionShape::CAPSULE:
            result = DetectCapsuleCapsuleCollision(
                static_cast<CapsuleShape*>(shape1),
                static_cast<CapsuleShape*>(shape2),
                hit
            );
            break;

        case CollisionShape::POLYGON:
            result = DetectPolygonCapsuleCollision(
                static_cast<PolygonShape*>(shape2),
                static_cast<CapsuleShape*>(shape1),
                hit
            );
            inverse = true;
            break;

        case CollisionShape::RAY:
            result = DetectCapsuleRayCollision(
                static_cast<CapsuleShape*>(shape1),
                static_cast<RayShape*>(shape2),
                hit
            );
            break;
        }
        break;

    case CollisionShape::POLYGON:
        switch (shape2->type)
        {
        case CollisionShape::SPHERE:
            result = DetectPolygonSphereCollision(
                static_cast<PolygonShape*>(shape1),
                static_cast<SphereShape*>(shape2),
                hit
            );
            break;

        case CollisionShape::CAPSULE:
            result = DetectPolygonCapsuleCollision(
                static_cast<PolygonShape*>(shape1),
                static_cast<CapsuleShape*>(shape2),
                hit
            );
            break;

        case CollisionShape::POLYGON:
            break;

        case CollisionShape::RAY:
            result = DetectPolygonRayCollision(
                static_cast<PolygonShape*>(shape1),
                static_cast<RayShape*>(shape2),
                hit
            );
            break;
        }
        break;

    case CollisionShape::RAY:
        switch (shape2->type)
        {
        case CollisionShape::SPHERE:
            result = DetectSphereRayCollision(
                static_cast<SphereShape*>(shape2),
                static_cast<RayShape*>(shape1),
                hit
            );
            inverse = true;
            break;

        case CollisionShape::CAPSULE:
            result = DetectCapsuleRayCollision(
                static_cast<CapsuleShape*>(shape2),
                static_cast<RayShape*>(shape1),
                hit
            );
            inverse = true;
            break;

        case CollisionShape::POLYGON:
            result = DetectPolygonRayCollision(
                static_cast<PolygonShape*>(shape2),
                static_cast<RayShape*>(shape1),
                hit
            );
            inverse = true;
            break;

        case CollisionShape::RAY:
            break;
        }
    }

    if (inverse)
    {
        hit.normal *= -1;
    }

    return result;
}

bool Physics::DetectSphereSphereCollision(
    SphereShape* sphere1, 
    SphereShape* sphere2,
    Hit& hit
)
{
    Vec3 position1 = sphere1->position;
    Vec3 position2 = sphere2->position;
    float radius1 = sphere1->radius;
    float radius2 = sphere2->radius;

    Vec3 v = position1 - position2;
    float distance = radius1 + radius2 - v.Length();

    if (distance > 0)
    {
        auto normal = v.Normalized();

        hit.normal = normal;
        hit.depth = distance;

        return true;
    }

    return false;
}

bool Physics::DetectSphereRayCollision(SphereShape* sphere, RayShape* ray, Hit& hit)
{
    Vec3 position = sphere->position;
    float radius = sphere->radius;
    Vec3 ray_dir = ray->direction;
    Vec3 rayA = ray->origin;
    Vec3 rayB = rayA + ray_dir * ray->distance;

    auto point = ClosestPointOnLineSegment(rayA, rayB, position);

    auto v = position - point;
    auto distance = radius - v.Length();

    if (distance > 0)
    {
        // 球とRayの交点を求める
        auto dis = rayA - position;
        double D = std::pow(Vec3::Dot(ray_dir, dis), 2) - (std::pow(dis.Length(), 2) - radius * radius);
        auto hit_point = rayA + ray_dir * (-Vec3::Dot(ray_dir, dis) - (float)std::sqrt(D));

        auto normal = -ray_dir.Normalized();
        auto depth = (hit_point - rayB).Length();

        hit.normal = normal;
        hit.depth = depth;

        return true;
    }

    return false;
}

bool Physics::DetectCapsuleCapsuleCollision(
    CapsuleShape* capsule1, 
    CapsuleShape* capsule2,
    Hit& hit
)
{
    Vec3 position1 = capsule1->position;
    Vec3 position2 = capsule2->position;
    float radius1 = capsule1->radius;
    float radius2 = capsule2->radius;
    float height1 = capsule1->height;
    float height2 = capsule2->height;

    Vec3 A1 = position1 - Vec3(0, 1, 0) * (height1 / 2);
    Vec3 B1 = position1 + Vec3(0, 1, 0) * (height1 / 2);
    Vec3 A2 = position2 - Vec3(0, 1, 0) * (height2 / 2);
    Vec3 B2 = position2 + Vec3(0, 1, 0) * (height2 / 2);

    Vec3 v0 = A2 - A1;
    Vec3 v1 = B2 - A1;
    Vec3 v2 = A2 - B1;
    Vec3 v3 = B2 - B1;

    float d0 = Vec3::Dot(v0, v0);
    float d1 = Vec3::Dot(v1, v1);
    float d2 = Vec3::Dot(v2, v2);
    float d3 = Vec3::Dot(v3, v3);

    Vec3 best1;
    if (d2 < d0 || d2 < d1 || d3 < d0 || d3 < d1)
    {
        best1 = B1;
    }
    else
    {
        best1 = A1;
    }

    Vec3 best2 = ClosestPointOnLineSegment(A2, B2, best1);
    best1 = ClosestPointOnLineSegment(A1, B1, best2);

    Vec3 v = best1 - best2;
    float distance = radius1 + radius2 - v.Length();

    if (distance > 0)
    {
        auto normal = v.Normalized();

        hit.normal = normal;
        hit.depth = distance;

        return true;
    }

    return false;
}

bool Physics::DetectCapsuleSphereCollision(
    CapsuleShape* capsule, 
    SphereShape* sphere,
    Hit& hit
)
{
    Vec3 position1 = capsule->position;
    Vec3 position2 = sphere->position;
    float radius1 = capsule->radius;
    float radius2 = sphere->radius;

    Vec3 A = position1 - Vec3(0, 1, 0) * (capsule->height / 2);
    Vec3 B = position1 + Vec3(0, 1, 0) * (capsule->height / 2);

    // 球の中心と最も近い点を見つける
    Vec3 reference_position = ClosestPointOnLineSegment(A, B, position2);

    Vec3 v = reference_position - position2;
    float distance = radius1 + radius2 - v.Length();

    if (distance > 0)
    {
        auto normal = v.Normalized();

        hit.normal = normal;
        hit.depth = distance;

        return true;
    }

    return false;
}

bool Physics::DetectCapsuleRayCollision(CapsuleShape* capsule, RayShape* ray, Hit& hit)
{
    return false;
}

bool Physics::DetectPolygonSphereCollision(
    PolygonShape* polygon, 
    SphereShape* sphere,
    Hit& hit
)
{
    auto p1 = polygon->v1 + polygon->position;
    auto p2 = polygon->v2 + polygon->position;
    auto p3 = polygon->v3 + polygon->position;

    auto center = sphere->position;
    auto radius = sphere->radius;

    Vec3 N = polygon->normal;	// 平面の法線
    float dist = Vec3::Dot(center - p1, N);	// 球と平面の距離

    //if (dist < 0)	// 裏面なら
    //	return false;

    if (dist < -radius || dist > radius)
        return false;

    auto point0 = center - N * dist;	// 球の中心を平面に投影

    // point0がポリゴン内部にあるか判定
    auto c1 = Vec3::Cross(point0 - p1, p2 - p1);
    auto c2 = Vec3::Cross(point0 - p2, p3 - p2);
    auto c3 = Vec3::Cross(point0 - p3, p1 - p3);
    bool inside = Vec3::Dot(c1, N) <= 0 && Vec3::Dot(c2, N) <= 0 && Vec3::Dot(c3, N) <= 0;

    // 各エッジと交差しているか判定
    float radiussq = radius * radius;

    // Edge 1
    auto point1 = ClosestPointOnLineSegment(p1, p2, center);
    auto v1 = center - point1;
    float distsq1 = Vec3::Dot(v1, v1);
    bool intersects = distsq1 < radiussq;

    // Edge 2
    auto point2 = ClosestPointOnLineSegment(p2, p3, center);
    auto v2 = center - point2;
    float distsq2 = Vec3::Dot(v2, v2);
    intersects |= distsq2 < radiussq;

    // Edge 3
    auto point3 = ClosestPointOnLineSegment(p3, p1, center);
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
        Vec3 normal = intersectionVec.Normalized();
        float distance = radius - len;

        hit.normal = -normal;
        hit.depth = distance;

        return true;
    }

    return false;
}

bool Physics::DetectPolygonCapsuleCollision(
    PolygonShape* polygon, 
    CapsuleShape* capsule,
    Hit& hit
)
{
    bool res = false;

    float radius = capsule->radius;
    float height = capsule->height;

    Vec3 base = capsule->position - Vec3(0, 1, 0) * (height / 2 + radius);
    Vec3 tip  = capsule->position + Vec3(0, 1, 0) * (height / 2 + radius);
    Vec3 capsuleNormal = (tip - base).Normalized();
    Vec3 lineEndOffset = capsuleNormal * radius;
    Vec3 A = base + lineEndOffset;
    Vec3 B = tip - lineEndOffset;

    auto p1 = polygon->v1 + polygon->position;
    auto p2 = polygon->v2 + polygon->position;
    auto p3 = polygon->v3 + polygon->position;

    Vec3 N = polygon->normal;	// 平面の法線

    Vec3 referencePoint;	// カプセルに最も近いポリゴン上の点

    // 平面の法線とカプセルが平行である場合、平面上の1つの点をreferencePointとする
    if (Vec3::Dot(N, capsuleNormal) == 0)
    {
        referencePoint = p1;
    }
    else
    {
        // カプセルのトレース線と平面の交点を見つける
        float t = Vec3::Dot(N, (p1 - base)) / Vec3::Dot(N, capsuleNormal);
        Vec3 linePlaneIntersection = base + capsuleNormal * t;

        // ポリゴンに最も近い点を見つける
        auto c1 = Vec3::Cross(linePlaneIntersection - p1, p2 - p1);
        auto c2 = Vec3::Cross(linePlaneIntersection - p2, p3 - p2);
        auto c3 = Vec3::Cross(linePlaneIntersection - p3, p1 - p3);
        bool inside = Vec3::Dot(c1, N) <= 0 && Vec3::Dot(c2, N) <= 0 && Vec3::Dot(c3, N) <= 0;

        if (inside)
        {
            referencePoint = linePlaneIntersection;
        }
        else
        {
            // Edge 1
            auto point1 = ClosestPointOnLineSegment(p1, p2, linePlaneIntersection);
            auto v1 = linePlaneIntersection - point1;
            float distsq = Vec3::Dot(v1, v1);
            float bestDist = distsq;
            referencePoint = point1;

            // Edge 2
            auto point2 = ClosestPointOnLineSegment(p2, p3, linePlaneIntersection);
            auto v2 = linePlaneIntersection - point2;
            distsq = Vec3::Dot(v2, v2);
            if (distsq < bestDist)
            {
                referencePoint = point2;
                bestDist = distsq;
            }

            // Edge 3
            auto point3 = ClosestPointOnLineSegment(p3, p1, linePlaneIntersection);
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

    SphereShape sphere(radius);
    sphere.position = center;

    // 衝突
    if (DetectPolygonSphereCollision(polygon, &sphere, hit))
    {
        return true;
    }

    return false;
}

bool Physics::DetectPolygonRayCollision(PolygonShape* polygon, RayShape* ray, Hit& hit)
{
    bool res = false;

    auto rayOrigin = ray->origin;
    auto rayDirection = ray->direction;
    auto rayDistance = ray->distance;

    auto p1 = polygon->v1 + polygon->position;
    auto p2 = polygon->v2 + polygon->position;
    auto p3 = polygon->v3 + polygon->position;

    Vec3 N = polygon->normal;	// 平面の法線

    // Rayの点座標
    auto rp1 = rayOrigin;
    auto rp2 = rayOrigin + rayDirection * rayDistance;

    // Rayと平面の貫通
    if (Vec3::Dot(rp1 - p1, N) * Vec3::Dot(rp2 - p1, N) > 0)
    {
        return false;
    }

    // 貫通点の座標を確定
    auto d1 = abs(Vec3::Dot(N, rp1 - p1));
    auto d2 = abs(Vec3::Dot(N, rp2 - p1));
    auto dist = rayDistance * d1 / (d1 + d2);
    auto point = rayOrigin + rayDirection * dist;

    // 貫通点がポリゴン内部に含まれているか
    auto in1 = Vec3::Cross(point - p1, p2 - p1);
    auto in2 = Vec3::Cross(point - p2, p3 - p2);
    auto in3 = Vec3::Cross(point - p3, p1 - p3);
    bool inside = Vec3::Dot(in1, N) <= 0 && Vec3::Dot(in2, N) <= 0 && Vec3::Dot(in3, N) <= 0;

    if (inside)
    {
        hit.normal = N;
        hit.depth = rayDistance - dist;

        res = true;
    }

    return res;
}

Vec3 Physics::ClosestPointOnLineSegment(const Vec3& a, const Vec3& b, const Vec3& point)
{
    Vec3 ab = b - a;
    float t = Vec3::Dot(point - a, ab) / Vec3::Dot(ab, ab);
    return a + ab * std::min(std::max(t, 0.0f), 1.0f);
}
