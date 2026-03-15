#ifndef SCRIPT_HPP_DEFINED
#define SCRIPT_HPP_DEFINED

#include "define.h"
#include "World.h"

struct ScriptRegistry
{
    static constexpr uint64 MAX_SCRIPTS = 16;
    ComponentId ownedScripts[MAX_SCRIPTS] = {};
    uint64 count = 0;

    void push_back(ComponentId cid) { ownedScripts[count++] = cid; }
    void remove(ComponentId cid)
    {
        if (count == 0) return;
        
        for (int i = 0; i < count - 1; i++)
        {
            if (ownedScripts[i] == cid)
            {
                ownedScripts[i] = ownedScripts[count - 1];
            }
        }
        
        count--;
    }
};

struct IScript
{
    EntityId entity = 0;
    World* world = nullptr;
    
    virtual void Awake() {}
    virtual void Start() {}
    virtual void Update(float _dt) {}
    virtual void Destroy() {}

    // TODO Add OnCollide & OnTrigger methods

    virtual ~IScript() = default;

    template<typename T> T&     GetComponent() { return world->GetComponent<T>(entity); }
    template<typename T> bool   HasComponent() { return world->HasComponent<T>(entity); }

    // TODO add command queue system to allow following
    // template<typename T> T&     AddComponent() { return world->AddComponent<T>(entity); }
    // template<typename T> void   RemoveComponent() { world->RemoveComponent<T>(entity); }

private:
    bool m_isStarted = false;

    template <typename T>
    friend struct ScriptSystem;
};

#endif