#ifndef SCRIPT_H_DEFINED
#define SCRIPT_H_DEFINED

#include "define.h"

class World;

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

    template<typename T> T&     AddComponent();
    template<typename T> void   RemoveComponent();
    template<typename T> T&     GetComponent();
    template<typename T> bool   HasComponent();
    

private:
    bool m_isStarted = false;

    template <typename T>
    friend struct ScriptSystem;
};

#include "Script.inl"

#endif