#include "ImageSystem.h"

#include "EngineManager.h"
#include "RessourceManager.h"

void ImageSystem::OnStartUpdate(float _dt)
{
    int o = 0;
}

void ImageSystem::OnUpdate(float _dt, EntityId _e, UiImageComponent& _image)
{
    Sprite* sp = RessourceManager::GetSprite(_image.spriteId);
    UiMaterial* uiMat = RessourceManager::GetUiMaterial(_image.materialId);

    Mat4f32 t = _image.transform.GetMatrix();
    
    if (sp == nullptr) return;
    if (uiMat == nullptr) return;
    
    Device* pDevice = EngineManager::GetDevice();
    if (pDevice == nullptr) return;
    
    pDevice->SetUiMaterial(uiMat);
    pDevice->DrawUi(sp, t);
}
