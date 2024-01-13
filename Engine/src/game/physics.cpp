#include "game/physics.h"
#include "game/game.h"
#include "game/scene.h"
#include "game/collision_manager.h"
#include "game/component/collider/ray.h"

bool Physics::Raycast(const Vec3& origin, const Vec3& direction, const float& maxDistance, RaycastHit& hit)
{
    auto collisionManager = Game::Get()->GetCurrentScene()->GetCollisionManager();
    auto ray = Ray(origin, direction, maxDistance);

    // �Փˌ��o
    collisionManager->Detect(&ray);

    // �Փ˂Ȃ�
    if (ray.hit_colliders.empty())
        return false;

    // �ł��߂��Փ˂�T��
    auto minDistance = maxDistance;
    auto index = 0;
    for (int i = 0; i < ray.hit_colliders.size(); i++)
    {
        auto distance = ray.hit_depths[i];
        if (distance < minDistance)
        {
            minDistance = distance;
            index = i;
        }
    }

    hit.collider = ray.hit_colliders[index];
    hit.distance = minDistance;
    hit.point = origin + direction * minDistance;
    hit.normal = ray.hit_normals[index];

    return true;
}
