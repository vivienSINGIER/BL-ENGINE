#ifndef TRANSFORM_CPP_DEFINED
#define TRANSFORM_CPP_DEFINED

#include "Transform.h"


Transform::Transform()
{
    dirty = 0;
    SetIdentity();
    ResetRotation();
}

Transform::Transform(const Transform& other)
        : pos(other.pos),
          scale(other.scale),
          forward(other.forward),
          up(other.up),
          right(other.right),
          quat(other.quat),
          rotMatrix(other.rotMatrix),
          matrix(other.matrix),
          invMatrix(other.invMatrix),
          dirty(other.dirty)
{
}

Transform::Transform(Transform&& other) noexcept
    : pos(std::move(other.pos)),
      scale(std::move(other.scale)),
      forward(std::move(other.forward)),
      up(std::move(other.up)),
      right(std::move(other.right)),
      quat(std::move(other.quat)),
      rotMatrix(std::move(other.rotMatrix)),
      matrix(std::move(other.matrix)),
      invMatrix(std::move(other.invMatrix)),
      dirty(other.dirty)
{
}

Transform& Transform::operator=(const Transform& other)
{
    if (this == &other)
        return *this;
    pos = other.pos;
    scale = other.scale;
    forward = other.forward;
    up = other.up;
    right = other.right;
    quat = other.quat;
    rotMatrix = other.rotMatrix;
    matrix = other.matrix;
    invMatrix = other.invMatrix;
    dirty = other.dirty;
    return *this;
}

Transform& Transform::operator=(Transform&& other) noexcept
{
    if (this == &other)
        return *this;
    pos  = std::move(other.pos);
    scale = std::move(other.scale);
    forward  = std::move(other.forward);
    up       = std::move(other.up);
    right    = std::move(other.right);
    quat = std::move(other.quat);
    rotMatrix  = std::move(other.rotMatrix);
    matrix = std::move(other.matrix);
    invMatrix = std::move(other.invMatrix);
    dirty     = other.dirty;
    return *this;
}

XMFLOAT4X4& Transform::GetMatrix()
{
    if (dirty & WORLD)
        UpdateMatrix();
    
    return matrix;
}

XMFLOAT4X4& Transform::GetInvMatrix()
{
    if (dirty & INVERSE)
        UpdateInvMatrix();
    
    return invMatrix;
}

void Transform::SetIdentity()
{
    pos    = XMFLOAT3(0.0f, 0.0f, 0.0f);
    scale  = XMFLOAT3(1.0f, 1.0f, 1.0f);
    quat   = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    ResetRotation();
    XMStoreFloat4x4(&matrix, XMMatrixIdentity());
    XMStoreFloat4x4(&invMatrix, XMMatrixIdentity());
}

void Transform::UpdateMatrix()
{
    XMVECTOR p = XMLoadFloat3(&pos);
    XMVECTOR s = XMLoadFloat3(&scale);
    XMVECTOR r = XMLoadFloat4(&quat);

    XMMATRIX m = XMMatrixAffineTransformation(s, XMVectorZero(), r, p);
    XMStoreFloat4x4(&matrix, m);

    dirty &= ~WORLD;
    dirty &= ~POS;
    dirty &= ~SCALE;
    dirty &= ~ROTATE;
    
    dirty |= INVERSE;
}

void Transform::UpdateInvMatrix()
{
    if (dirty & WORLD)
        UpdateMatrix();
    
    XMMATRIX m = XMLoadFloat4x4(&matrix);
    XMStoreFloat4x4(&invMatrix, XMMatrixInverse(nullptr, m));

    dirty &= ~INVERSE;
}

void Transform::UpdateFromParent(Transform const& parent)
{
    XMVECTOR parentScale = XMLoadFloat3(&parent.scale);
    XMVECTOR localScale  = XMLoadFloat3(&scale);
    XMVECTOR worldScale  = XMVectorMultiply(parentScale, localScale);
    XMStoreFloat3(&scale, worldScale);
    
    XMVECTOR parentQuat = XMLoadFloat4(&parent.quat);
    XMVECTOR localQuat  = XMLoadFloat4(&quat);
    XMVECTOR worldQuat  = XMQuaternionMultiply(localQuat, parentQuat);
    worldQuat           = XMQuaternionNormalize(worldQuat);
    XMStoreFloat4(&quat, worldQuat);
    
    XMVECTOR localPos  = XMLoadFloat3(&pos);
    XMVECTOR parentPos = XMLoadFloat3(&parent.pos);
    
    localPos = XMVectorMultiply(localPos, parentScale);
    
    localPos = XMVector3Rotate(localPos, parentQuat);
    
    XMVECTOR worldPos = XMVectorAdd(localPos, parentPos);
    XMStoreFloat3(&pos, worldPos);
    
    UpdateRotationFromQuaternion();
    dirty |= WORLD | INVERSE;
}

