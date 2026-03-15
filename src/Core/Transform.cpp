#ifndef TRANSFORM_CPP_DEFINED
#define TRANSFORM_CPP_DEFINED

#include "Transform.h"


Transform::Transform()
{
    dirty = 0;
    SetIdentity();
    ResetWorldRotation();
    ResetLocalRotation();
}

Transform::Transform(const Transform& other)
        : localPos(other.localPos),
          worldPos(other.worldPos),  
          localScale(other.localScale),
          worldScale(other.worldScale),
          mForward(other.mForward),
          mUp(other.mUp),
          mRight(other.mRight),
          localQuat(other.localQuat),
          worldQuat(other.worldQuat),
          localRot(other.localRot),
          worldRot(other.worldRot),
          worldMatrix(other.worldMatrix),
          invMatrix(other.invMatrix),
          dirty(other.dirty)
{
}

Transform::Transform(Transform&& other) noexcept
    : localPos(std::move(other.localPos)),
      worldPos(std::move(other.worldPos)),  
      localScale(std::move(other.localScale)),
      worldScale(std::move(other.worldScale)),  
      mForward(std::move(other.mForward)),
      mUp(std::move(other.mUp)),
      mRight(std::move(other.mRight)),
      localQuat(std::move(other.localQuat)),
      worldQuat(std::move(other.worldQuat)),
      localRot(std::move(other.localRot)),
      worldRot(std::move(other.worldRot)),
      worldMatrix(std::move(other.worldMatrix)),
      invMatrix(std::move(other.invMatrix)),
      dirty(other.dirty)
{
}

Transform& Transform::operator=(const Transform& other)
{
    if (this == &other)
        return *this;
    localPos = other.localPos;
    worldPos = other.worldPos;
    localScale = other.localScale;
    worldScale = other.worldScale;
    mForward = other.mForward;
    mUp = other.mUp;
    mRight = other.mRight;
    localQuat = other.localQuat;
    worldQuat = other.worldQuat;
    localRot = other.localRot;
    worldRot = other.worldRot;
    worldMatrix = other.worldMatrix;
    invMatrix = other.invMatrix;
    dirty = other.dirty;
    return *this;
}

Transform& Transform::operator=(Transform&& other) noexcept
{
    if (this == &other)
        return *this;
    localPos  = std::move(other.localPos);
    worldPos  = std::move(other.worldPos);  
    localScale = std::move(other.localScale);
    worldScale = std::move(other.worldScale);
    mForward  = std::move(other.mForward);
    mUp       = std::move(other.mUp);
    mRight    = std::move(other.mRight);
    localQuat = std::move(other.localQuat);
    worldQuat = std::move(other.worldQuat); 
    localRot  = std::move(other.localRot);
    worldRot  = std::move(other.worldRot);  
    worldMatrix = std::move(other.worldMatrix);
    invMatrix = std::move(other.invMatrix);
    dirty     = other.dirty;
    return *this;
}

XMFLOAT4X4& Transform::GetWorldMatrix()
{
    if (dirty & WORLD)
        UpdateWorldMatrix();
    
    return worldMatrix;
}

XMFLOAT4X4& Transform::GetInvMatrix()
{
    if (dirty & INVERSE)
        UpdateInvMatrix();
    
    return invMatrix;
}

void Transform::SetIdentity()
{
    localPos    = XMFLOAT3(0.0f, 0.0f, 0.0f);
    localScale  = XMFLOAT3(1.0f, 1.0f, 1.0f);
    localQuat   = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    worldPos    = XMFLOAT3(0.0f, 0.0f, 0.0f);
    worldScale  = XMFLOAT3(1.0f, 1.0f, 1.0f);
    worldQuat   = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
    XMStoreFloat4x4(&invMatrix, XMMatrixIdentity());
    ResetLocalRotation();
    ResetWorldRotation();
}

void Transform::UpdateWorldMatrix()
{
    XMVECTOR p = XMLoadFloat3(&worldPos);
    XMVECTOR s = XMLoadFloat3(&worldScale);
    XMVECTOR r = XMLoadFloat4(&worldQuat);

    XMMATRIX m = XMMatrixAffineTransformation(s, XMVectorZero(), r, p);
    XMStoreFloat4x4(&worldMatrix, m);

    dirty &= ~WORLD;
}

void Transform::UpdateInvMatrix()
{
    if (dirty & WORLD)
        UpdateWorldMatrix();
    
    XMMATRIX m = XMLoadFloat4x4(&worldMatrix);
    XMStoreFloat4x4(&invMatrix, XMMatrixInverse(nullptr, m));

    dirty &= ~INVERSE;
}


