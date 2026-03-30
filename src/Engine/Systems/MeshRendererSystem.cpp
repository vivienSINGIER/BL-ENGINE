#include "MeshRendererSystem.h"

#include "EngineManager.h"
#include "RessourceManager.h"
#include "../Components/MeshRenderer.hpp"

void MeshRendererSystem::OnUpdate(float _dt, EntityId _e, MeshRenderer& _mesh, TransformComponent& _transform)
{
    Device* pDevice = EngineManager::GetDevice();

    Geometry* geo = RessourceManager::GetGeometry(_mesh.geoId);
    Material* mat = RessourceManager::GetMaterial(_mesh.materialId);
    
    if (mat == nullptr || geo == nullptr) return;

    pDevice->SetMaterial(mat);
    pDevice->Draw(geo, _transform.world.GetMatrix());
}
