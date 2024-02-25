#include "game/physics.h"
#include "game/game.h"
#include "game/scene.h"
#include "game/collision_manager.h"
#include "game/component/collider/ray.h"

bool Physics::Raycast(const Vec3& origin, const Vec3& direction, const float& maxDistance, 
    RaycastHit& hit, const std::vector<std::string>& mask_tags)
{
    auto collision_manager = Game::Get()->GetCollisionManager();
    auto ray = Ray(origin, direction, maxDistance);

    // è’ìÀåüèo
    collision_manager->Detect(&ray, mask_tags);

    // è’ìÀÇ»Çµ
    if (ray.GetHits().empty())
        return false;

    auto& nearest = ray.GetNearestHit();

    hit.collider = nearest.collider;
    hit.distance = maxDistance - nearest.depth;
    hit.point = origin + direction * hit.distance;
    hit.normal = nearest.normal;

    return true;
}
