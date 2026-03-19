#ifndef COMPONENT_COMMANDQUEUE_HPP_DEFINED
#define COMPONENT_COMMANDQUEUE_HPP_DEFINED

#include <functional>

#include "define.h"
#include "ComponentStorage.hpp"

class ComponentCommandQueue
{
public:

    template <typename T>
    T& EmplaceAdd(EntityId _e, T& _val = {})
    {
        Command cmd;
        cmd.entity = _e;
        cmd.component = ComponentRegistry::Id<T>();
        cmd.size = sizeof(T);
        cmd.offset = m_offset;
        
        T* ptr = reinterpret_cast<T*>(m_componentSideBuffer + m_offset);
        memcpy(ptr, &_val, sizeof(T));

        cmd.applyFunc = [](ComponentId _cid, const void* _data, ComponentStorage& _storage) -> void
        {
            _storage.Push<T>(_cid, *static_cast<const T*>(_data));
        };

        m_toAdd.push_back(cmd);
        
        return *ptr;
    }
    
    template <typename T>
    void EmplaceRemove(EntityId _e)
    {
        Command cmd;
        cmd.entity = _e;
        cmd.component = ComponentRegistry::Id<T>();
        m_toRemove.push_back(cmd);
    }

    void EmplaceDestroy(EntityId _e)
    {
        Command cmd;
        cmd.entity = _e;
        m_toDestroy.push_back(cmd);
    }

    void Flush()
    {
        // TODO
    }
    
private:
    struct Command
    {
        EntityId entity;
        ComponentId component;

        uint64 size;
        uint64 offset;
        std::function<void*(ComponentId, const void*, ComponentStorage&)> applyFunc;
    };
    
    static constexpr uint64 BUFFER_SIZE = 1024 * 1024;

    uint8 m_componentSideBuffer[BUFFER_SIZE] = {};
    uint64 m_offset = 0;

    Vector<Command> m_toAdd;
    Vector<Command> m_toRemove;
    Vector<Command> m_toDestroy;
    
};

#endif
