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

Collider::~Collider()
{
    collision_manager_->Remove(this);
}

bool Collider::Init()
{
    rigidbody_ = GetEntity()->GetComponent<Rigidbody>();
    
    collision_manager_ = Game::Get()->GetCollisionManager();
    collision_manager_->Add(this);

    return true;
}

void Collider::ResetHits()
{
    // 衝突情報のリセット
    hits_.clear();
}

void Collider::Prepare()
{
    // 位置の更新
    position_ = transform->position;
    if (rigidbody_ != nullptr)
    {
        position_ = rigidbody_->position;
        //position_ += rigidbody_->velocity;
        //m_extension = m_pRigidbody->isGrounded ? Vec3(0, -0.1, 0) : Vec3::Zero();
    }

    // AABBの計算
    PrepareAABB();
    aabb_.Scale(scale);
    aabb_.Translate(offset);
    //aabb_.Rotate(transform->rotation);
    aabb_.Translate(position_);

    // 最も近い衝突情報のリセット
    nearest_hit_ = { nullptr, Vec3::Zero(), 0 };

    has_detected_ = false;
}

Vec3 Collider::GetPosition()
{
    return position_;
}

Vec3 Collider::GetExtension()
{
    return extension_;
}

AABB Collider::GetAABB()
{
    return aabb_;
}

bool Collider::HasDetected()
{
    return has_detected_;
}

void Collider::SetHasDetected(bool flag)
{
    has_detected_ = flag;
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

void Collider::AddHit(const HitInfo& hit)
{
    hits_.push_back(hit);

    if (hit.depth > nearest_hit_.depth && hit.collider->rigidbody_)
    {
        nearest_hit_ = hit;
    }
}

const std::vector<Collider::HitInfo>& Collider::GetHits()
{
    return hits_;
}

const Collider::HitInfo& Collider::GetNearestHit()
{
    return nearest_hit_;
}

