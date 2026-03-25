#ifndef COMPONENT_REGISTRY_HPP_DEFINED
#define COMPONENT_REGISTRY_HPP_DEFINED

#include "../define.h"

#include "../Components/TransformComponent.hpp"
#include "../Components/MeshRenderer.hpp"
#include "../Components/CameraComponent.hpp"
#include "../Components/LightComponent.hpp"
#include "../Components/ColliderComponent.hpp"
#include "../Components/PhysicComponent.hpp"
#include "../Components/NetworkComponent.hpp"

struct ComponentRegistry 
{
public:
    template<typename T>
    static ComponentId Id()
    {
        static const ComponentId id = NextId();
        return id;
    }

    void Init()
    {
        RegisterComponent<TransformComponent>();
        RegisterComponent<MeshRenderer>();
        RegisterComponent<CameraComponent>();
        RegisterComponent<LightComponent>();
        RegisterComponent<ColliderComponent>();
        RegisterComponent<PhysicComponent>();
        RegisterComponent<NetworkComponent>();
    }

    template <typename T>
    void RegisterComponent()
    {
        if (ComponentRegistry::Id<T>() < m_registeredComponent.size()) return;
        
        m_registeredComponent.push_back({ComponentRegistry::Id<T>(), sizeof(T)});
    }
    
    uint32 GetSize(ComponentId _id)
    {
        if (_id >= m_registeredComponent.size()) return 0;
        
        return m_registeredComponent[_id].second;
    }
    
private: 
    static ComponentId NextId()
    {
        static ComponentId counter = 0;
        assert(counter < MAX_COMPONENTS && "Component count out of bounds");
        return counter++;
    }
    
    Vector<Pair<ComponentId, uint32>> m_registeredComponent;
};

#endif