///////////////////////////////////////////////////////////////////////////////////////
/// POSITION ///
///////////////////////////////////////////////////////////////////////////////////////

void Transform::SetPosition(XMFLOAT3 const& position)
{
    pos = position;
}

void Transform::Move(XMFLOAT3 const& delta)
{
    pos.x += delta.x;
    pos.y += delta.y;
    pos.z += delta.z;
    
    dirty |= POS;
}

void Transform::Move(XMFLOAT3 const& dir, float distance)
{
    pos.x += dir.x * distance;
    pos.y += dir.y * distance;
    pos.z += dir.z * distance;

    dirty |= POS;
}

///////////////////////////////////////////////////////////////////////////////////////
/// SCALE ///
///////////////////////////////////////////////////////////////////////////////////////

void Transform::SetScale(XMFLOAT3 const& _scale)
{
    scale = _scale;
    dirty |= SCALE;
}

void Transform::SetScale(float _scale)
{
    XMStoreFloat3(&scale, XMVectorReplicate(_scale));
    dirty |= SCALE;
}

void Transform::Scale(XMFLOAT3 const& _scale)
{
    XMVECTOR s = XMVectorMultiply(XMLoadFloat3(&scale), XMLoadFloat3(&_scale));
    XMStoreFloat3(&scale, s);
    dirty |= SCALE;
}

void Transform::Scale(float _scale)
{
    XMVECTOR s = XMVectorMultiply(XMLoadFloat3(&scale), XMVectorReplicate(_scale));
    XMStoreFloat3(&scale, s);
    dirty |= SCALE;
}

///////////////////////////////////////////////////////////////////////////////////////
/// ROTATION ///
///////////////////////////////////////////////////////////////////////////////////////

static void ExtractAxesFromMatrix(const XMFLOAT4X4& m,
                                  XMFLOAT3& right,
                                  XMFLOAT3& up,
                                  XMFLOAT3& forward)
{
    right   = XMFLOAT3(m._11, m._12, m._13);
    up      = XMFLOAT3(m._21, m._22, m._23);
    forward = XMFLOAT3(m._31, m._32, m._33);
}

void Transform::LookTo(XMFLOAT3 const& dir)
{
    // XMVECTOR forward = XMVector3Normalize(XMLoadFloat3(&dir));
    // XMVECTOR up      = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    //
    // XMVECTOR right  = XMVector3Normalize(XMVector3Cross(up, forward));
    // XMVECTOR newUp  = XMVector3Cross(forward, right);
    //
    // XMMATRIX rotationMatrix;
    // rotationMatrix.r[0] = XMVectorSetW(right,   0.0f);
    // rotationMatrix.r[1] = XMVectorSetW(newUp,   0.0f);
    // rotationMatrix.r[2] = XMVectorSetW(forward, 0.0f);
    // rotationMatrix.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    //
    // XMVECTOR quat = XMQuaternionRotationMatrix(rotationMatrix);
    //
    // XMStoreFloat4(&localQuat, quat);
    //
    // XMVECTOR scaleVec = XMLoadFloat3(&worldScale);
    // XMVECTOR posVec   = XMLoadFloat3(&worldPos);
    // XMMATRIX worldMat = XMMatrixAffineTransformation(scaleVec, XMVectorZero(), quat, posVec);
    // XMStoreFloat4x4(&worldMatrix, worldMat);
    //
    // XMStoreFloat4x4(&worldRot, rotationMatrix);
    // XMStoreFloat4x4(&localRot, rotationMatrix);
    //
    // ExtractAxesFromMatrix(worldRot, mRight, mUp, mForward);/*
    // ExtractAxesFromMatrix(worldRot, worldRight, worldUp, worldForward);*/
    //
    // dirty |= WORLD_ROTATE;
}

void Transform::LookToCamera(XMFLOAT3 const& dir)
{
    // XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    // XMMATRIX m  = XMMatrixLookToLH(XMLoadFloat3(&worldPos), XMLoadFloat3(&dir), up);
    // XMStoreFloat4x4(&worldMatrix, m);
    // UpdateLocalRotationFromMatrix();
    //
    // dirty |= WORLD_ROTATE;
}

