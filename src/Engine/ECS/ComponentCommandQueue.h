#ifndef COMPONENT_COMMANDQUEUE_HPP_DEFINED
#define COMPONENT_COMMANDQUEUE_HPP_DEFINED

#include <functional>
#include "define.h"

#include "ComponentStorage.hpp"

class World;

class ComponentCommandQueue
{
public:

    void EmplaceCreate(EntityId _e);
    
    template <typename T>
    T& EmplaceAdd(EntityId _e, T const& _val = {});
    void* EmplaceAddRaw(EntityId _e, ComponentId _cid, uint64 _size, const void* _data = nullptr);
    
    template <typename T>
    void EmplaceRemove(EntityId _e);
    void EmplaceRemoveRaw(EntityId _e, ComponentId _cid);

    void EmplaceDestroy(EntityId _e);

    void Flush(World* _pWorld);
    
private:
    struct Command
    {
        EntityId entity;
        ComponentId component;

        bool isScript = false;
        
        uint64 size;
        uint64 offset;
        std::function<void(ComponentId, const void*, ComponentStorage&)> applyFunc;
    };
    
    static constexpr uint64 BUFFER_SIZE = 2048 * 2048;

    uint8 m_componentSideBuffer[BUFFER_SIZE] = {};
    uint64 m_offset = 0;

    Vector<Command> m_toCreate;
    Vector<Command> m_toAdd;
    Vector<Command> m_toRemove;
    Vector<Command> m_toDestroy;

    void FlushCreate(World* _pWorld);
    void FlushAdd(World* _pWorld);
    void FlushRemove(World* _pWorld);
    void FlushDestroy(World* _pWorld);
};

#include "ComponentCommandQueue.inl"

#endif
