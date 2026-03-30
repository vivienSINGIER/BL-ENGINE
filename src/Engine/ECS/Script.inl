#ifndef SCRIPT_INL_DEFINED
#define SCRIPT_INL_DEFINED

#include "Script.h"
#include "World.h"

template<typename T>
T& IScript::AddComponent()
{
    assert(m_isStarted && "Cannot call AddComponent before Start, use Start instead of Awake");
    return world->AddComponent<T>(entity);
}
template<typename T>
void IScript::RemoveComponent()
{
    assert(m_isStarted && "Cannot call RemoveComponent before Start, use Start instead of Awake");
    world->RemoveComponent<T>(entity);
}
template<typename T>
T& IScript::GetComponent()
{
    assert(m_isStarted && "Cannot call GetComponent before Start, use Start instead of Awake");
    return world->GetComponent<T>(entity);
}
template<typename T>
bool IScript::HasComponent()
{
    assert(m_isStarted && "Cannot call HasComponent before Start, use Start instead of Awake");
    return world->HasComponent<T>(entity);
}


#endif