///////////////////////////////////////////////////////////////////////////////////////
/// POSITION ///
///////////////////////////////////////////////////////////////////////////////////////

void Transform::SetLocalPosition(XMFLOAT3 const& position)
{
    localPos = position;
    dirty |= LOCAL_POS;
}

void Transform::SetWorldPosition(XMFLOAT3 const& position)
{
    worldPos = position;
    dirty |= WORLD_POS;
}

void Transform::MoveLocal(XMFLOAT3 const& delta)
{
    XMVECTOR p = XMLoadFloat3(&localPos);

    XMVECTOR offset = XMVectorZero();
    offset = XMVectorAdd(offset, XMVectorScale(XMLoadFloat3(&mRight),   delta.x));
    offset = XMVectorAdd(offset, XMVectorScale(XMLoadFloat3(&mUp),      delta.y));
    offset = XMVectorAdd(offset, XMVectorScale(XMLoadFloat3(&mForward), delta.z));

    p = XMVectorAdd(p, offset);
    XMStoreFloat3(&localPos, p);
    
    dirty |= LOCAL_POS;
}

void Transform::MoveLocal(XMFLOAT3 const& dir, float distance)
{
    XMVECTOR p      = XMLoadFloat3(&localPos);
    XMVECTOR d      = XMLoadFloat3(&dir);
    XMVECTOR scalar = XMVectorReplicate(distance);

    p = XMVectorAdd(p, XMVectorMultiply(d, scalar));
    XMStoreFloat3(&localPos, p);

    dirty |= LOCAL_POS;
}

void Transform::MoveWorld(XMFLOAT3 const& delta)
{
    XMVECTOR p = XMLoadFloat3(&worldPos);
    p = XMVectorAdd(p, XMLoadFloat3(&delta));
    XMStoreFloat3(&worldPos, p);
    dirty |= WORLD_POS;
}

void Transform::MoveWorld(XMFLOAT3 const& dir, float distance)
{
    XMVECTOR p      = XMLoadFloat3(&worldPos);
    XMVECTOR d      = XMLoadFloat3(&dir);
    XMVECTOR scalar = XMVectorReplicate(distance);

    p = XMVectorAdd(p, XMVectorMultiply(d, scalar));
    XMStoreFloat3(&worldPos, p);
    dirty |= WORLD_POS;
}

///////////////////////////////////////////////////////////////////////////////////////
/// SCALE ///
///////////////////////////////////////////////////////////////////////////////////////

void Transform::SetLocalScale(XMFLOAT3 const& _scale)
{
    localScale = _scale;
    worldScale = _scale;
    dirty |= LOCAL_SCALE;
}

void Transform::SetLocalScale(float _scale)
{
    XMStoreFloat3(&localScale, XMVectorReplicate(_scale));
    worldScale = localScale;
    dirty |= LOCAL_SCALE;
}

void Transform::ScaleLocal(XMFLOAT3 const& _scale)
{
    XMVECTOR s = XMVectorMultiply(XMLoadFloat3(&localScale), XMLoadFloat3(&_scale));
    XMStoreFloat3(&localScale, s);
    worldScale = localScale;
    dirty |= LOCAL_SCALE;
}

void Transform::ScaleLocal(float _scale)
{
    XMVECTOR s = XMVectorMultiply(XMLoadFloat3(&localScale), XMVectorReplicate(_scale));
    XMStoreFloat3(&localScale, s);
    worldScale = localScale;
    dirty |= LOCAL_SCALE;
}

void Transform::SetWorldScale(XMFLOAT3 const& _scale)
{
    worldScale = _scale;
    dirty |= WORLD_SCALE;
}

void Transform::SetWorldScale(float _scale)
{
    XMStoreFloat3(&worldScale, XMVectorReplicate(_scale));
    dirty |= WORLD_SCALE;
}

void Transform::ScaleWorld(XMFLOAT3 const& _scale)
{
    XMVECTOR s = XMVectorMultiply(XMLoadFloat3(&worldScale), XMLoadFloat3(&_scale));
    XMStoreFloat3(&worldScale, s);
    dirty |= WORLD_SCALE;
}

