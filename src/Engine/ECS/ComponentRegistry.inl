#ifndef COMPONENT_REGISTRY_INL_DEFINED
#define COMPONENT_REGISTRY_INL_DEFINED

#include "ComponentId.hpp"
#include "ComponentRegistry.h"

#include "SystemScheduler.h"
#include "ISystem.h"

template <typename T>
void ComponentRegistry::RegisterComponent(bool _isClientOnly)
{
    ComponentId cid = ComponentType::Id<T>();
    if (IsRegistered(cid)) return;
    
    m_registeredComponents.push_back({ cid, sizeof(T), false, _isClientOnly, 0 });
}

template <typename T>
void ComponentRegistry::RegisterScript(uint8 _nFlag, bool _isClientOnly)
{
    ComponentId cid = ComponentType::Id<T>();
    if (IsRegistered(cid)) return;

    m_registeredComponents.push_back({ 
        cid, sizeof(T), true, _isClientOnly, _nFlag,
        [](EntityId _e, World& _w) -> IScript* { return &_w.GetComponent<T>(_e); },
        [](void* ptr) -> IScript* { return new (ptr) T(); }
    });

    SystemScheduler::Get().RegisterSystem<ScriptSystem<T>>(Phase::Update, _nFlag);
}

#endif