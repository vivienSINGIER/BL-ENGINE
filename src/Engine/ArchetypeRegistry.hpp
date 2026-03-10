#ifndef ARCHETYPE_REGISTRY_HPP
#define ARCHETYPE_REGISTRY_HPP

#include "Archetype.hpp"
#include "Engine.h"
#include "Common/Common.h"

class SystemScheduler;

class ArchetypeRegistry
{
public:
    Archetype* GetOrCreate(ComponentMask const& _mask)
    {
        auto [it, inserted] = m_archetypes.emplace(_mask.to_string(), nullptr);
        if (inserted && m_scheduler != nullptr)
        {
            it->second = new Archetype(_mask);
            m_scheduler->OnArchetypeCreated(it->second);
        }
        return it->second;
    }

    Vector<Archetype*> All()
    {
        Vector<Archetype*> result;
        for (auto const& it : m_archetypes)
            result.push_back(it.second);
        return result;
    }
    
private:
    Map<String, Archetype*> m_archetypes;
    SystemScheduler* m_scheduler = nullptr;
};

#endif