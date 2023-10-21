#include "Camera.h"
#include "Engine.h"


XMMATRIX Camera::GetViewMatrix()
{
    return m_mtxView;
}

XMMATRIX Camera::GetProjMatrix()
{
    return m_mtxProj;
}

Vec3 Camera::GetFocusPosition()
{
    return m_focusPos;
}

Vec3 Camera::GetViewDirection()
{
    return Vec3(m_focusPos) - transform->position;
}

void Camera::SetFocusPosition(XMFLOAT3 focusPos)
{
    m_focusPos = focusPos;
}

void Camera::SetFocusPosition(float x, float y, float z)
{
    SetFocusPosition(XMFLOAT3(x, y, z));
}

void Camera::SetUpDirection(XMFLOAT3 upDir)
{
    m_upDir = upDir;
}

void Camera::SetUpDirection(float x, float y, float z)
{
    SetUpDirection(XMFLOAT3(x, y, z));
}

void Camera::SetFov(float fov)
{
    m_fov = fov;
}

bool Camera::Init()
{
    m_focusPos = XMFLOAT3(0, 0, 0);
    m_upDir = XMFLOAT3(0, 1, 0);
    m_fov = 60;
    m_near = 0.3f;
    m_far = 100000;

    m_mtxView = XMMatrixIdentity();
    m_mtxProj = XMMatrixIdentity();

    return true;
}

void Camera::CameraUpdate()
{
    auto pos = XMFLOAT3(transform->position);
    auto fov = XMConvertToRadians(m_fov);
    auto aspect = g_Engine->AspectRate();

    if (transform->position != m_focusPos)
    {
        m_mtxView = XMMatrixLookAtRH(XMLoadFloat3(&pos), XMLoadFloat3(&m_focusPos), XMLoadFloat3(&m_upDir));
        transform->rotation = XMQuaternionRotationMatrix(m_mtxView);
    }
    m_mtxProj = XMMatrixPerspectiveFovRH(fov, aspect, m_near, m_far);
}
