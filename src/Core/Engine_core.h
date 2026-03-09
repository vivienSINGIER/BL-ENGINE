#ifndef ENGINE_CORE_H_DEFINED
#define ENGINE_CORE_H_DEFINED 

namespace Engine
{
    template <typename S>
    int& GetStateID() { static int id = -1; return id; }

    template <typename C>
    uint64_t& GetComponentMask() { static uint64_t id = (uint64_t)-1; return id; }
}

#define NO_STATE				-1

#define STATE_ID(s)				Engine::GetStateID<s>()
#define COMPONENT_MASK(c)		Engine::GetComponentMask<c>()

#endif

