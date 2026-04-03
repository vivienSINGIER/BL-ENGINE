#ifndef SYSTEM_SCHEDULER_HPP_DEFINED
#define SYSTEM_SCHEDULER_HPP_DEFINED

#include "Archetype.hpp"
#include "../define.h"

struct ISystem;
class World;

enum Phase
{
    PreUpdate, Update, FixedUpdate, PostUpdate, PreRender, Render, PostRender, NetworkReceive, NetworkSend, Count
};

class SystemScheduler
{
public:
    static SystemScheduler& Get()
    {
        static SystemScheduler instance;
        return instance;
    }

    void BindWorld(World* world);
    
    SystemScheduler(const SystemScheduler&) = delete;
    SystemScheduler& operator=(const SystemScheduler&) = delete;

    template <typename T, typename... Args>
    T* RegisterSystem(Phase _phase, uint8 _nFlag = NetworkFlag::ALL, Args&&... args);

    template <typename T>
    T* GetSystem();
    
    void Run(float _dt);
    
private:
    SystemScheduler() = default;

    Array<Vector<ISystem*>, Phase::Count> m_phases;
    float m_accumulator = 0.0f;
};

#include "SystemScheduler.inl"

#endif