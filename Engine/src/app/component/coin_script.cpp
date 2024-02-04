#include "coin_script.h"
#include "game/component/transform.h"
#include "math/quaternion.h"
#include "game/entity.h"
#include "game/scene.h"
#include "game/component/collider/sphere_collider.h"
#include "math/easing.h"

CoinScript::CoinScript()
{
    angle_ = 0;
    angle_speed_ = 0.02;
    effect_time_ = 0;
}

bool CoinScript::Init()
{
    collider_ = GetEntity()->GetComponent<SphereCollider>();

    return true;
}

void CoinScript::Update()
{
    if (destroy_flag_)
    {
        float t = (float)effect_time_ / 20;

        transform->scale = Vec3(1, 1, 1) * (1.0 - Easing::InBack(t));
        transform->position = Vec3(0, 2, 0) + Vec3(0, 2.5, 0) * Easing::OutBack(t);

        effect_time_ += 1;

        if (effect_time_ > 20)
        {
            GetEntity()->Destroy();
        }
    }

    angle_ += angle_speed_;
    transform->rotation = Quaternion::FromEuler(0, angle_, 0);
}

void CoinScript::OnCollisionEnter(Collider* collider)
{
    if (destroy_flag_) return;

    if (collider->GetEntity()->tag == "player")
    {
        GetEntity()->SetParent(collider->GetEntity());
        transform->position = Vec3(0, 2, 0);
        angle_speed_ = 0.3;

        destroy_flag_ = true;
    }
}
