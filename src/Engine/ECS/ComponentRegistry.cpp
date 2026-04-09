#ifndef COMPONENT_REGISTRY_CPP_DEFINED
#define COMPONENT_REGISTRY_CPP_DEFINED

#include "../define.h"
#include "Script.h"

#include "../Components/TransformComponent.hpp"
#include "../Components/MeshRenderer.hpp"
#include "../Components/CameraComponent.hpp"
#include "../Components/LightComponent.hpp"
#include "../Components/ColliderComponent.hpp"
#include "../Components/PhysicComponent.hpp"
#include "../Components/NetworkComponent.hpp"
#include "../Components/OwnerComponent.hpp"
#include "../Components/UiImageComponent.hpp"
#include "../Components/UiButtonComponent.hpp"
#include "../Components/TextComponent.hpp"

void ComponentRegistry::Init()
{
    RegisterComponent<TransformComponent>();
    RegisterComponent<MeshRenderer>();
    RegisterComponent<CameraComponent>(true);
    RegisterComponent<LightComponent>();
    RegisterComponent<ColliderComponent>();
    RegisterComponent<PhysicComponent>();
    RegisterComponent<NetworkComponent>();
    RegisterComponent<ScriptRegistry>();
    RegisterComponent<OwnerComponent>();
    RegisterComponent<UiImageComponent>(true);
    RegisterComponent<UiButtonComponent>(true);
    RegisterComponent<TextComponent>(true);
}

IScript* ComponentRegistry::GetScript(ComponentId _id, EntityId _e, World&_w)
{
    for (auto& info : m_registeredComponents)
        if (info.id == _id && info.scriptAccessor)
            return info.scriptAccessor(_e, _w);
    return nullptr;
}

IScript* ComponentRegistry::ConstructScript(ComponentId _id, void* ptr)
{
    for (auto& info : m_registeredComponents)
    {
        if (info.id == _id)
        {
            return info.scriptConstructor(ptr);
        }
    }
    
    return nullptr;
}

bool ComponentRegistry::IsRegistered(ComponentId _id)
{
    for (auto& info : m_registeredComponents)
        if (info.id == _id)
            return true;
    return false;
}

bool ComponentRegistry::IsClientOnly(ComponentId _id)
{
    for (auto& info : m_registeredComponents)
        if (info.id == _id)
            return info.isClientOnly;
    return false;
}

bool ComponentRegistry::IsScript(ComponentId _id)
{
    for (auto& info : m_registeredComponents)
        if (info.id == _id)
            return info.isScript;
    return false;
}

uint32 ComponentRegistry::GetSize(ComponentId _id)
{
    for (auto& info : m_registeredComponents)
        if (info.id == _id)
            return info.size;
    return 0;
}

uint8 ComponentRegistry::GetNetworkFlag(ComponentId _id)
{
    for (auto& info : m_registeredComponents)
        if (info.id == _id)
            return info.networkFlag;
    return 0;
}

void ComponentRegistry::ClearClientSideBits(ComponentMask& _mask)
{
    for (uint32 i = 0; i < ComponentType::Count(); i++)
    {
        if (IsRegistered(i) == false) continue;
        if (IsClientOnly(i) == false) continue;

        _mask.reset(i);
    }
}

ComponentId ComponentRegistry::NextId()
{
    static ComponentId counter = 0;
    assert(counter < MAX_COMPONENTS && "Component count out of bounds");
    return counter++;
}


#endif