void Transform::ScaleWorld(float _scale)
{
    XMVECTOR s = XMVectorMultiply(XMLoadFloat3(&worldScale), XMVectorReplicate(_scale));
    XMStoreFloat3(&worldScale, s);
    dirty |= WORLD_SCALE;
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
    XMVECTOR forward = XMVector3Normalize(XMLoadFloat3(&dir));
    XMVECTOR up      = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    
    XMVECTOR right  = XMVector3Normalize(XMVector3Cross(up, forward));
    XMVECTOR newUp  = XMVector3Cross(forward, right);
    
    XMMATRIX rotationMatrix;
    rotationMatrix.r[0] = XMVectorSetW(right,   0.0f);
    rotationMatrix.r[1] = XMVectorSetW(newUp,   0.0f);
    rotationMatrix.r[2] = XMVectorSetW(forward, 0.0f);
    rotationMatrix.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    
    XMVECTOR quat = XMQuaternionRotationMatrix(rotationMatrix);
    
    XMStoreFloat4(&worldQuat, quat);
    XMStoreFloat4(&localQuat, quat);
    
    XMVECTOR scaleVec = XMLoadFloat3(&worldScale);
    XMVECTOR posVec   = XMLoadFloat3(&worldPos);
    XMMATRIX worldMat = XMMatrixAffineTransformation(scaleVec, XMVectorZero(), quat, posVec);
    XMStoreFloat4x4(&worldMatrix, worldMat);
    
    XMStoreFloat4x4(&worldRot, rotationMatrix);
    XMStoreFloat4x4(&localRot, rotationMatrix);
    
    ExtractAxesFromMatrix(worldRot, mRight, mUp, mForward);/*
    ExtractAxesFromMatrix(worldRot, worldRight, worldUp, worldForward);*/
    
    dirty |= WORLD_ROTATE;
}

void Transform::LookToCamera(XMFLOAT3 const& dir)
{
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX m  = XMMatrixLookToLH(XMLoadFloat3(&worldPos), XMLoadFloat3(&dir), up);
    XMStoreFloat4x4(&worldMatrix, m);
    UpdateLocalRotationFromMatrix();

    dirty |= WORLD_ROTATE;
}

void Transform::LookAt(XMFLOAT3 const& target)
{
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX m  = XMMatrixLookAtLH(XMLoadFloat3(&worldPos), XMLoadFloat3(&target), up);
    XMStoreFloat4x4(&worldMatrix, m);
    UpdateLocalRotationFromMatrix();

    dirty |= WORLD_ROTATE;
}

void Transform::SetLocalRotationMatrix(XMFLOAT4X4 const& rotation)
{
    localRot = rotation;
    UpdateLocalRotationFromMatrix();

    dirty |= LOCAL_ROTATE;
}

void Transform::SetLocalRotationQuaternion(XMFLOAT4 const& _quat)
{
    localQuat = _quat;
    UpdateLocalRotationFromQuaternion();

    dirty |= LOCAL_ROTATE;
}

void Transform::ResetLocalRotation()
{
    mRight   = XMFLOAT3(1.0f, 0.0f, 0.0f);
    mUp      = XMFLOAT3(0.0f, 1.0f, 0.0f);
    mForward = XMFLOAT3(0.0f, 0.0f, 1.0f);

    XMStoreFloat4x4(&localRot,  XMMatrixIdentity());
    XMStoreFloat4  (&localQuat, XMQuaternionIdentity());

    dirty |= LOCAL_ROTATE;
}

void Transform::ResetWorldRotation()
{
    XMStoreFloat4x4(&worldRot,  XMMatrixIdentity());
    XMStoreFloat4  (&worldQuat, XMQuaternionIdentity());

    dirty |= WORLD_ROTATE;
}

void Transform::SetLocalYPR(XMFLOAT3 const& ypr)
{
    ResetLocalRotation();
    AddLocalYPR(ypr);
}

void Transform::AddLocalYPR(const XMFLOAT3& ypr)
{
    XMVECTOR qRot = XMLoadFloat4(&localQuat);

    if (ypr.x != 0.0f) // yaw (Y world axis)
    {
        XMVECTOR qYaw = XMQuaternionRotationAxis(XMVectorSet(0,1,0,0), ypr.x);
        qRot = XMQuaternionMultiply(qRot, qYaw);
    }

    if (ypr.y != 0.0f) // pitch (local X axis)
    {
        XMVECTOR right  = XMVector3Rotate(XMVectorSet(1,0,0,0), qRot);
        XMVECTOR qPitch = XMQuaternionRotationAxis(right, ypr.y);
        qRot = XMQuaternionMultiply(qPitch, qRot);
    }

    if (ypr.z != 0.0f) // roll (local Z axis)
    {
        XMVECTOR qRoll = XMQuaternionRotationAxis(XMVectorSet(0,0,1,0), ypr.z);
        qRot = XMQuaternionMultiply(qRot, qRoll);
    }

    qRot = XMQuaternionNormalize(qRot);
    XMStoreFloat4(&localQuat, qRot);

    UpdateLocalRotationFromQuaternion();
}

void Transform::SetWorldYPR(XMFLOAT3 const& ypr)
{
    ResetWorldRotation();
    AddWorldYPR(ypr);
}

