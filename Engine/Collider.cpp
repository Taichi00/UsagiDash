#include "Collider.h"
#include "Entity.h"
#include "Rigidbody.h"
#include "SphereCollider.h"
#include "Scene.h"
#include "CollisionManager.h"
#include "FloorCollider.h"
#include "MeshCollider.h"
#include <typeinfo>

bool Collider::Init()
{
    m_pRigidbody = m_pEntity->GetComponent<Rigidbody>();

    m_pEntity->GetScene()->GetCollisionManager()->Add(this);

    return true;
}

void Collider::Prepare()
{
    // ˆÊ’u‚ÌXV
    m_position = transform->position;
    if (m_pRigidbody != nullptr)
    {
        m_position += m_pRigidbody->velocity;
    }

    m_scale = transform->scale;

    // Õ“Ëî•ñ‚Ì‰Šú‰»
    hitColliders.clear();
    hitNormals.clear();
    hitDistances.clear();
}

Vec3 Collider::GetPosition()
{
    return m_position;
}

Rigidbody* Collider::GetRigidbody()
{
    return m_pRigidbody;
}

bool Collider::Intersects(Collider* collider)
{
    if (typeid(*collider) == typeid(SphereCollider))
    {
        return Intersects((SphereCollider*)collider);
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

bool Collider::Intersects(FloorCollider* floor)
{
    return false;
}

bool Collider::Intersects(MeshCollider* collider)
{
    return false;
}
