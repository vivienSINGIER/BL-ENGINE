#ifndef COMPONENT_REGISTRY_H_DEFINED
#define COMPONENT_REGISTRY_H_DEFINED

#include "../define.h"
#include <functional>

class World;
struct IScript;

struct ComponentInfo
{
    ComponentId id;
    uint32      size;
    bool        isScript;
    bool        isClientOnly;
    uint8       networkFlag;
    std::function<IScript*(EntityId, World&)> scriptAccessor;
    std::function<IScript*(void* ptr)> scriptConstructor;
};

struct ComponentRegistry 
{
public:
    static void Init();

    template <typename T>
    static void RegisterComponent(bool _isClientOnly = false);

    template <typename T>
    static void RegisterScript(uint8 _nFlag = NetworkFlag::ALL, bool _isClientOnly = false);
    
    static IScript* GetScript(ComponentId _id, EntityId _e, World& _w);
    static IScript* ConstructScript(ComponentId _id, void* ptr);
    static bool IsScript(ComponentId _id);
    
    static bool IsRegistered(ComponentId _id);
    static bool IsClientOnly(ComponentId _id);
    
    static uint32 GetSize(ComponentId _id);
    static uint8 GetNetworkFlag(ComponentId _id);
    
private: 
    static ComponentId NextId();
    
    static inline Vector<ComponentInfo> m_registeredComponents = {};
};

#endif