void Transform::AddWorldYPR(XMFLOAT3 const& ypr)
{
    XMVECTOR qRot = XMLoadFloat4(&worldQuat);

    if (ypr.x != 0.0f) // yaw (Y world axis)
    {
        XMVECTOR qYaw = XMQuaternionRotationAxis(XMVectorSet(0,1,0,0), ypr.x);
        qRot = XMQuaternionMultiply(qRot, qYaw);
    }

    if (ypr.y != 0.0f) // pitch (local X axis)
    {
        XMVECTOR right  = XMVector3Rotate(XMVectorSet(1,0,0,0), qRot);
        XMVECTOR qPitch = XMQuaternionRotationAxis(right, ypr.y);
        qRot = XMQuaternionMultiply(qPitch, qRot);
    }

    if (ypr.z != 0.0f) // roll (local Z axis)
    {
        XMVECTOR qRoll = XMQuaternionRotationAxis(XMVectorSet(0,0,1,0), ypr.z);
        qRot = XMQuaternionMultiply(qRot, qRoll);
    }

    qRot = XMQuaternionNormalize(qRot);
    XMStoreFloat4(&worldQuat, qRot);

    UpdateWorldRotationFromQuaternion();
}

void Transform::UpdateLocalRotationFromAxes()
{
    localRot._11 = mRight.x;   localRot._12 = mRight.y;   localRot._13 = mRight.z;
    localRot._21 = mUp.x;      localRot._22 = mUp.y;      localRot._23 = mUp.z;
    localRot._31 = mForward.x; localRot._32 = mForward.y; localRot._33 = mForward.z;
    localRot._14 = 0.0f; localRot._24 = 0.0f; localRot._34 = 0.0f;
    localRot._41 = 0.0f; localRot._42 = 0.0f; localRot._43 = 0.0f; localRot._44 = 1.0f;

    XMStoreFloat4(&localQuat, XMQuaternionRotationMatrix(XMLoadFloat4x4(&localRot)));

    dirty |= LOCAL_ROTATE;
}

void Transform::UpdateLocalRotationFromQuaternion()
{
    XMStoreFloat4x4(&localRot, XMMatrixRotationQuaternion(XMLoadFloat4(&localQuat)));

    //ExtractAxesFromMatrix(worldRot, mRight, mUp, mForward);

    dirty |= LOCAL_ROTATE;
}

void Transform::UpdateLocalRotationFromMatrix()
{
    XMMATRIX worldMat = XMLoadFloat4x4(&worldMatrix);
    
    XMVECTOR scale, rotQuat, trans;
    XMMatrixDecompose(&scale, &rotQuat, &trans, worldMat);
    
    XMStoreFloat4(&worldQuat, rotQuat);
    XMStoreFloat4(&localQuat, rotQuat);
    XMStoreFloat4x4(&localRot, XMMatrixRotationQuaternion(rotQuat));

    worldRot = localRot;

    ExtractAxesFromMatrix(worldRot, mRight, mUp, mForward);

    /*
    ExtractAxesFromMatrix(worldRot, worldRight, worldUp, worldForward);*/

    dirty |= LOCAL_ROTATE;
}

void Transform::UpdateWorldRotationFromAxes()
{
    worldRot._11 = mRight.x;   worldRot._12 = mRight.y;   worldRot._13 = mRight.z;
    worldRot._21 = mUp.x;      worldRot._22 = mUp.y;      worldRot._23 = mUp.z;
    worldRot._31 = mForward.x; worldRot._32 = mForward.y; worldRot._33 = mForward.z;
    worldRot._14 = 0.0f; worldRot._24 = 0.0f; worldRot._34 = 0.0f;
    worldRot._41 = 0.0f; worldRot._42 = 0.0f; worldRot._43 = 0.0f; worldRot._44 = 1.0f;

    XMStoreFloat4(&worldQuat, XMQuaternionRotationMatrix(XMLoadFloat4x4(&worldRot)));

    dirty |= WORLD_ROTATE;
}

void Transform::UpdateWorldRotationFromQuaternion()
{
    XMStoreFloat4x4(&worldRot, XMMatrixRotationQuaternion(XMLoadFloat4(&worldQuat)));

    ExtractAxesFromMatrix(worldRot, mRight, mUp, mForward);

    dirty |= WORLD_ROTATE;
}

void Transform::UpdateWorldRotationFromMatrix()
{
    XMStoreFloat4(&worldQuat, XMQuaternionRotationMatrix(XMLoadFloat4x4(&worldRot)));

    ExtractAxesFromMatrix(worldRot, mRight, mUp, mForward);

    dirty |= WORLD_ROTATE;
}

///////////////////////////////////////////////////////////////////////////////////////

#endif