void Transform::LookAt(XMFLOAT3 const& target)
{
    // XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    // XMMATRIX m  = XMMatrixLookAtLH(XMLoadFloat3(&worldPos), XMLoadFloat3(&target), up);
    // XMStoreFloat4x4(&worldMatrix, m);
    // UpdateLocalRotationFromMatrix();
    //
    // dirty |= WORLD_ROTATE;
}

void Transform::SetRotationMatrix(XMFLOAT4X4 const& rotation)
{
    rotMatrix = rotation;
    UpdateRotationFromMatrix();

    dirty |= ROTATE;
}

void Transform::SetRotationQuaternion(XMFLOAT4 const& _quat)
{
    quat = _quat;
    UpdateRotationFromQuaternion();

    dirty |= ROTATE;
}

void Transform::ResetRotation()
{
    right   = XMFLOAT3(1.0f, 0.0f, 0.0f);
    up      = XMFLOAT3(0.0f, 1.0f, 0.0f);
    forward = XMFLOAT3(0.0f, 0.0f, 1.0f);

    XMStoreFloat4x4(&rotMatrix,  XMMatrixIdentity());
    XMStoreFloat4  (&quat, XMQuaternionIdentity());

    dirty |= ROTATE;
}

void Transform::SetYPR(XMFLOAT3 const& ypr)
{
    ResetRotation();
    AddYPR(ypr);
}

void Transform::AddYPR(const XMFLOAT3& ypr)
{
    XMVECTOR qRot = XMLoadFloat4(&quat);

    if (ypr.x != 0.0f) // yaw (Y world axis)
    {
        XMVECTOR qYaw = XMQuaternionRotationAxis(XMVectorSet(0,1,0,0), ypr.x);
        qRot = XMQuaternionMultiply(qRot, qYaw);
    }

    if (ypr.y != 0.0f) // pitch (local X axis)
    {
        XMVECTOR right  = XMVector3Rotate(XMVectorSet(1,0,0,0), qRot);
        XMVECTOR qPitch = XMQuaternionRotationAxis(right, ypr.y);
        qRot = XMQuaternionMultiply(qRot, qPitch);
    }

    if (ypr.z != 0.0f) // roll (local Z axis)
    {
        XMVECTOR qRoll = XMQuaternionRotationAxis(XMVectorSet(0,0,1,0), ypr.z);
        qRot = XMQuaternionMultiply(qRot, qRoll);
    }

    qRot = XMQuaternionNormalize(qRot);
    XMStoreFloat4(&quat, qRot);

    UpdateRotationFromQuaternion();
}

void Transform::UpdateRotationFromAxes()
{
    rotMatrix._11 = right.x;   rotMatrix._12 = right.y;   rotMatrix._13 = right.z;
    rotMatrix._21 = up.x;      rotMatrix._22 = up.y;      rotMatrix._23 = up.z;
    rotMatrix._31 = forward.x; rotMatrix._32 = forward.y; rotMatrix._33 = forward.z;
    rotMatrix._14 = 0.0f; rotMatrix._24 = 0.0f; rotMatrix._34 = 0.0f;
    rotMatrix._41 = 0.0f; rotMatrix._42 = 0.0f; rotMatrix._43 = 0.0f; rotMatrix._44 = 1.0f;

    XMStoreFloat4(&quat, XMQuaternionRotationMatrix(XMLoadFloat4x4(&rotMatrix)));

    dirty |= ROTATE;
}

void Transform::UpdateRotationFromQuaternion()
{
    XMStoreFloat4x4(&rotMatrix, XMMatrixRotationQuaternion(XMLoadFloat4(&quat)));
    ExtractAxesFromMatrix(rotMatrix, right, up, forward);
    dirty |= ROTATE;
}

void Transform::UpdateRotationFromMatrix()
{
    XMMATRIX worldMat = XMLoadFloat4x4(&matrix);
    
    XMVECTOR scale, rotQuat, trans;
    XMMatrixDecompose(&scale, &rotQuat, &trans, worldMat);
    
    XMStoreFloat4(&quat, rotQuat);
    XMStoreFloat4x4(&rotMatrix, XMMatrixRotationQuaternion(rotQuat));

    ExtractAxesFromMatrix(rotMatrix, right, up, forward);

    dirty |= ROTATE;
}

///////////////////////////////////////////////////////////////////////////////////////

#endif