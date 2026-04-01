#ifndef COMPONENT_COMMANDQUEUE_INL_DEFINED
#define COMPONENT_COMMANDQUEUE_INL_DEFINED

#include "ComponentCommandQueue.h"
#include "ComponentId.hpp"

template <typename T>
T& ComponentCommandQueue::EmplaceAdd(EntityId _e, bool _isClientSide, T const& _val)
{
    Command cmd;
    cmd.entity = _e;
    cmd.component = ComponentType::Id<T>();
    cmd.size = sizeof(T);
    cmd.offset = m_offset;
    cmd.isScript = ComponentRegistry::IsScript(ComponentType::Id<T>());
    cmd.isClientSide = _isClientSide;
        
    T* ptr = reinterpret_cast<T*>(m_componentSideBuffer + m_offset);
    memcpy(ptr, &_val, sizeof(T));

    cmd.applyFunc = [](ComponentId _cid, const void* _data, ComponentStorage& _storage) -> void
    {
        _storage.Push<T>(_cid, *static_cast<const T*>(_data));
    };

    m_offset += cmd.size;
    m_toAdd.emplace(m_toAdd.begin(), cmd);
        
    return *ptr;
}

template <typename T>
void ComponentCommandQueue::EmplaceRemove(EntityId _e, bool _isClientSide)
{
    Command cmd;
    cmd.entity = _e;
    cmd.component = ComponentType::Id<T>();
    cmd.isScript = ComponentRegistry::IsScript(ComponentType::Id<T>());
    cmd.isClientSide = _isClientSide;
    m_toRemove.emplace(m_toRemove.begin(), cmd);
}

#endif
