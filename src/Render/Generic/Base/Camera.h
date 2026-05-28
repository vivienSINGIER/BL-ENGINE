#ifndef CAMERA_H_DEFINED
#define CAMERA_H_DEFINED

#include "../../Common/Common.h"
#include "../../Common/Utils/FrustumHelper.hpp"

struct PassData
{
    XMFLOAT4X4 viewProj;
    XMFLOAT4X4 view;
    XMFLOAT4X4 proj;
    XMFLOAT4X4 invViewProj;

    XMFLOAT3   eyePosW;
    float               nearZ;
    
    float               farZ;
    float               totalTime;
    float               deltaTime;
    float               pad0;

    XMFLOAT2   renderTargetSize;
    XMFLOAT2   invRenderTargetSize;
};

class Camera 
{
public:
    float fov = 0.25f * MathHelper::Pi;
    float nearPlane = 1.0f;
    float farPlane = 1000.0f;
    
    Camera();
    ~Camera();
    
    void SetWorld(XMFLOAT4X4& _world);
    void SetAspectRatio(float _aspectRatio);
    void UpdateMatrices();
    
    Frustum& GetFrustum() { return m_frustum; }

    // TODO REMOVE
    void SetRotation(XMFLOAT3 ypr);
    void LookAt(XMFLOAT3 _target);
    void SetPos(XMFLOAT3 _pos);
    
private:
    XMFLOAT4X4 m_world      = MathHelper::Identity4x4();
    XMFLOAT4X4 m_view       = MathHelper::Identity4x4();
    XMFLOAT4X4 m_proj       = MathHelper::Identity4x4();
    XMFLOAT4X4 m_viewProj   = MathHelper::Identity4x4();
    
    XMFLOAT3 m_pos = {0.0f, 0.0f, 0.0f};
    
    float m_aspectRatio = 0.0f;
    Frustum m_frustum;
    
    void FillData(PassData* _passData);
    
    Plane ExctractPlane(XMFLOAT3 _p0, XMFLOAT3 _p1, XMFLOAT3 _p2);
    void CalculateFrustum();
    
    friend class Device;
    friend class D3D12Device;
};

#endif
