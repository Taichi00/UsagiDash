#include "Physics.h"
#include "Game.h"
#include "Scene.h"
#include "CollisionManager.h"
#include "Ray.h"

bool Physics::Raycast(const Vec3& origin, const Vec3& direction, const float& maxDistance, RaycastHit& hit)
{
    auto collisionManager = Game::Get()->GetCurrentScene()->GetCollisionManager();
    auto ray = Ray(origin, direction, maxDistance);

    // Õ“ËŒŸo
    collisionManager->Detect(&ray);

    // Õ“Ë‚È‚µ
    if (ray.hitColliders.empty())
        return false;

    // Å‚à‹ß‚¢Õ“Ë‚ğ’T‚·
    auto minDistance = maxDistance;
    auto index = 0;
    for (int i = 0; i < ray.hitColliders.size(); i++)
    {
        auto distance = ray.hitDistances[i];
        if (distance < minDistance)
        {
            minDistance = distance;
            index = i;
        }
    }

    hit.collider = ray.hitColliders[index];
    hit.distance = minDistance;
    hit.point = origin + direction * minDistance;
    hit.normal = ray.hitNormals[index];

    return true;
}
