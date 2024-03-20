#include "game/component/camera.h"
#include "engine/engine.h"
#include "game/game.h"

Camera::Camera()
{
    focus_position_ = XMFLOAT3(0, 0, 0);
    up_direction_ = XMFLOAT3(0, 1, 0);
    fov_ = 60;
    near_ = 0.3f;
    far_ = 1000;
    focus_ = false;

    mtx_view_ = XMMatrixIdentity();
    mtx_proj_ = XMMatrixIdentity();
}

void Camera::Focus(bool flag)
{
    focus_ = flag;
}

XMMATRIX Camera::GetViewMatrix() const
{
    return mtx_view_;
}

XMMATRIX Camera::GetProjMatrix() const
{
    return mtx_proj_;
}

Vec3 Camera::GetFocusPosition() const
{
    return focus_position_;
}

Vec3 Camera::GetViewDirection() const
{
    if (focus_)
    {
        return focus_position_ - position_;
    }
    else
    {
        return rotation_ * Vec3(0, 0, -1);
    }
}

void Camera::SetFocusPosition(const Vec3& focus_pos)
{
    focus_position_ = focus_pos;

    // target へのフォーカスは解除
    focus_target_ = nullptr;
}

void Camera::SetFocusPosition(float x, float y, float z)
{
    SetFocusPosition(XMFLOAT3(x, y, z));
}

void Camera::SetFocusTarget(Entity* focus_target)
{
    focus_target_ = focus_target;
}

void Camera::SetUpDirection(const Vec3& up_dir)
{
    up_direction_ = up_dir;
}

void Camera::SetUpDirection(float x, float y, float z)
{
    SetUpDirection(XMFLOAT3(x, y, z));
}

void Camera::SetFov(float fov)
{
    fov_ = fov;
}

bool Camera::Init()
{
    position_ = transform->position;
    rotation_ = transform->rotation;

    return true;
}

void Camera::CameraUpdate(const float delta_time)
{
    position_ = transform->position;
    rotation_ = transform->rotation;

    auto fov = XMConvertToRadians(fov_);
    auto aspect = Game::Get()->GetEngine()->AspectRate();

    if (focus_)
    {
        if (focus_target_)
        {
            // focus positionを更新
            focus_position_ = focus_target_->transform->WorldPosition();
        }

        if (position_ != focus_position_)
        {
            // view 行列
            mtx_view_ = XMMatrixLookAtRH(position_, focus_position_, up_direction_);

            // rotation を更新
            transform->rotation = XMQuaternionRotationMatrix(mtx_view_);
        }
    }
    else
    {
        // view 行列
        auto mtx =
            XMMatrixRotationQuaternion(rotation_.Conjugate()) *
            XMMatrixTranslationFromVector(position_);
        mtx_view_ = XMMatrixInverse(nullptr, mtx);
    }

    // projection 行列
    mtx_proj_ = XMMatrixPerspectiveFovRH(fov, aspect, near_, far_);
}
