#include "TransformSystem.h"


bool TransformSystem::IsDirty(Transform& _transform, uint32 _flag)
{
    uint32 dirty = _transform.GetDirty();
    return (dirty & _flag);
}

void TransformSystem::UpdateMatrix(TransformComponent& _t)
{
    if (_t.hasParent == true)
    {
        TransformComponent& p = world->GetComponent<TransformComponent>(_t.parent);
        UpdateMatrix(p);
        
        if (IsDirty(_t.local, POS | SCALE | ROTATE) || IsDirty(p.local, WORLD))
        {
            _t.world = _t.local;
            _t.world.UpdateFromParent(p.world);
            _t.local.dirty = WORLD;
        }
    }
    else if (IsDirty(_t.local, POS | SCALE | ROTATE))
    {
        _t.world = _t.local;
        _t.world.UpdateMatrix();
        _t.local.dirty = WORLD;
    }
}

void TransformSystem::OnUpdate(float _dt, TransformComponent& _t)
{
    UpdateMatrix(_t);
}
