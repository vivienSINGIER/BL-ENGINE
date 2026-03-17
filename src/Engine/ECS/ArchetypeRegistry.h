 #ifndef ARCHETYPE_REGISTRY_H_DEFINED
#define ARCHETYPE_REGISTRY_H_DEFINED

#include "Archetype.hpp"
#include "../define.h"
#include "../Core/define.h"

class World;

class ArchetypeRegistry
{
public:
    Archetype* GetOrCreate(ComponentMask const& _mask);
    Vector<Archetype*> All();
    void SetWorld(World* _pWorld);
    
private:
    Map<String, Archetype*> m_archetypes;
    World* m_world = nullptr;
};

#endif