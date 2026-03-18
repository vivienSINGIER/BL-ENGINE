#include "CameraSystem.h"

#include "../../Render/Generic/Render.h"
#include "../EngineManager.h"

void CameraSystem::OnUpdate(float _dt, CameraComponent& _camera, TransformComponent& _transform)
{
    if (_camera.camera == nullptr) return;
    
    Device* d = EngineManager::GetInstance().GetDevice();

    if ((_transform.world.dirty & WORLD) == WORLD)
    {
        _camera.camera->SetWorld(_transform.world.GetMatrix());
        if (_camera.isMainCamera)
        {
            d->SetMainCamera(_camera.camera);
            _transform.world.dirty &= ~WORLD;
        }   
    }
}
