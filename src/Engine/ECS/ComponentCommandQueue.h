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
    
    void SetRequiredMask(EntityId _e, ComponentMask _mask);
    
    template <typename T>
    void EmplaceRemove(EntityId _e);
    void EmplaceRemoveRaw(EntityId _e, ComponentId _cid);

    void EmplaceDestroy(EntityId _e);

    void Flush(World* _pWorld);
    
    void* GetComponent(EntityId _e, ComponentId _cid);
    bool HasComponent(EntityId _e, ComponentId _cid);
    bool IsAlive(EntityId _e);
    
private:
    struct Command
    {
        EntityId entity;
        ComponentId component;

        bool isScript = false;
        bool isClientSide = false;
        
        uint64 size;
        Vector<uint8> data;
        std::function<void(ComponentId, const void*, ComponentStorage&)> applyFunc;
    };

    Vector<Command> m_toCreate;
    Vector<Command> m_toAdd;
    Vector<Command> m_toRemove;
    Vector<Command> m_toDestroy;

    UnorderedMap<EntityId, ComponentMask> m_requiredMasks;
    
    void FlushCreate(World* _pWorld);
    void FlushAdd(World* _pWorld, Vector<Command>::iterator& _it);
    void FlushRemove(World* _pWorld);
    void FlushDestroy(World* _pWorld);
};

#include "ComponentCommandQueue.inl"

#endif
