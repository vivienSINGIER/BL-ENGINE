#ifndef IMAGESYSTEM_H_DEFINED
#define IMAGESYSTEM_H_DEFINED

#include "../ECS/ISystem.h"
#include "Components/UiImageComponent.hpp"

struct ImageSystem : public System<UiImageComponent>
{
    void OnUpdate(float _dt, EntityId _e, UiImageComponent& _image);
};

#endif
