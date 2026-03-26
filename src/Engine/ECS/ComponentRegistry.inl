#ifndef COMPONENT_REGISTRY_INL_DEFINED
#define COMPONENT_REGISTRY_INL_DEFINED

#include "ComponentId.hpp"
#include "ComponentRegistry.h"

#include "SystemScheduler.h"
#include "ISystem.h"

template <typename T>
void ComponentRegistry::RegisterComponent()
{
    ComponentId cid = ComponentType::Id<T>();
    if (IsRegistered(cid)) return;
    
    m_registeredComponents.push_back({ cid, sizeof(T), false, 0 });
}

template <typename T>
void ComponentRegistry::RegisterScript(uint8 _nFlag)
{
    ComponentId cid = ComponentType::Id<T>();
    if (IsRegistered(cid)) return;

    m_registeredComponents.push_back({ 
        cid, sizeof(T), true, _nFlag,
        [](EntityId _e, World& _w) -> IScript* { return &_w.GetComponent<T>(_e); }
    });

    SystemScheduler::Get().RegisterSystem<ScriptSystem<T>>(Phase::Update, _nFlag);
}

#endif