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
    uint8       networkFlag;
    std::function<IScript*(EntityId, World&)> scriptAccessor;
};

struct ComponentRegistry 
{
public:
    static void Init();

    template <typename T>
    static void RegisterComponent();

    template <typename T>
    static void RegisterScript(uint8 _nFlag = 0);
    static IScript* GetScript(ComponentId _id, EntityId _e, World& _w);
    static bool IsRegistered(ComponentId _id);
    static bool IsScript(ComponentId _id);
    
    static uint32 GetSize(ComponentId _id);
    static uint8 GetNetworkFlag(ComponentId _id);
    
private: 
    static ComponentId NextId();
    
    static inline Vector<ComponentInfo> m_registeredComponents = {};
};

#endif