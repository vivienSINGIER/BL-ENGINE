#include "TextSystem.h"

#include "EngineManager.h"
#include "RessourceManager.h"

void TextSystem::OnUpdate(float _dt, EntityId _e, TextComponent& _t)
{
    Text* txt = RessourceManager::GetText(_t.textId);
    if (txt == nullptr) return;

    Mat4f32 t = _t.transform.GetMatrix();
    
    Device* pDevice = EngineManager::GetDevice();
    if (pDevice == nullptr) return;
    
    pDevice->DrawRenderText(txt, t);
}
