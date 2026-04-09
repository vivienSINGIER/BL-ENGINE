#ifndef BUTTON_SYSTEM_H_DEFINED
#define BUTTON_SYSTEM_H_DEFINED

#include "../ECS/ISystem.h"
#include "../Components/UiButtonComponent.hpp"

class ButtonSystem : public System<UiButtonComponent>
{
public:
    void OnUpdate(float _dt, EntityId _e, UiButtonComponent& _button);

    void Init(UiButtonComponent& _button);
    bool IsHovered(UiButtonComponent& _button);
    void TransitionState(UiButtonComponent& _b, UiButtonComponent::ButtonStateType _newState);

    void Draw(UiButtonComponent& _button);
};

#endif
