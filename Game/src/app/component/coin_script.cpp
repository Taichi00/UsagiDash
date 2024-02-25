#include "coin_script.h"
#include "game/component/transform.h"
#include "math/quaternion.h"
#include "game/entity.h"
#include "game/scene.h"
#include "game/component/collider/sphere_collider.h"
#include "math/easing.h"
#include "app/component/game_manager.h"
#include "game/component/audio/audio_source.h"

CoinScript::CoinScript()
{
    angle_ = 0;
    angle_speed_ = 1.2f;
    effect_time_ = 0;
}

bool CoinScript::Init()
{
    collider_ = GetEntity()->GetComponent<SphereCollider>();
    audio_source_ = GetEntity()->GetComponent<AudioSource>();

    return true;
}

void CoinScript::Update(const float delta_time)
{
    if (destroy_flag_)
    {
        float t = effect_time_ / 20;

        transform->scale = Vec3(1, 1, 1) * (1.0f - Easing::InBack(t));
        transform->position = Vec3(0, 2, 0) + Vec3(0, 2.5, 0) * Easing::OutBack(t);

        effect_time_ += 60.0f * delta_time;

        if (effect_time_ > 20)
        {
            GetEntity()->Destroy();
        }
    }

    angle_ += angle_speed_ * delta_time;
    transform->rotation = Quaternion::FromEuler(0, angle_, 0);
}

void CoinScript::OnCollisionEnter(Collider* collider)
{
    if (destroy_flag_) return;

    if (collider->GetEntity()->tag == "player")
    {
        GetEntity()->SetParent(collider->GetEntity());
        transform->position = Vec3(0, 2, 0);
        angle_speed_ = 18;

        destroy_flag_ = true;

        GameManager::Get()->AddCoin(1);
        audio_source_->Play(0.8f);
        audio_source_->SetPitch(50.f);
    }
}
