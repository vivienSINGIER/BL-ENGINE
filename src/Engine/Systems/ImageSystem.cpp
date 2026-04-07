#include "ImageSystem.h"

#include "RessourceManager.h"

void Image::OnUpdate(float _dt, EntityId _e, UiImageComponent& _image)
{
    Sprite* sp = RessourceManager::GetSprite(_image.spriteId);
    Material* mat = RessourceManager::GetMaterial(_image.materialId);

    XMFLOAT4X4 t = _image.transform.GetMatrix();
}
