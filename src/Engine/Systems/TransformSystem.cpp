#include "TransformSystem.h"
#include "../ECS/World.h"

void TransformSystem::UpdateMatrix(TransformComponent& _t, EntityId _e)
{
    if (_t.hasParent == true)
    {
        TransformComponent& p = world->GetComponent<TransformComponent>(_t.parent);
        UpdateMatrix(p, _e);
        
        // if (IsDirty(_t.local, (uint8)DIRTY_FLAG::POS | (uint8)DIRTY_FLAG::SCALE | (uint8)DIRTY_FLAG::ROTATE) || IsDirty(p.local, (uint8)DIRTY_FLAG::WORLD))
        // {
        //     _t.world = _t.local;
        //     _t.world.UpdateFromParent(p.world);
        //     _t.local.dirty = (uint8)DIRTY_FLAG::WORLD;
        //     SetNetworkDirty<TransformComponent>(_e);
        // }
    }
    // else if (IsDirty(_t.local, (uint8)DIRTY_FLAG::POS | (uint8)DIRTY_FLAG::SCALE | (uint8)DIRTY_FLAG::ROTATE))
    // {
    //     _t.world = _t.local;
    //     _t.world.UpdateMatrix();
    //     _t.local.dirty = (uint8)DIRTY_FLAG::WORLD;
    //     SetNetworkDirty<TransformComponent>(_e);
    // }
}

void TransformSystem::OnUpdate(float _dt, EntityId _e, TransformComponent& _t)
{
    UpdateMatrix(_t, _e);
}
