#include "game/component/camera.h"
#include "engine/engine.h"
#include "game/game.h"

Camera::Camera()
{
    focus_pos_ = XMFLOAT3(0, 0, 0);
    up_dir_ = XMFLOAT3(0, 1, 0);
    fov_ = 60;
    near_ = 0.3f;
    far_ = 1000;

    mtx_view_ = XMMatrixIdentity();
    mtx_proj_ = XMMatrixIdentity();
}

XMMATRIX Camera::GetViewMatrix()
{
    return mtx_view_;
}

XMMATRIX Camera::GetProjMatrix()
{
    return mtx_proj_;
}

Vec3 Camera::GetFocusPosition()
{
    return focus_pos_;
}

Vec3 Camera::GetViewDirection()
{
    return Vec3(focus_pos_) - transform->position;
}

void Camera::SetFocusPosition(XMFLOAT3 focusPos)
{
    focus_pos_ = focusPos;
}

void Camera::SetFocusPosition(float x, float y, float z)
{
    SetFocusPosition(XMFLOAT3(x, y, z));
}

void Camera::SetUpDirection(XMFLOAT3 upDir)
{
    up_dir_ = upDir;
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
    return true;
}

void Camera::CameraUpdate(const float delta_time)
{
    auto pos = XMFLOAT3(transform->position);
    auto fov = XMConvertToRadians(fov_);
    auto aspect = Game::Get()->GetEngine()->AspectRate();

    if (transform->position != focus_pos_)
    {
        mtx_view_ = XMMatrixLookAtRH(XMLoadFloat3(&pos), XMLoadFloat3(&focus_pos_), XMLoadFloat3(&up_dir_));
        transform->rotation = XMQuaternionRotationMatrix(mtx_view_);
    }
    mtx_proj_ = XMMatrixPerspectiveFovRH(fov, aspect, near_, far_);
}
