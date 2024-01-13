#include "game/component/collider/collider.h"
#include "game/entity.h"
#include "game/component/rigidbody.h"
#include "game/component/collider/sphere_collider.h"
#include "game/scene.h"
#include "game/collision_manager.h"
#include "game/component/collider/floor_collider.h"
#include "game/component/collider/mesh_collider.h"
#include "game/component/collider/capsule_collider.h"
#include <typeinfo>

bool Collider::Init()
{
    rigidbody_ = GetEntity()->GetComponent<Rigidbody>();

    GetEntity()->GetScene()->GetCollisionManager()->Add(this);

    return true;
}

void Collider::Prepare()
{
    // ˆÊ’u‚ÌXV
    position_ = transform->position;
    if (rigidbody_ != nullptr)
    {
        position_ += rigidbody_->velocity;
        //m_extension = m_pRigidbody->isGrounded ? Vec3(0, -0.1, 0) : Vec3::Zero();
    }

    scale_ = transform->scale;

    // Õ“Ëî•ñ‚Ì‰Šú‰»
    hit_colliders.clear();
    hit_normals.clear();
    hit_depths.clear();
}

Vec3 Collider::GetPosition()
{
    return position_;
}

Vec3 Collider::GetExtension()
{
    return extension_;
}

Rigidbody* Collider::GetRigidbody()
{
    return rigidbody_;
}

bool Collider::Intersects(Collider* collider)
{
    if (typeid(*collider) == typeid(SphereCollider))
    {
        return Intersects((SphereCollider*)collider);
    }
    else if (typeid(*collider) == typeid(CapsuleCollider))
    {
        return Intersects((CapsuleCollider*)collider);
    }
    else if (typeid(*collider) == typeid(FloorCollider))
    {
        return Intersects((FloorCollider*)collider);
    }
    else if (typeid(*collider) == typeid(MeshCollider))
    {
        return Intersects((MeshCollider*)collider);
    }
    
    return false;
}

bool Collider::Intersects(SphereCollider* sphere)
{
    return false;
}

bool Collider::Intersects(CapsuleCollider* sphere)
{
    return false;
}

bool Collider::Intersects(FloorCollider* floor)
{
    return false;
}

bool Collider::Intersects(MeshCollider* collider)
{
    return false;
}

bool Collider::Intersects(Ray* ray)
{
    return false;
}

Vec3 Collider::ClosestPointOnLineSegment(const Vec3& a, const Vec3& b, const Vec3& point)
{
    Vec3 ab = b - a;
    float t = Vec3::Dot(point - a, ab) / Vec3::Dot(ab, ab);
    return a + ab * (std::min)((std::max)(t, 0.0f), 1.0f);
}

