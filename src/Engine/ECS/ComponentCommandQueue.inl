#ifndef COMPONENT_COMMANDQUEUE_INL_DEFINED
#define COMPONENT_COMMANDQUEUE_INL_DEFINED

#include "ComponentCommandQueue.h"
#include "ComponentId.hpp"

template <typename T>
T& ComponentCommandQueue::EmplaceAdd(EntityId _e, T const& _val)
{
    Command cmd;
    cmd.entity = _e;
    cmd.component = ComponentType::Id<T>();
    cmd.size = sizeof(T);
    cmd.isClientSide = ComponentRegistry::IsClientOnly(ComponentType::Id<T>());
    cmd.isScript = ComponentRegistry::IsScript(ComponentType::Id<T>());
    
    cmd.applyFunc = [](ComponentId _cid, const void* _data, ComponentStorage& _storage) -> void
    {
        _storage.Push<T>(_cid, *static_cast<const T*>(_data));
    };
    
    m_toAdd.emplace(m_toAdd.begin(), cmd);
    Command& ref = m_toAdd.front();
    
    ref.data.resize(ref.size);
    T* ptr = reinterpret_cast<T*>(ref.data.data());
    memcpy(ptr, &_val, sizeof(T));

    if (ref.isScript)
        ComponentRegistry::ConstructScript(ref.component, ptr);
        
    if (m_requiredMasks.contains(_e))
    {
        m_requiredMasks[_e].reset(ref.component);
        if (m_requiredMasks[_e] == 0)
            m_requiredMasks.erase(_e);
    }
    
    return *ptr;
}

template <typename T>
void ComponentCommandQueue::EmplaceRemove(EntityId _e)
{
    Command cmd;
    cmd.entity = _e;
    cmd.component = ComponentType::Id<T>();
    cmd.isScript = ComponentRegistry::IsScript(ComponentType::Id<T>());
    cmd.isClientSide = ComponentRegistry::IsClientOnly(ComponentType::Id<T>());
    m_toRemove.emplace(m_toRemove.begin(), cmd);
}

#endif
