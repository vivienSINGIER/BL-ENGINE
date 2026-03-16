#include "TransformSystem.h"


bool TransformSystem::IsDirty(Transform& _transform, DIRTY_FLAG _flag)
{
    uint8 dirty = _transform.GetDirty();
    return (_transform.GetDirty() & _flag) == _flag;
}

void TransformSystem::OnUpdate(float _dt, TransformComponent& _t)
{
    bool isDirty = _t.transform.GetDirty();
    
    if (IsDirty(_t.transform, WORLD_POS))
    {
        _t.transform.localPos = _t.transform.worldPos;
    }
    else if (IsDirty(_t.transform, LOCAL_POS))
    {
        _t.transform.worldPos = _t.transform.localPos;
    }

    if (IsDirty(_t.transform, WORLD_SCALE))
    {
        _t.transform.localScale = _t.transform.worldScale;
    }
    else if (IsDirty(_t.transform, LOCAL_SCALE))
    {
        _t.transform.worldScale = _t.transform.localScale;
    }

    if (IsDirty(_t.transform, WORLD_ROTATE))
    {
        _t.transform.localQuat = _t.transform.worldQuat;
        _t.transform.UpdateLocalRotationFromQuaternion();
    }
    else if (IsDirty(_t.transform, LOCAL_ROTATE))
    {
        _t.transform.worldQuat = _t.transform.localQuat;
        _t.transform.UpdateWorldRotationFromQuaternion();
    }

    if (isDirty)
    {
        _t.transform.UpdateWorldMatrix();
        _t.transform.dirty = WORLD | INVERSE;
    }

    int o = 0;
}
