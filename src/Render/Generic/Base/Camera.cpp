#ifndef CAMERA_CPP_DEFINED
#define CAMERA_CPP_DEFINED

#include "Camera.h"

Camera::Camera()
{
}

Camera::~Camera()
{

}

void Camera::SetWorld(XMFLOAT4X4& _world)
{
    m_world = _world;
    UpdateMatrices();
}

void Camera::SetAspectRatio(float _aspectRatio)
{
    m_aspectRatio = _aspectRatio;
    XMMATRIX P = XMMatrixPerspectiveFovLH(fov, m_aspectRatio, nearPlane, farPlane);
    XMStoreFloat4x4(&m_proj, P);

    UpdateMatrices();
}

void Camera::UpdateMatrices()
{
    XMMATRIX w = XMLoadFloat4x4(&m_world);
    XMMATRIX v = XMMatrixInverse(nullptr, w);
    XMMATRIX p = XMLoadFloat4x4(&m_proj);
    
    XMStoreFloat4x4(&m_view, v);

    XMMATRIX vp = XMMatrixMultiply(v, p);
    vp = XMMatrixTranspose(vp);

    XMStoreFloat4x4(&m_viewProj, vp);

    m_pos = {m_world._41, m_world._42, m_world._43};

    CalculateFrustum();
}

void Camera::SetRotation(XMFLOAT3 ypr)
{
    XMMATRIX rot = XMMatrixRotationRollPitchYaw(ypr.y, ypr.x, ypr.z);

    XMMATRIX world = rot;
    world.r[3] = XMVectorSet(m_pos.x, m_pos.y, m_pos.z, 1.0f);

    XMStoreFloat4x4(&m_world, world);

    UpdateMatrices();
}

void Camera::LookAt(XMFLOAT3 _target)
{
    XMVECTOR posV    = XMLoadFloat3(&m_pos);
    XMVECTOR targetV = XMLoadFloat3(&_target);

    // Guard against degenerate input
    XMVECTOR dir = XMVectorSubtract(targetV, posV);
    if (XMVector3Equal(dir, XMVectorZero()))
        return;

    XMFLOAT3 up = { 0.0f, 1.0f, 0.0f };
    XMMATRIX view = XMMatrixLookAtLH(posV, targetV, XMLoadFloat3(&up));

    XMMATRIX world = XMMatrixInverse(nullptr, view);
    XMStoreFloat4x4(&m_world, world);

    UpdateMatrices();
}

void Camera::SetPos(XMFLOAT3 _pos)
{
    m_pos = _pos;
    m_world._41 = _pos.x;
    m_world._42 = _pos.y;
    m_world._43 = _pos.z;

    UpdateMatrices();
}

void Camera::FillData(PassData* _passData)
{
    _passData->viewProj = m_viewProj;
    _passData->view = m_view;
    _passData->proj = m_proj;

    XMMATRIX temp = XMLoadFloat4x4(&m_viewProj);
    temp = XMMatrixInverse(nullptr, temp);
    XMStoreFloat4x4(&_passData->invViewProj, temp);

    _passData->eyePosW = m_pos;

    _passData->nearZ = nearPlane;
    _passData->farZ = farPlane;
}

Plane Camera::ExctractPlane(XMFLOAT3 _p0, XMFLOAT3 _p1, XMFLOAT3 _p2)
{
    Plane p;
    
}

void Camera::CalculateFrustum()
{
    XMMATRIX w = XMLoadFloat4x4(&m_world);
    
    XMFLOAT4 right, up, front, pos;
    XMStoreFloat4(&right, w.r[0]);
    XMStoreFloat4(&up, w.r[1]);
    XMStoreFloat4(&front, w.r[2]);
    XMStoreFloat4(&pos, w.r[3]);
    
    XMFLOAT3 nearCenter =   {m_pos.x + front.x * nearPlane, m_pos.y + front.y * nearPlane, m_pos.z + front.z * nearPlane};
    XMFLOAT3 farCenter =    {m_pos.x + front.x * farPlane, m_pos.y + front.y * farPlane, m_pos.z + front.z * farPlane};
    float nearHeight = 2 * tanf(fov / 2) * nearPlane;
    float farHeight = 2 * tanf(fov / 2) * farPlane;
    float nearWidth = nearHeight * m_aspectRatio;
    float farWidth = farHeight * m_aspectRatio;
    
    XMFLOAT3 farTopLeft = {
        farCenter.x + up.x * farHeight * 0.5f - right.x * farWidth * 0.5f,
        farCenter.y + up.y * farHeight * 0.5f - right.y * farWidth * 0.5f,
        farCenter.z + up.z * farHeight * 0.5f - right.z * farWidth * 0.5f
    };
    XMFLOAT3 farTopRight = {
        farCenter.x + up.x * farHeight * 0.5f + right.x * farWidth * 0.5f,
        farCenter.y + up.y * farHeight * 0.5f + right.y * farWidth * 0.5f,
        farCenter.z + up.z * farHeight * 0.5f + right.z * farWidth * 0.5f
    };
    XMFLOAT3 farBottomLeft = {
        farCenter.x - up.x * farHeight * 0.5f - right.x * farWidth * 0.5f,
        farCenter.y - up.y * farHeight * 0.5f - right.y * farWidth * 0.5f,
        farCenter.z - up.z * farHeight * 0.5f - right.z * farWidth * 0.5f
    };
    XMFLOAT3 farBottomRight = {
        farCenter.x - up.x * farHeight * 0.5f + right.x * farWidth * 0.5f,
        farCenter.y - up.y * farHeight * 0.5f + right.y * farWidth * 0.5f,
        farCenter.z - up.z * farHeight * 0.5f + right.z * farWidth * 0.5f
    };
    XMFLOAT3 nearTopLeft = {
        nearCenter.x + up.x * nearHeight * 0.5f - right.x * nearWidth * 0.5f,
        nearCenter.y + up.y * nearHeight * 0.5f - right.y * nearWidth * 0.5f,
        nearCenter.z + up.z * nearHeight * 0.5f - right.z * nearWidth * 0.5f
    };
    XMFLOAT3 nearTopRight = {
        nearCenter.x + up.x * nearHeight * 0.5f + right.x * nearWidth * 0.5f,
        nearCenter.y + up.y * nearHeight * 0.5f + right.y * nearWidth * 0.5f,
        nearCenter.z + up.z * nearHeight * 0.5f + right.z * nearWidth * 0.5f
    };
    XMFLOAT3 nearBottomLeft = {
        nearCenter.x - up.x * nearHeight * 0.5f - right.x * nearWidth * 0.5f,
        nearCenter.y - up.y * nearHeight * 0.5f - right.y * nearWidth * 0.5f,
        nearCenter.z - up.z * nearHeight * 0.5f - right.z * nearWidth * 0.5f
    };
    XMFLOAT3 nearBottomRight = {
        nearCenter.x - up.x * nearHeight * 0.5f + right.x * nearWidth * 0.5f,
        nearCenter.y - up.y * nearHeight * 0.5f + right.y * nearWidth * 0.5f,
        nearCenter.z - up.z * nearHeight * 0.5f + right.z * nearWidth * 0.5f
    };
    
    
    
    OutputDebugStringA(("Far normal: " + std::to_string(m_frustum.farFace.normal.x) + ", " + std::to_string(m_frustum.farFace.normal.y) + ", " + std::to_string(m_frustum.farFace.normal.z) + "\n").c_str());
    OutputDebugStringA(("Far distance: " + std::to_string(m_frustum.farFace.distance) + "\n").c_str());
}

#endif
