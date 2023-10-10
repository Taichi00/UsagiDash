#include "Camera.h"
#include "Engine.h"

Camera::Camera()
{
    m_focusPos = XMFLOAT3(0, 0, 0);
    m_upDir = XMFLOAT3(0, 1, 0);
    m_fov = 60;
    m_near = 0.3f;
    m_far = 100000;

    m_mtxView = XMMatrixIdentity();
    m_mtxProj = XMMatrixIdentity();
}

XMMATRIX Camera::GetViewMatrix()
{
    return m_mtxView;
}

XMMATRIX Camera::GetProjMatrix()
{
    return m_mtxProj;
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
    return true;
}

void Camera::CameraUpdate()
{
    auto pos = XMFLOAT3(m_pEntity->Position());
    auto fov = XMConvertToRadians(m_fov);
    auto aspect = g_Engine->AspectRate();

    m_mtxView = XMMatrixLookAtRH(XMLoadFloat3(&pos), XMLoadFloat3(&m_focusPos), XMLoadFloat3(&m_upDir));
    m_mtxProj = XMMatrixPerspectiveFovRH(fov, aspect, m_near, m_far);
}
