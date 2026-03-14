#include "ArchetypeRegistry.h"
#include "World.h"

Archetype* ArchetypeRegistry::GetOrCreate(ComponentMask const& _mask)
{
    auto [it, inserted] = m_archetypes.emplace(_mask.to_string(), nullptr);
    if (inserted && m_world != nullptr)
    {
        it->second = new Archetype(_mask);
        m_world->OnArchetypeCreated(it->second);
    }
    return it->second;
}

Vector<Archetype*> ArchetypeRegistry::All()
{
    Vector<Archetype*> result;
    for (auto const& it : m_archetypes)
        result.push_back(it.second);
    return result;
}

void ArchetypeRegistry::SetWorld(World* _pWorld) { m_world = _pWorld; }