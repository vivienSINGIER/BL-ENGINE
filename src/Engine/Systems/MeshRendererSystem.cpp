#include "MeshRendererSystem.h"

#include "EngineManager.h"
#include "RessourceManager.h"
#include "../Components/MeshRenderer.hpp"

void MeshRendererSystem::OnUpdate(float _dt, MeshRenderer& _mesh, TransformComponent& _transform)
{
    Device* pDevice = EngineManager::GetDevice();

    if (_mesh.geo == nullptr) return;
    
    Material* mat = _mesh.material;

    if (mat == nullptr)
        mat = RessourceManager::GetMaterial("Default");
    if (mat == nullptr)
        return;

    pDevice->SetMaterial(mat);
    pDevice->Draw(_mesh.geo, _transform.transform.GetWorldMatrix());
}
