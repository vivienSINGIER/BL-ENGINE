#include "CameraSystem.h"

#include "RessourceManager.h"
#include "../../Render/Generic/Render.h"
#include "../EngineManager.h"

void CameraSystem::OnUpdate(float _dt, EntityId _e, CameraComponent& _camera, TransformComponent& _transform)
{
    Camera* cam = RessourceManager::GetCamera(_camera.camId);
    if (cam == nullptr) return;
    
    Device* d = EngineManager::GetInstance().GetDevice();
    
    cam->SetWorld(_transform.world.GetMatrix());
    if (_camera.isMainCamera)
    {
        d->SetMainCamera(cam);
    }   
}
