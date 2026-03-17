#include "CameraSystem.h"

#include "../../Render/Generic/Render.h"
#include "../EngineManager.h"

void CameraSystem::OnUpdate(float _dt, CameraComponent& _camera, TransformComponent& _transform)
{
    Device* d = EngineManager::GetInstance().GetDevice();

    if ((_transform.world.GetDirty() & DIRTY_FLAG::INVERSE) == DIRTY_FLAG::INVERSE)
    {
        _camera.camera.SetWorld(_transform.world.GetInvMatrix());
        if (_camera.isMainCamera)
        {
            d->SetMainCamera(&_camera.camera);
        }
    }
}
