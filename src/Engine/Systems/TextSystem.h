#ifndef TEXT_SYSTEM_H_DEFINED
#define TEXT_SYSTEM_H_DEFINED

#include "../ECS/ISystem.h"
#include "Components/TextComponent.hpp"

struct TextSystem : public System<TextComponent>
{
    void OnUpdate(float _dt, EntityId _e, TextComponent& _t);
};

#endif
